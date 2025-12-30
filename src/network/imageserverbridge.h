#ifndef IMAGESERVERBRIDGE_H
#define IMAGESERVERBRIDGE_H

#include <QObject>
#include <QHostAddress>
#include <QByteArray>
#include <QVariantMap>
#include <QImage>

#include "eventcodes.h"

class WebSocketServer;
class ClientModel;
class QSettings;

class ImageServerBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* clientModel READ clientModel CONSTANT)
    Q_PROPERTY(int frameId READ frameId NOTIFY frameIdChanged)
    Q_PROPERTY(QString activeClient READ activeClient NOTIFY activeClientChanged)
    Q_PROPERTY(QString activeClientAlias READ activeClientAlias NOTIFY activeClientAliasChanged)
    Q_PROPERTY(int currentFps READ currentFps NOTIFY currentFpsChanged)
    Q_PROPERTY(int activeClientMeasuredFps READ activeClientMeasuredFps NOTIFY activeClientMeasuredFpsChanged)
    Q_PROPERTY(int configuredFps READ configuredFps WRITE setConfiguredFps NOTIFY configuredFpsChanged)
    Q_PROPERTY(ServerState serverState READ serverState NOTIFY serverStateChanged)
    Q_PROPERTY(ConnectionState connectionState READ connectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    enum ServerState {
        Idle = 0,
        Starting,
        Running,
        Stopping,
        Error
    };
    Q_ENUM(ServerState)

    enum ConnectionState {
        NoClients = 0,
        ClientsConnected,
        ReceivingFrames
    };
    Q_ENUM(ConnectionState)

    explicit ImageServerBridge(QObject* parent = nullptr);
    ~ImageServerBridge() override;
    
    int activeClientMeasuredFps() const;
    int configuredFps() const;

    QObject* clientModel() const;
    QString activeClient() const;
    QString activeClientAlias() const;

    ServerState serverState() const;
    ConnectionState connectionState() const;
    QString statusMessage() const;

    Q_INVOKABLE bool start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void setActiveClient(const QString& clientId);
    Q_INVOKABLE void setFps(int fps);
    Q_INVOKABLE quint16 serverPort() const;
    Q_INVOKABLE void setPort(quint16 port);
    Q_INVOKABLE void recordFrameReceived(const QString& clientId);
    Q_INVOKABLE void setConfiguredFps(int fps);

    // Helper to emit events to QML along with optional details
    void emitEvent(imagesocket::EventCode code, const QVariantMap &details = QVariantMap());

    QImage lastFrame() const;
    int frameId() const;
    int currentFps() const;

signals:
    void currentFpsChanged(int fps);
    void configuredFpsChanged(int fps);

signals:
    void activeClientChanged(const QString& clientId);
    void activeClientAliasChanged(const QString& alias);
    void newFrameReady(const QImage& frame);
    void frameIdChanged(int newId);
    void connectionLost();

    // Server state & connection state notifications
    void serverStateChanged();
    void connectionStateChanged();
    void statusMessageChanged();

    // Centralized event signal for UI messages. UI maps codes to localized text.
    void eventOccurred(imagesocket::EventCode code, const QVariantMap& details);

signals:
    // Client-specific signals with alias when available
    void clientConnectedWithAlias(const QString& clientId, const QString& alias);
    void clientDisconnectedWithAlias(const QString& clientId, const QString& alias);

private slots:
    void onClientConnected(const QString& clientId, const QHostAddress& address);
    void onControlMessageReceived(const QString& clientId, const QByteArray& serialized);
    void onSessionDisconnected(const QString& clientId);
    void onFrameReceived(const QString& clientId, const QImage& frame);

    // Handle server errors from WebSocketServer and forward to UI
    void onServerError(imagesocket::EventCode code, const QVariantMap &details);

private:
    // State helpers
    void setServerState(ServerState state);
    void setConnectionState(ConnectionState state);
    void setStatusMessage(const QString& msg);

    WebSocketServer* m_server = nullptr;
    ClientModel* m_clientModel = nullptr;
    QString m_activeClientId;

    // Cache of last frame for the QML image provider
    QImage m_lastFrame;
    int m_frameId = 0;
    int m_port = 0;

    // New state members
    ServerState m_serverState = Idle;
    ConnectionState m_connectionState = NoClients;
    QString m_statusMessage; 

    // Current FPS configured/applied (0 == unset)
    int m_currentFps = 0;

    // Server-wide configured FPS (persisted value that should be reapplied on new connections)
    int m_configuredFps = 0;

    // Persistent settings (QSettings) used to store configured FPS
    QSettings* m_settings = nullptr;

    // Measured FPS for the active client
    int m_activeClientMeasuredFps = 0;

signals:
    void activeClientMeasuredFpsChanged(int fps);
};

#endif // IMAGESERVERBRIDGE_H
