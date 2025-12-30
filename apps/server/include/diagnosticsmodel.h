#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <deque>

struct DiagnosticEntry {
    QString id;
    int code;
    QString message;
    int severity; // use ErrorSeverity enum in DiagnosticsManager
    QString source;
    QDateTime firstTimestamp;
    QDateTime lastTimestamp;
    int count = 1;
    QVariantMap metadata;
    bool suppressed = false;
};

class DiagnosticsModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        CodeRole,
        MessageRole,
        SeverityRole,
        CountRole,
        FirstTimestampRole,
        LastTimestampRole,
        SourceRole,
        MetadataRole,
        SuppressedRole
    };

    explicit DiagnosticsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // update or append an entry; owns copy of entry
    void upsertEntry(const DiagnosticEntry &entry);

    // configuration
    void setMaxEntries(int maxEntries);
    int maxEntries() const;

    Q_INVOKABLE void clear();

private:
    std::deque<DiagnosticEntry> m_entries;
    int m_maxEntries = 500; // user agreed memory limit
};
