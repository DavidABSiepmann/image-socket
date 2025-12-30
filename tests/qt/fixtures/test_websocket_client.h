/**
 * @file test_websocket_client.h
 * @brief Lightweight WebSocket client fixture for Qt tests
 *
 * Provides:
 * - Connect/disconnect helpers
 * - Send binary message helper
 * - Signal exposure for connection state
 * - Optional protobuf message send helper
 *
 * Header-only, no assertions, no production logic
 *
 * Usage:
 *   WebSocketClientFixture client;
 *   client.connect("ws://127.0.0.1:5000");
 *   // ... test server response ...
 *   client.disconnect();
 */

#ifndef TEST_WEBSOCKET_CLIENT_H
#define TEST_WEBSOCKET_CLIENT_H

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QByteArray>
#include <QtWebSockets/QWebSocket>
#include <QtCore/QUrl>

/**
 * @namespace qt_test
 * @brief Common Qt test utilities
 */
namespace qt_test {

/**
 * @class WebSocketClientFixture
 * @brief Test fixture for WebSocket client
 *
 * Lightweight client suitable for testing server interactions.
 * NOT for production use.
 *
 * Features:
 * - Non-blocking connect (uses Qt signals)
 * - Binary message sending
 * - Message reception with signals
 * - Connection state tracking
 * - RAII cleanup
 *
 * Signals emitted:
 * - connected()
 * - disconnected()
 * - binaryMessageReceived(QByteArray data)
 * - textMessageReceived(QString text)
 * - errorOccurred(QString error)
 */
class WebSocketClientFixture : public QObject {
    Q_OBJECT

public:
    /**
     * Create WebSocket client fixture
     * @param parent Parent object
     */
    explicit WebSocketClientFixture(QObject* parent = nullptr)
        : QObject(parent),
          m_socket(nullptr) {
    }

    /**
     * Destructor ensures cleanup
     */
    ~WebSocketClientFixture() {
        disconnect();
    }

    /**
     * No copying
     */
    WebSocketClientFixture(const WebSocketClientFixture&) = delete;
    WebSocketClientFixture& operator=(const WebSocketClientFixture&) = delete;

    /**
     * Connect to WebSocket server
     * @param url Server URL (e.g., "ws://127.0.0.1:5000")
     */
    void connect(const QString& url) {
        connect(QUrl(url));
    }

    /**
     * Connect to WebSocket server (QUrl overload)
     * @param url Server URL as QUrl
     */
    void connect(const QUrl& url) {
        if (m_socket) {
            return;  // Already connected or connecting
        }

        m_socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

        // Connect signals
        QObject::connect(m_socket, &QWebSocket::connected,
                        this, &WebSocketClientFixture::onConnected);

        QObject::connect(m_socket, &QWebSocket::disconnected,
                        this, &WebSocketClientFixture::onDisconnected);

        QObject::connect(m_socket, &QWebSocket::binaryMessageReceived,
                        this, &WebSocketClientFixture::binaryMessageReceived);

        QObject::connect(m_socket, QOverload<const QString&>::of(&QWebSocket::textMessageReceived),
                        this, &WebSocketClientFixture::textMessageReceived);

        QObject::connect(m_socket,
                        QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
                        this, &WebSocketClientFixture::onError);

        // Open connection
        m_socket->open(url);
    }

    /**
     * Disconnect from WebSocket server
     */
    void disconnect() {
        if (!m_socket) {
            return;
        }

        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }

    /**
     * Send binary message to server
     * @param data Binary data to send
     * @return True if sent successfully
     */
    bool sendBinaryMessage(const QByteArray& data) {
        if (!m_socket || !isConnected()) {
            return false;
        }

        qint64 sent = m_socket->sendBinaryMessage(data);
        return sent == data.size();
    }

    /**
     * Send text message to server
     * @param text Text message to send
     * @return True if sent successfully
     */
    bool sendTextMessage(const QString& text) {
        if (!m_socket || !isConnected()) {
            return false;
        }

        qint64 sent = m_socket->sendTextMessage(text);
        return sent == text.length();
    }

    /**
     * Check if connected to server
     */
    bool isConnected() const {
        return m_socket && m_socket->isValid();
    }

    /**
     * Get connection URL
     */
    QUrl url() const {
        if (!m_socket) {
            return QUrl();
        }
        return m_socket->requestUrl();
    }

    /**
     * Get last error message
     */
    QString lastError() const {
        if (!m_socket) {
            return QString();
        }
        return m_socket->errorString();
    }

    /**
     * Get WebSocket object (for advanced usage)
     * WARNING: Direct manipulation may break test fixture assumptions
     */
    QWebSocket* socket() const {
        return m_socket;
    }

signals:
    /**
     * Emitted when connected to server
     */
    void connected();

    /**
     * Emitted when disconnected from server
     */
    void disconnected();

    /**
     * Emitted when binary message received from server
     * @param data Binary message data
     */
    void binaryMessageReceived(const QByteArray& data);

    /**
     * Emitted when text message received from server
     * @param text Text message
     */
    void textMessageReceived(const QString& text);

    /**
     * Emitted when connection error occurs
     * @param error Error message
     */
    void errorOccurred(const QString& error);

private slots:
    void onConnected() {
        emit connected();
    }

    void onDisconnected() {
        emit disconnected();
    }

    void onError(QAbstractSocket::SocketError error) {
        QString msg = QString("Socket error %1: %2")
            .arg(static_cast<int>(error))
            .arg(m_socket ? m_socket->errorString() : "Unknown");
        emit errorOccurred(msg);
    }

private:
    QPointer<QWebSocket> m_socket;
};

}  // namespace qt_test

#endif  // TEST_WEBSOCKET_CLIENT_H
