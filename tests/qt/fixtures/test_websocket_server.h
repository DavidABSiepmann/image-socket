/**
 * @file test_websocket_server.h
 * @brief Lightweight WebSocket server fixture for Qt tests
 *
 * Provides:
 * - Start/stop server helpers
 * - Automatic port selection (avoid hardcoded ports)
 * - Signal exposure for client connect/disconnect
 * - RAII-safe lifetime management
 *
 * Header-only, no assertions, no production logic
 *
 * Usage:
 *   WebSocketServerFixture server(5000);  // Use port 5000
 *   server.start();
 *   // ... clients can now connect ...
 *   server.stop();
 */

#ifndef TEST_WEBSOCKET_SERVER_H
#define TEST_WEBSOCKET_SERVER_H

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QList>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QtCore/QByteArray>

/**
 * @namespace qt_test
 * @brief Common Qt test utilities
 */
namespace qt_test {

/**
 * @class WebSocketServerFixture
 * @brief Test fixture for WebSocket server
 *
 * Lightweight server suitable for testing client connections.
 * NOT for production use.
 *
 * Features:
 * - Automatic port assignment (pass 0 for OS selection)
 * - Signal emission on client connect/disconnect
 * - Binary message reception
 * - RAII cleanup
 *
 * Signals emitted:
 * - clientConnected(QString clientId)
 * - clientDisconnected(QString clientId)
 * - messageReceived(QString clientId, QByteArray data)
 */
class WebSocketServerFixture : public QObject {
    Q_OBJECT

public:
    /**
     * Create WebSocket server fixture
     * @param port Port to listen on (0 = automatic OS assignment)
     * @param parent Parent object
     */
    explicit WebSocketServerFixture(quint16 port = 0, QObject* parent = nullptr)
        : QObject(parent),
          m_port(port),
          m_server(nullptr),
          m_client_counter(0) {
    }

    /**
     * Destructor ensures cleanup
     */
    ~WebSocketServerFixture() {
        stop();
    }

    /**
     * No copying
     */
    WebSocketServerFixture(const WebSocketServerFixture&) = delete;
    WebSocketServerFixture& operator=(const WebSocketServerFixture&) = delete;

    /**
     * Start the WebSocket server
     * @return True if started successfully
     */
    bool start() {
        if (m_server) {
            return false;  // Already started
        }

        m_server = new QWebSocketServer("TestServer", QWebSocketServer::NonSecureMode, this);

        // Connect to accept new connections
        connect(m_server, &QWebSocketServer::newConnection,
                this, &WebSocketServerFixture::onNewConnection);

        // Listen on specified port
        if (!m_server->listen(QHostAddress::LocalHost, m_port)) {
            delete m_server;
            m_server = nullptr;
            return false;
        }

        // Store actual port (in case 0 was passed for auto-assignment)
        m_port = m_server->serverPort();
        return true;
    }

    /**
     * Stop the WebSocket server
     */
    void stop() {
        if (!m_server) {
            return;
        }

        // Close all connected clients
        for (QWebSocket* socket : m_sockets) {
            if (socket) {
                socket->close();
            }
        }
        m_sockets.clear();

        // Close server
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }

    /**
     * Get server port
     * Useful when 0 was passed to constructor (OS assigns port)
     */
    quint16 port() const {
        return m_port;
    }

    /**
     * Get server address
     */
    QHostAddress address() const {
        return QHostAddress::LocalHost;
    }

    /**
     * Get full server URL for clients
     */
    QString serverUrl() const {
        return QString("ws://%1:%2").arg("127.0.0.1").arg(m_port);
    }

    /**
     * Check if server is running
     */
    bool isListening() const {
        return m_server && m_server->isListening();
    }

    /**
     * Get number of connected clients
     */
    int clientCount() const {
        return m_sockets.count();
    }

    /**
     * Send message to specific client
     * @param clientId Client identifier
     * @param data Binary data to send
     * @return True if sent successfully
     */
    bool sendToClient(const QString& clientId, const QByteArray& data) {
        QWebSocket* socket = findSocket(clientId);
        if (!socket) {
            return false;
        }

        qint64 sent = socket->sendBinaryMessage(data);
        return sent == data.size();
    }

    /**
     * Broadcast message to all connected clients
     * @param data Binary data to broadcast
     */
    void broadcast(const QByteArray& data) {
        for (QWebSocket* socket : m_sockets) {
            if (socket) {
                socket->sendBinaryMessage(data);
            }
        }
    }

    /**
     * Get list of connected client IDs
     */
    QList<QString> connectedClients() const {
        return m_client_ids;
    }

signals:
    /**
     * Emitted when new client connects
     * @param clientId Auto-assigned client identifier
     */
    void clientConnected(const QString& clientId);

    /**
     * Emitted when client disconnects
     * @param clientId Client identifier
     */
    void clientDisconnected(const QString& clientId);

    /**
     * Emitted when message received from client
     * @param clientId Client identifier
     * @param data Binary message data
     */
    void messageReceived(const QString& clientId, const QByteArray& data);

    /**
     * Emitted when text message received (rare in binary-only scenarios)
     * @param clientId Client identifier
     * @param text Text message
     */
    void textMessageReceived(const QString& clientId, const QString& text);

private slots:
    void onNewConnection() {
        if (!m_server) {
            return;
        }

        while (m_server->hasPendingConnections()) {
            QWebSocket* socket = m_server->nextPendingConnection();
            if (!socket) {
                continue;
            }

            // Generate client ID
            QString clientId = QString("client_%1").arg(++m_client_counter);
            m_client_ids.append(clientId);
            m_sockets.append(socket);

            // Connect signals
            connect(socket, &QWebSocket::binaryMessageReceived,
                    this, [this, clientId](const QByteArray& message) {
                        emit messageReceived(clientId, message);
                    });

            connect(socket, QOverload<const QString&>::of(&QWebSocket::textMessageReceived),
                    this, [this, clientId](const QString& text) {
                        emit textMessageReceived(clientId, text);
                    });

            connect(socket, &QWebSocket::disconnected,
                    this, [this, clientId, socket]() {
                        onClientDisconnected(clientId, socket);
                    });

            // Emit connected signal
            emit clientConnected(clientId);
        }
    }

    void onClientDisconnected(const QString& clientId, QWebSocket* socket) {
        m_client_ids.removeAll(clientId);
        m_sockets.removeAll(socket);
        socket->deleteLater();
        emit clientDisconnected(clientId);
    }

private:
    /**
     * Find socket by client ID
     */
    QWebSocket* findSocket(const QString& clientId) const {
        int index = m_client_ids.indexOf(clientId);
        if (index < 0 || index >= m_sockets.count()) {
            return nullptr;
        }
        return m_sockets.at(index);
    }

    quint16 m_port;
    QPointer<QWebSocketServer> m_server;
    QList<QString> m_client_ids;
    QList<QWebSocket*> m_sockets;
    int m_client_counter;
};

}  // namespace qt_test

#endif  // TEST_WEBSOCKET_SERVER_H
