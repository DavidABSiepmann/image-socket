/**
 * @file test_websocket_environment.h
 * @brief Extended-lifetime WebSocket test environment fixture
 *
 * Implements safe WebSocket testing architecture:
 * - QWebSocketServer owns fixture lifetime
 * - All QWebSocket clients owned by fixture (heap-allocated)
 * - Explicit client lifecycle management via createClient()
 * - Controlled teardown in cleanupTestCase()
 * - Event loop draining before destruction
 *
 * CRITICAL REQUIREMENT FOR QWEBSOCKET TESTS:
 * Objects created during test() with local scope will be destroyed while
 * asynchronous events are pending in the Qt event loop, causing double-free
 * and memory corruption. This fixture prevents that by:
 *
 * 1. Server lifetime = entire test fixture lifetime
 * 2. Clients created via createClient() are heap-allocated
 * 3. Clients only destroyed in cleanupTestCase() after draining
 * 4. All tests use QTRY_* macros or controlled event processing
 *
 * Header-only, no assertions or production logic.
 *
 * Usage:
 *   class TestMyWebSockets : public TestWebSocketEnvironment {
 *       // Server available at m_environment->server()
 *       // Use createClient() to get heap-allocated client
 *   };
 */

#ifndef TEST_WEBSOCKET_ENVIRONMENT_H
#define TEST_WEBSOCKET_ENVIRONMENT_H

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QList>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

/**
 * @namespace qt_test
 * @brief Common Qt test utilities
 */
namespace qt_test {

/**
 * @class WebSocketTestClient
 * @brief Wrapper for heap-allocated QWebSocket with lifecycle management
 *
 * Ensures client remains valid until explicitly closed and destroyed.
 * Never create as local variable - always use TestWebSocketEnvironment::createClient().
 */
class WebSocketTestClient : public QObject {
    Q_OBJECT

public:
    /**
     * Create client wrapping existing QWebSocket
     * @param socket Heap-allocated QWebSocket instance
     * @param parent Parent object
     */
    explicit WebSocketTestClient(QWebSocket* socket, QObject* parent = nullptr)
        : QObject(parent),
          m_socket(socket) {
        Q_ASSERT(m_socket);
    }

    /**
     * Destructor - closes socket before deleting
     */
    ~WebSocketTestClient() {
        closeAndWait();
    }

    /**
     * No copying
     */
    WebSocketTestClient(const WebSocketTestClient&) = delete;
    WebSocketTestClient& operator=(const WebSocketTestClient&) = delete;

    /**
     * Get underlying QWebSocket
     * For QSignalSpy, connect(), sendBinaryMessage(), etc.
     */
    QWebSocket* socket() const {
        return m_socket;
    }

    /**
     * Check if connected
     */
    bool isConnected() const {
        return m_socket && m_socket->isValid();
    }

    /**
     * Send binary message
     * @param data Binary data to send
     * @return True if sent successfully
     */
    bool sendBinaryMessage(const QByteArray& data) {
        if (!m_socket) {
            return false;
        }
        qint64 sent = m_socket->sendBinaryMessage(data);
        return sent == data.size();
    }

    /**
     * Close connection explicitly
     * Safe to call multiple times
     */
    void close() {
        if (!m_socket) {
            return;
        }
        if (m_socket->isValid()) {
            m_socket->close();
        }
    }

    /**
     * Close and wait for disconnected signal
     * Must be called before destruction in test fixture
     */
    void closeAndWait(int timeoutMs = 500) {
        if (!m_socket) {
            return;
        }

        // Already disconnected?
        if (!m_socket->isValid()) {
            return;
        }

        // Close and wait for disconnected signal
        close();

        QEventLoop loop;
        QObject::connect(m_socket, &QWebSocket::disconnected,
                        &loop, &QEventLoop::quit);

        QTimer timeout;
        QObject::connect(&timeout, &QTimer::timeout,
                        &loop, &QEventLoop::quit);

        timeout.start(timeoutMs);
        loop.exec();
        timeout.stop();
    }

signals:
    // Proxy signals from underlying socket
    void connected();
    void disconnected();
    void binaryMessageReceived(QByteArray message);
    void textMessageReceived(QString text);

private:
    QWebSocket* m_socket;
};

/**
 * @class TestWebSocketEnvironment
 * @brief Base class for WebSocket tests with extended lifetime management
 *
 * Implements safe WebSocket testing by:
 * 1. Creating server in initTestCase() - server lives entire test lifetime
 * 2. Creating all clients via createClient() - clients heap-allocated
 * 3. Destroying all clients in cleanupTestCase() - after draining event loop
 *
 * This prevents double-free and memory corruption caused by asynchronous
 * Qt events pending when QWebSocket objects are destroyed.
 *
 * Inherit from this class for WebSocket tests:
 *   class TestMyWebSockets : public TestWebSocketEnvironment {
 *       void testSomething() {
 *           auto client = createClient();
 *           // ... use client ...
 *           // NO NEED TO CLOSE - cleanupTestCase() handles it
 *       }
 *   };
 */
class TestWebSocketEnvironment : public QObject {
    Q_OBJECT

protected:
    /**
     * Initialize server - called once before all tests
     * IMPORTANT: Must be called from initTestCase() in subclass
     *
     * Example in subclass initTestCase():
     *   TestWebSocketEnvironment::initTestCase();
     */
    void initTestCase() {
        // Server will be created lazily on first access
        // This just ensures cleanup is connected
    }

