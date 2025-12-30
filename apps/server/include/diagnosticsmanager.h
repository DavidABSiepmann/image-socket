#pragma once

#include <QObject>
#include <QDateTime>
#include <QHash>
#include <QTimer>
#include <QMutex>
#include "diagnosticsmodel.h"

class DiagnosticsManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QAbstractItemModel* diagnosticsModel READ diagnosticsModel NOTIFY diagnosticsModelChanged)
    Q_PROPERTY(QString lastErrorMessage READ lastErrorMessage NOTIFY lastErrorChanged)
    Q_PROPERTY(bool panelVisible READ panelVisible WRITE setPanelVisible NOTIFY panelVisibleChanged)
public:
    enum ErrorSeverity { Info = 0, Warning = 1, Error = 2, Critical = 3 };
    Q_ENUM(ErrorSeverity)

    explicit DiagnosticsManager(QObject *parent = nullptr);

    QAbstractItemModel* diagnosticsModel() const { return m_model; }
    bool isEnabled() const { return m_enabled; }
    QString lastErrorMessage() const { return m_lastErrorMessage; }
    bool panelVisible() const { return m_panelVisible; }
    void setPanelVisible(bool v);
    
public slots:
    void postError(int code, const QString &message, ErrorSeverity severity = Error, const QString &source = QString(), const QVariantMap &metadata = QVariantMap());
    void postErrorEvent(const DiagnosticEntry &entry);
    void setEnabled(bool v);

    Q_INVOKABLE void toggleVisibility() {
        setPanelVisible(!m_panelVisible);
    }

    // test-only injection - enabled only when DIAGNOSTICS_TEST_API is defined
#ifdef DIAGNOSTICS_TEST_API
    Q_INVOKABLE void injectTestEvent(const DiagnosticEntry &event);
#endif

    void setAggregationWindowMs(int ms);
    void setRateLimitUniquePerMinute(int count);
    void setVerbose(bool v);
    void dumpLogs(const QString &path);
    Q_INVOKABLE void clear();

    // Generate demo logs (useful for testing & demo UI)
    Q_INVOKABLE void generateDemoLogs();

signals:
    void errorPosted();
    void aggregatedUpdated();
    void criticalAlert(int code, const QString &message);
    void enabledChanged(bool);
    void diagnosticsModelChanged();
    void lastErrorChanged();
    void panelVisibleChanged();

private:
    QString makeSignature(int code, const QString &message, const QString &source) const;

    DiagnosticsModel *m_model;
    QMutex m_mutex; // guards internal maps
    struct AggState { QDateTime first; QDateTime last; int count; bool suppressed; DiagnosticEntry entry; };
    QHash<QString, AggState> m_aggregation;

    int m_aggregationWindowMs = 1000; // default 1s
    int m_rateLimitUniquePerMinute = 100; // default
    bool m_verbose = false;
    bool m_enabled = true;
    QString m_lastErrorMessage;

    // burst handling
    bool m_burstMode = false;
    QTimer m_burstTimer;
    int m_burstConsolidationMs = 5000; // 5s

    // UI state: whether the diagnostics panel is visible
    bool m_panelVisible = false;
};
