#include <QHostAddress>
#include <QDebug>
#include <QDateTime>
#include <QSettings>

#include "imageserverbridge.h"
#include "websocketserver.h"
#include "clientmodel.h"
#include "control.pb.h"

ImageServerBridge::ImageServerBridge(QObject* parent)
    : QObject(parent)
{
    // Initialize persistent settings for configured FPS
    m_settings = new QSettings("ImageSocket", "Server", this);
    m_configuredFps = m_settings->value("fps", m_configuredFps).toInt();
    emit configuredFpsChanged(m_configuredFps);

    m_server = new WebSocketServer(this);
    m_clientModel = new ClientModel(this);

    // connect server signals
    connect(m_server, &WebSocketServer::clientConnected, this, &ImageServerBridge::onClientConnected);
    connect(m_server, &WebSocketServer::clientDisconnected, this, &ImageServerBridge::onSessionDisconnected);
    connect(m_server, &WebSocketServer::controlMessageReceived, this, &ImageServerBridge::onControlMessageReceived);
    connect(m_server, &WebSocketServer::frameReceived, this, &ImageServerBridge::onFrameReceived);

    // Forward server-level errors to UI via eventOccurred
    connect(m_server, &WebSocketServer::serverError, this, &ImageServerBridge::onServerError);
}

// --- State getters and helpers ---
ImageServerBridge::ServerState ImageServerBridge::serverState() const { return m_serverState; }
ImageServerBridge::ConnectionState ImageServerBridge::connectionState() const { return m_connectionState; }
QString ImageServerBridge::statusMessage() const { return m_statusMessage; }
QImage ImageServerBridge::lastFrame() const { return m_lastFrame; }
int ImageServerBridge::frameId() const { return m_frameId; }
int ImageServerBridge::currentFps() const { return m_currentFps; }


void ImageServerBridge::setServerState(ServerState state)
{
    if (m_serverState == state) return;

    m_serverState = state;

    emit serverStateChanged();
}

void ImageServerBridge::setConnectionState(ConnectionState state)
{
    if (m_connectionState == state) return;

    m_connectionState = state;

    emit connectionStateChanged();
}

void ImageServerBridge::setStatusMessage(const QString& msg)
{
    if (m_statusMessage == msg) return;

    m_statusMessage = msg;

    emit statusMessageChanged();
} 

ImageServerBridge::~ImageServerBridge()
{
    stop();
}

void ImageServerBridge::emitEvent(imagesocket::EventCode code, const QVariantMap &details)
{
    emit eventOccurred(code, details);
}

QObject* ImageServerBridge::clientModel() const
{
    return m_clientModel;
}

bool ImageServerBridge::start()
{
    setServerState(ServerState::Starting);
    setStatusMessage(QStringLiteral("Starting"));

    if (!m_server->start(m_port)) {
        QVariantMap details;
        details["port"] = m_port;
        details["reason"] = "failed to bind";
        setServerState(ServerState::Error);
        setStatusMessage(QStringLiteral("Failed to start"));
        emit eventOccurred(imagesocket::ServerStartFailed, details);
        return false;
    }

    QVariantMap details;
    details["port"] = serverPort();
    setServerState(ServerState::Running);
    setStatusMessage(QStringLiteral("Running"));
    emit eventOccurred(imagesocket::ServerStarted, details);
    return true;
} 

quint16 ImageServerBridge::serverPort() const
{
    return m_server ? m_server->port() : 0;
}

void ImageServerBridge::setPort(quint16 port)
{
    m_port = port;
}



void ImageServerBridge::stop()
{
    if (m_server) {
        setServerState(ServerState::Stopping);
        setStatusMessage(QStringLiteral("Stopping"));
        m_server->stop();
        emit eventOccurred(imagesocket::ServerStopped, QVariantMap());
        setServerState(ServerState::Idle);
        setStatusMessage(QStringLiteral("Stopped"));
    }
} 