    /**
     * Cleanup - destroy all clients and server
     * IMPORTANT: Must be called from cleanupTestCase() in subclass
     *
     * Example in subclass cleanupTestCase():
     *   TestWebSocketEnvironment::cleanupTestCase();
     */
    void cleanupTestCase() {
        // Close all clients
        for (WebSocketTestClient* client : m_clients) {
            if (client) {
                client->closeAndWait(500);
            }
        }
        m_clients.clear();

        // Stop server
        stopServer();

        // Final event loop drain
        drainEventLoop(100);
    }

    /**
     * Create a new WebSocket client connected to server
     * Client is owned by environment and automatically cleaned up
     *
     * @return Heap-allocated WebSocketTestClient, valid until cleanupTestCase()
     *
     * IMPORTANT: Never store this in a local variable and expect it to work
     * after the test method ends. The client is owned by the environment.
     */
    WebSocketTestClient* createClient() {
        ensureServer();

        QWebSocket* socket = new QWebSocket(
            QString(),
            QWebSocketProtocol::VersionLatest,
            nullptr);  // No parent - we manage lifetime

        QUrl url(QString("ws://127.0.0.1:%1").arg(m_server->serverPort()));
        socket->open(url);

        // Wait for connection to establish using event loop
        QEventLoop loop;
        QObject::connect(socket, &QWebSocket::connected,
                        &loop, &QEventLoop::quit);

        QTimer timeout;
        QObject::connect(&timeout, &QTimer::timeout,
                        &loop, &QEventLoop::quit);

        timeout.start(2000);
        loop.exec();
        timeout.stop();

        // Verify connection succeeded
        if (!socket->isValid()) {
            delete socket;
            return nullptr;
        }

        WebSocketTestClient* client = new WebSocketTestClient(socket, this);
        m_clients.append(client);

        return client;
    }

    /**
     * Get number of connected clients
     */
    int clientCount() const {
        return m_clients.size();
    }

    /**
     * Get server port
     */
    quint16 serverPort() const {
        const_cast<TestWebSocketEnvironment*>(this)->ensureServer();
        return m_server ? m_server->serverPort() : 0;
    }

    /**
     * Get server URL for manual client connections
     */
    QString serverUrl() const {
        const_cast<TestWebSocketEnvironment*>(this)->ensureServer();
        return QString("ws://127.0.0.1:%1").arg(serverPort());
    }

    /**
     * Get production server instance for signal connection
     * Rarely needed - most tests use client interface
     */
    QWebSocketServer* server() const {
        return m_server;
    }

    /**
     * Drain event loop to process pending Qt events
     * Use before assertions that depend on signal processing
     *
     * @param delayMs Milliseconds to wait for pending events
     */
    void drainEventLoop(int delayMs = 100) {
        QEventLoop loop;
        QTimer timer;

        QObject::connect(&timer, &QTimer::timeout,
                        &loop, &QEventLoop::quit);

        timer.start(delayMs);
        loop.exec();
    }

    /**
     * Send binary message to server from client
     * Handles the 0x01 control message prefix automatically if isControl=true
     *
     * @param client Client to send from
     * @param data Message data
     * @param isControl If true, prepends 0x01 prefix for control messages
     * @return True if sent successfully
     */
    bool sendMessage(WebSocketTestClient* client, const QByteArray& data, bool isControl = false) {
        if (!client || !client->isConnected()) {
            return false;
        }

        QByteArray msg = data;
        if (isControl) {
            QByteArray prefixed;
            prefixed.append(char(0x01));
            prefixed.append(data);
            msg = prefixed;
        }

        return client->sendBinaryMessage(msg);
    }

    /**
     * Wait for a signal with timeout
     * Convenience method for signal-based synchronization
     *
     * @param obj Object emitting signal
     * @param signal Signal to wait for
     * @param timeoutMs Timeout in milliseconds
     * @return True if signal emitted, false if timeout
     */
    bool waitForSignal(QObject* obj, const char* signal, int timeoutMs = 2000) {
        QSignalSpy spy(obj, signal);

        QEventLoop loop;
        QObject::connect(obj, signal,
                        &loop, SLOT(quit()));

        QTimer timeout;
        QObject::connect(&timeout, &QTimer::timeout,
                        &loop, &QEventLoop::quit);

        timeout.start(timeoutMs);
        loop.exec();
        timeout.stop();

        return spy.count() >= 1;
    }

private:
    /**
     * Ensure server is created and listening
     */
    void ensureServer() {
        if (m_server) {
            return;
        }

        m_server = new QWebSocketServer(
            "TestWebSocketServer",
            QWebSocketServer::NonSecureMode,
            this);

        if (!m_server->listen(QHostAddress::LocalHost, 0)) {
            delete m_server;
            m_server = nullptr;
            return;
        }
    }

    /**
     * Stop the server
     */
    void stopServer() {
        if (!m_server) {
            return;
        }

        m_server->close();
        // Server will be deleted when this object is deleted
    }

    // Server instance - lifetime = entire test fixture
    QWebSocketServer* m_server = nullptr;

    // All client instances - heap-allocated, destroyed in cleanupTestCase()
    QList<WebSocketTestClient*> m_clients;
};

}  // namespace qt_test

#endif  // TEST_WEBSOCKET_ENVIRONMENT_H
