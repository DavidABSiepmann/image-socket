#ifndef CLIENTMODEL_H
#define CLIENTMODEL_H

#include <QAbstractListModel>
#include <QVector>

struct ClientEntry {
    QString id;
    QString status;
    QString alias;

    int configuredFps = 0;   // FPS requested by server for this client
    int measuredFps = 0;     // Measured FPS (updated once per accumulation window)
    qint64 lastFrameTsMs = 0; // timestamp of last frame (ms)

    // Windowed accumulation for simple FPS measurement
    int framesInWindow = 0;
    qint64 windowStartMs = 0; // start timestamp of counting window (ms)
};

class ClientModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        StatusRole,
        AliasRole,
        ConfiguredFpsRole,
        MeasuredFpsRole
    };

    Q_PROPERTY(int count READ count NOTIFY countChanged)

    explicit ClientModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void addClient(const QString& id, const QString& status = QString());
    void removeClient(const QString& id);
    void clear();
    void setClientStatus(const QString& id, const QString& status);
    void setClientAlias(const QString& id, const QString& alias);

    Q_INVOKABLE int indexOfClient(const QString& id) const;
    Q_INVOKABLE QString clientIdAt(int index) const; 
    Q_INVOKABLE QString aliasAt(int index) const;
    Q_INVOKABLE int configuredFpsAt(int index) const;
    Q_INVOKABLE int measuredFpsAt(int index) const;
    Q_INVOKABLE int count() const { return m_clients.size(); }

public slots:
    void setClientConfiguredFps(const QString& id, int fps);
    void setClientMeasuredFps(const QString& id, int fps);
    void recordFrameReceived(const QString& id, qint64 timestampMs = 0);

signals:
    void countChanged(int newCount);

private:
    QVector<ClientEntry> m_clients;
};

#endif // CLIENTMODEL_H
