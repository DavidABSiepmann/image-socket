#include "websocketserver.h"
#include "clientsession.h"
#include "control.pb.h"
#include <QWebSocketServer>
#include <QWebSocket>
#include <QUuid>
#include <QDebug>
#include <QTimer>

WebSocketServer::WebSocketServer(QObject* parent)
    : QObject(parent)
{
}

WebSocketServer::~WebSocketServer()
{
    stop();
}

bool WebSocketServer::start(quint16 port)
{
    if (m_server)
        return false; // already started
    
    quint16 actualPort = port;
    qDebug() << "Starting WebSocketServer on port" << actualPort;

    m_server = new QWebSocketServer(QStringLiteral("ImageSocketServer"), QWebSocketServer::NonSecureMode, this);

    if (!m_server->listen(QHostAddress::Any, port)) {
        actualPort = m_server->serverPort(); // get assigned port if 0 was given
    
        QString err = QStringLiteral("Failed to start WebSocketServer on port %1").arg(actualPort);
        qCritical() << err;
        QVariantMap details;
        details["port"] = actualPort;
        details["reason"] = err;
        emit serverError(imagesocket::ServerStartFailed, details);
        delete m_server;
        m_server = nullptr;
        return false;
    }

    connect(m_server, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);

    QTimer::singleShot(200, [this]() {
        qInfo() << "WebSocketServer started on port" << m_server->serverPort();
    });

    return true;
}

void WebSocketServer::stop()
{
    if (!m_server)
        return;

    m_server->close();
    delete m_server;
    m_server = nullptr;
    qInfo() << "WebSocketServer stopped";
}

quint16 WebSocketServer::port() const
{
    return m_server ? m_server->serverPort() : 0;
}

void WebSocketServer::onNewConnection()
{
    if (!m_server)
        return;

    QWebSocket* socket = m_server->nextPendingConnection();
    if (!socket)
        return;

    const QHostAddress addr = socket->peerAddress();

    // Create session and manage its lifecycle
    ClientSession* session = new ClientSession(socket, this);
    m_sessions.append(QPointer<ClientSession>(session));

    // Forward session events
    connect(session, &ClientSession::disconnected, this, &WebSocketServer::onSessionDisconnected);
    connect(session, &ClientSession::controlMessageReceived, this, [this](const QString& clientId, const QByteArray& serialized){
        emit controlMessageReceived(clientId, serialized);
    });
    connect(session, &ClientSession::frameReceived, this, [this](const QString& clientId, const QImage& img){
        emit frameReceived(clientId, img);
    });

    qInfo() << "Accepted new WebSocket connection from" << addr.toString() << "id=" << session->id();

    emit clientConnected(session->id(), addr);

    // Request alias from newly connected client
    imagesocket::control::ControlMessage req;
    req.set_type(imagesocket::control::REQUEST_ALIAS);
    std::string out;
    if (req.SerializeToString(&out)) {
        QByteArray ba(out.data(), (int)out.size());
        session->sendControlMessage(ba);
    } else {
        qWarning() << "Failed to serialize alias request";
    }
}

void WebSocketServer::onSessionDisconnected(const QString& clientId)
{
    // Remove session with matching id
    for (int i = 0; i < m_sessions.size(); ++i) {
        QPointer<ClientSession> sPtr = m_sessions.at(i);
        ClientSession* s = sPtr.data();
        if (s && s->id() == clientId) {
            qInfo() << "Removing session" << clientId;
            m_sessions.removeAt(i);
            s->deleteLater();
            emit clientDisconnected(clientId);
            return;
        }
    }
    qWarning() << "Disconnected session not found:" << clientId;
}

bool WebSocketServer::sendControlToClient(const QString& clientId, const QByteArray& serialized)
{
    for (int i = 0; i < m_sessions.size(); ++i) {
        ClientSession* s = m_sessions.at(i).data();
        if (s && s->id() == clientId) {
            s->sendControlMessage(serialized);
            return true;
        }
    }
    qWarning() << "sendControlToClient: client not found" << clientId;
    return false;
}