void ImageServerBridge::setActiveClient(const QString& clientId)
{
    if (m_activeClientId == clientId)
        return;

    // Mark previous active client as connected (if present)
    if (!m_activeClientId.isEmpty()) {
        m_clientModel->setClientStatus(m_activeClientId, QStringLiteral("Connected"));
    }

    QString previousClient = m_activeClientId;
    m_activeClientId = clientId;

    // Update model to reflect active status
    m_clientModel->setClientStatus(m_activeClientId, QStringLiteral("Active"));

    setFps(m_configuredFps);

    // Emit event for UI
    QVariantMap details;
    details["clientId"] = clientId;
    QString alias = m_clientModel->aliasAt(m_clientModel->indexOfClient(clientId));
    details["alias"] = alias.isEmpty() ? clientId : alias;
    details["previousClient"] = previousClient;
    emit eventOccurred(imagesocket::ClientBecameActive, details);

    emit activeClientChanged(clientId);
}

QString ImageServerBridge::activeClient() const
{
    return m_activeClientId;
}

QString ImageServerBridge::activeClientAlias() const
{
    int idx = m_clientModel->indexOfClient(m_activeClientId);
    if (idx >= 0) {
        return m_clientModel->aliasAt(idx);
    }
    return QString();
}

void ImageServerBridge::setFps(int fps)
{
    if (m_activeClientId.isEmpty()) {
        emit eventOccurred(imagesocket::FpsFailedNoClient, QVariantMap());
        return;
    }

    imagesocket::control::ControlMessage msg;
    msg.set_type(imagesocket::control::SET_FPS);
    msg.set_fps(fps);

    std::string out;
    if (!msg.SerializeToString(&out)) {
        QVariantMap details;
        details["fps"] = fps;
        emit eventOccurred(imagesocket::FpsSendError, details);
        return;
    }

    QByteArray ba(out.data(), (int)out.size());
    if (!m_server->sendControlToClient(m_activeClientId, ba)) {
        QVariantMap details;
        details["fps"] = fps;
        details["client"] = m_activeClientId;
        emit eventOccurred(imagesocket::FpsSendError, details);
        return;
    }

    // Sucesso
    QVariantMap details;
    details["fps"] = fps;
    details["client"] = m_activeClientId;

    // Update current FPS and notify QML
    m_currentFps = fps;
    emit currentFpsChanged(m_currentFps);

    // Store configured FPS in client model
    if (m_clientModel) {
        m_clientModel->setClientConfiguredFps(m_activeClientId, fps);
    }

    emit eventOccurred(imagesocket::FpsApplied, details);
}

int ImageServerBridge::configuredFps() const {
    return m_configuredFps;
}

void ImageServerBridge::setConfiguredFps(int fps) {
    if (m_configuredFps == fps) return;
    m_configuredFps = fps;

    // Persist configured FPS
    if (m_settings) {
        m_settings->setValue("fps", m_configuredFps);
        m_settings->sync();
    }

    emit configuredFpsChanged(m_configuredFps);

    // If there's an active client, apply immediately
    if (!m_activeClientId.isEmpty()) {
        setFps(m_configuredFps);
    }
}

void ImageServerBridge::onClientConnected(const QString& clientId, const QHostAddress& address)
{
    Q_UNUSED(address);
    m_clientModel->addClient(clientId, QStringLiteral("Connected"));

    // Update connection state
    if (m_clientModel->rowCount() > 0) {
        setConnectionState(ConnectionState::ClientsConnected);
        setStatusMessage(QStringLiteral("Clients connected"));
    }

    // If there's no active client yet, make this client active
    if (m_activeClientId.isEmpty()) {
        setActiveClient(clientId);
    }

    // Prefer server-wide configured FPS if present; otherwise, reapply per-client configured FPS
    int idx = m_clientModel->indexOfClient(clientId);
    if (m_configuredFps > 0 && m_activeClientId == clientId) {
        setFps(m_configuredFps);
    } else if (idx >= 0) {
        int configured = m_clientModel->configuredFpsAt(idx);
        if (configured > 0 && m_activeClientId == clientId) {
            setFps(configured);
        }
    }
}

