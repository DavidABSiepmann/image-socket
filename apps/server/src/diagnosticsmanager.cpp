#include "diagnosticsmanager.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QThread>

DiagnosticsManager::DiagnosticsManager(QObject *parent)
    : QObject(parent), m_model(new DiagnosticsModel(this))
{
    // connect burst timer
    m_burstTimer.setSingleShot(false);
    m_burstTimer.setInterval(m_burstConsolidationMs);
    connect(&m_burstTimer, &QTimer::timeout, this, [this]() {
        // On consolidation, push current aggregation snapshot to model
        QMutexLocker locker(&m_mutex);
        for (auto it = m_aggregation.begin(); it != m_aggregation.end(); ++it) {
            const AggState &s = it.value();
            if (s.count > 0) {
                m_model->upsertEntry(s.entry);
            }
        }
    });
}

QString DiagnosticsManager::makeSignature(int code, const QString &message, const QString &source) const
{
    QByteArray data;
    data.append(QString::number(code).toUtf8());
    data.append("|");
    data.append(source.toUtf8());
    data.append("|");
    data.append(message.left(256).toUtf8()); // normalize
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha1);
    return QString::fromLatin1(hash.toHex());
}

void DiagnosticsManager::postError(int code, const QString &message, ErrorSeverity severity, const QString &source, const QVariantMap &metadata)
{
    if (!m_enabled) return;

    QDateTime now = QDateTime::currentDateTimeUtc();
    DiagnosticEntry e;
    e.id = makeSignature(code, message, source);
    e.code = code;
    e.message = message;
    e.severity = severity;
    e.source = source;
    e.firstTimestamp = now;
    e.lastTimestamp = now;
    e.count = 1;
    e.metadata = metadata;

    // ensure thread-safe post: marshal to this object's thread (usually main)
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, [this, e]() { postErrorEvent(e); }, Qt::QueuedConnection);
        return;
    }

    postErrorEvent(e);
}

void DiagnosticsManager::postErrorEvent(const DiagnosticEntry &entry)
{
    if (!m_enabled) return;

    QMutexLocker locker(&m_mutex);

    QString sig = entry.id;
    auto it = m_aggregation.find(sig);
    QDateTime now = QDateTime::currentDateTimeUtc();

    if (it == m_aggregation.end()) {
        AggState s;
        s.first = entry.firstTimestamp;
        s.last = entry.lastTimestamp;
        s.count = entry.count;
        s.suppressed = false;
        s.entry = entry;
        m_aggregation.insert(sig, s);
    } else {
        // update existing aggregation
        AggState &s = it.value();
        qint64 ms = s.last.msecsTo(now);
        if (ms <= m_aggregationWindowMs) {
            s.count += 1;
            s.last = now;
            s.entry.count = s.count;
            s.entry.lastTimestamp = now;
            s.entry.message = entry.message; // keep latest message snapshot
        } else {
            // bucket expired: start a new bucket by replacing
            AggState ns;
            ns.first = entry.firstTimestamp;
            ns.last = entry.lastTimestamp;
            ns.count = entry.count;
            ns.suppressed = false;
            ns.entry = entry;
            it.value() = ns;
        }
    }

    // Simple burst detection: if too many unique signatures, enable burst mode
    if (!m_burstMode && static_cast<int>(m_aggregation.size()) > m_rateLimitUniquePerMinute) {
        m_burstMode = true;
        m_burstTimer.start();
        qWarning() << "DiagnosticsManager: entering burst mode";
    }

    // If in burst mode, limit frequent updates and rely on consolidation timer
    if (m_burstMode) {
        // mark as suppressed for UI and do not upsert each time
        AggState &s = m_aggregation[sig];
        s.suppressed = true;
        // model will be updated on consolidation timer; record last message too
        return;
    }

    // not in burst mode: push immediate update to model
    AggState &s = m_aggregation[sig];
    s.entry.count = s.count;
    s.entry.lastTimestamp = s.last;
    m_model->upsertEntry(s.entry);

    m_lastErrorMessage = entry.message;
    emit lastErrorChanged();
}

void DiagnosticsManager::setEnabled(bool v)
{
    if (m_enabled == v) return;
    m_enabled = v;
    emit enabledChanged(v);
}

void DiagnosticsManager::setAggregationWindowMs(int ms)
{
    m_aggregationWindowMs = ms;
}

void DiagnosticsManager::setRateLimitUniquePerMinute(int count)
{
    m_rateLimitUniquePerMinute = count;
}

void DiagnosticsManager::setVerbose(bool v)
{
    m_verbose = v;
}

void DiagnosticsManager::dumpLogs(const QString &path)
{
    Q_UNUSED(path);
    qInfo() << "DiagnosticsManager::dumpLogs not implemented yet";
}

void DiagnosticsManager::clear()
{
    QMutexLocker locker(&m_mutex);
    m_aggregation.clear();
    m_model->clear();
}

void DiagnosticsManager::setPanelVisible(bool v)
{
    if (m_panelVisible == v) return;
    m_panelVisible = v;
    emit panelVisibleChanged();
}
void DiagnosticsManager::generateDemoLogs()
{
    // Generate a few messages of varying severity, plus a small burst to test aggregation
    postError(1000, "Demo: informational message", ErrorSeverity::Info, "demo");
    postError(2000, "Demo: warning message", ErrorSeverity::Warning, "demo");
    postError(3000, "Demo: error message", ErrorSeverity::Error, "demo");
    postError(4000, "Demo: critical message", ErrorSeverity::Critical, "demo");

    // Small burst: 50 repeated warnings over ~500ms to trigger aggregation
    for (int i = 0; i < 50; ++i) {
        int delayMs = i * 10; // 0,10,20,...490ms
        QTimer::singleShot(delayMs, this, [this]() {
            postError(2000, "Demo: warning message (burst)", ErrorSeverity::Warning, "demo-burst");
        });
    }
}