void ImageServerBridge::onControlMessageReceived(const QString& clientId, const QByteArray& serialized)
{
    imagesocket::control::ControlMessage msg;
    if (!msg.ParseFromArray(serialized.constData(), serialized.size())) {
        return;
    }

    // If client replied with an alias message, store it in the model
    if (msg.type() == imagesocket::control::ALIAS) {
        const std::string& r = msg.alias();
        if (!r.empty()) {
            QString lastAlias = m_clientModel->aliasAt(m_clientModel->indexOfClient(clientId));
            QString alias = QString::fromStdString(r);

            if(alias != lastAlias)
            {
                m_clientModel->setClientAlias(clientId, alias);
                qInfo() << "Set alias for" << clientId << "->" << alias;  
                
                if(clientId == m_activeClientId) {
                    emit activeClientAliasChanged(alias);
                }
            }
            
            // Emit connection event once alias is available
            QVariantMap details;
            details["clientId"] = clientId;
            details["alias"] = alias;
            emit eventOccurred(imagesocket::ClientConnected, details);
            
            // Also emit higher-level signal with alias for toast/UI
            emit clientConnectedWithAlias(clientId, alias);        
        }
    }
}

void ImageServerBridge::onSessionDisconnected(const QString& clientId)
{
    // Obtain alias before removing
    int idx = m_clientModel->indexOfClient(clientId);
    QString alias;
    if (idx >= 0) alias = m_clientModel->aliasAt(idx);

    // Remove client from model
    m_clientModel->removeClient(clientId);

    // Emit disconnection event with alias if available
    QVariantMap details;
    details["clientId"] = clientId;
    details["alias"] = alias.isEmpty() ? clientId : alias;
    emit eventOccurred(imagesocket::ClientDisconnected, details);

    // Also emit higher-level signal for UI/Toast
    emit clientDisconnectedWithAlias(clientId, details["alias"].toString());

    // If no clients left, clear active and notify UI
    if (m_clientModel->rowCount() == 0) {
        m_activeClientId.clear();
        setConnectionState(ConnectionState::NoClients);
        setStatusMessage(QStringLiteral("No clients available"));
        emit eventOccurred(imagesocket::NoClientsAvailable, QVariantMap());
        emit connectionLost();
        return;
    } 

    // If the disconnected client was the active one, promote the next available client
    if (m_activeClientId == clientId) {
        QString next = m_clientModel->clientIdAt(0);
        setActiveClient(next);
        return;
    }

    // If there is no active client but only one client remains, make it active
    if (m_activeClientId.isEmpty() && m_clientModel->rowCount() == 1) {
        QString remaining = m_clientModel->clientIdAt(0);
        setActiveClient(remaining);
    }
}

void ImageServerBridge::onFrameReceived(const QString& clientId, const QImage& frame)
{
    // Always record frame reception for measurement per-client
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_clientModel) {
        m_clientModel->recordFrameReceived(clientId, now);
    }

    // If the client is the active one, update receiving state and cache frame for display
    if (clientId != m_activeClientId){
        return; // ignore frames from non-active clients for display
    }

    // Update connection state to receiving frames
    setConnectionState(ConnectionState::ReceivingFrames);
    setStatusMessage(QStringLiteral("Receiving frames"));

    // Cache last frame for the image provider
    m_lastFrame = frame;
    m_frameId++;
    emit frameIdChanged(m_frameId);

    // Update active client measured FPS from the model
    int idx = m_clientModel ? m_clientModel->indexOfClient(clientId) : -1;
    if (idx >= 0) {
        int measured = m_clientModel->measuredFpsAt(idx);
        if (measured != m_activeClientMeasuredFps) {
            m_activeClientMeasuredFps = measured;
            emit activeClientMeasuredFpsChanged(m_activeClientMeasuredFps);
        }
    }

    // Notify QML/UI listeners
    emit newFrameReady(frame);
} 

void ImageServerBridge::onServerError(imagesocket::EventCode code, const QVariantMap &details)
{
    // Re-emit server-level errors to the UI (this ensures WebSocketServer serverError maps to UI events)
    qWarning() << "WebSocketServer error (code)" << static_cast<int>(code) << details;
    emit eventOccurred(code, details);
}

int ImageServerBridge::activeClientMeasuredFps() const
{
    return m_activeClientMeasuredFps;
}

void ImageServerBridge::recordFrameReceived(const QString& clientId)
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_clientModel) {
        m_clientModel->recordFrameReceived(clientId, now);
        int idx = m_clientModel->indexOfClient(clientId);
        if (idx >= 0) {
            int measured = m_clientModel->measuredFpsAt(idx);
            if (measured != m_activeClientMeasuredFps && clientId == m_activeClientId) {
                m_activeClientMeasuredFps = measured;
                emit activeClientMeasuredFpsChanged(m_activeClientMeasuredFps);
            }
        }
    }
}
