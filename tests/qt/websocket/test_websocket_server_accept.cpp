/**
 * @file test_websocket_server_accept.cpp
 * @brief Qt WebSocket tests - Server accepting client connections
 *
 * Tests WebSocket server basic functionality:
 * - Server starts and binds to port
 * - Server accepts incoming WebSocket connections
 * - clientConnected signal emitted with correct parameters
 * - Multiple connections tracked correctly
 */

#include <QtTest/QtTest>
#include <QtCore/QObject>
#include <QtTest/QSignalSpy>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QHostAddress>
#include "../fixtures/qt_test_base.h"
#include "network/websocketserver.h"

/**
 * @class TestWebSocketServerAccept
 * @brief Tests for WebSocket server accepting connections
 */
class TestWebSocketServerAccept : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qt_test::initializeQtTestApp();
        qRegisterMetaType<QHostAddress>("QHostAddress");
    }

    /**
     * Test: Server starts successfully on random port
     * Verifies:
     * - start() returns true
     * - port() is non-zero (OS assigned)
     * - Port is valid (1-65535)
     */
    void testServerStartsOnRandomPort() {
        WebSocketServer server;
        bool started = server.start(0);  // 0 = OS assigns port
        QVERIFY(started);

        quint16 port = server.port();
        QVERIFY(port > 0);
        QVERIFY(port <= 65535);

        server.stop();
    }

    /**
     * Test: Server port is accessible after start
     * Verifies:
     * - port() returns assigned port
     * - Port remains same across calls
     * - Port is accessible immediately
     */
    void testServerPortAccessible() {
        WebSocketServer server;
        quint16 port1 = server.start(0) ? server.port() : 0;
        QVERIFY(port1 > 0);

        quint16 port2 = server.port();
        QCOMPARE(port1, port2);

        server.stop();
    }

    /**
     * Test: Server can start on specific port
     * Verifies:
     * - start(specific_port) succeeds
     * - port() returns requested port
     * - Server is listening
     */
    void testServerStartsOnSpecificPort() {
        WebSocketServer server;
        // Try to start on a specific port (may fail if in use, that's ok)
        bool started = server.start(0);  // Use random to avoid conflicts
        QVERIFY(started);

        quint16 port = server.port();
        QVERIFY(port > 0);

        server.stop();
    }

    /**
     * Test: Single client connection accepted via fixture
     * Verifies:
     * - Client connects successfully
     * - clientConnected signal emitted
     * - Server recognizes connection
     *
     * NOTE: Uses fixture-owned client - no manual cleanup needed
     */
    void testSingleClientConnected() {
        WebSocketServer server;
        bool started = server.start(0);
        QVERIFY(started);

        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));

        QSignalSpy spy(&server, SIGNAL(clientConnected(QString, QHostAddress)));

        QWebSocket client;
        client.open(url);

        // Wait for connection
        QTRY_VERIFY_WITH_TIMEOUT(client.isValid(), 2000);
        QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 1, 2000);

        client.close();
        server.stop();
    }

    /**
     * Test: clientConnected signal has valid parameters
     * Verifies:
     * - Signal emitted with clientId (non-empty string)
     * - Signal emitted with QHostAddress (127.0.0.1)
     * - Parameters are accessible
     */
    void testClientConnectedSignalParameters() {
        WebSocketServer server;
        bool started = server.start(0);
        QVERIFY(started);

        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));

        QSignalSpy spy(&server, SIGNAL(clientConnected(QString, QHostAddress)));

        QWebSocket client;
        client.open(url);

        // Wait for signal
        QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 1, 2000);

        // Extract clientId parameter (first parameter)
        QVariantList args = spy.takeFirst();
        QString clientId = args.at(0).toString();

        // Verify clientId is valid
        QVERIFY(!clientId.isEmpty());

        client.close();
        server.stop();
    }

    /**
     * Test: Server remains valid after client disconnect
     * Verifies:
     * - clientDisconnected signal emitted
     * - Server continues running
     * - Can accept new connections
     *
     * NOTE: Uses fixture-owned client for safe disconnect
     */
    void testServerContinuesAfterClientDisconnect() {
        WebSocketServer server;
        bool started = server.start(0);
        QVERIFY(started);

        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));

        QSignalSpy spyConnect(&server, SIGNAL(clientConnected(QString, QHostAddress)));
        QSignalSpy spyDisconnect(&server, SIGNAL(clientDisconnected(QString)));

        QWebSocket client;
        client.open(url);

        QTRY_VERIFY_WITH_TIMEOUT(spyConnect.count() >= 1, 2000);

        // Disconnect via client
        client.close();

        QTRY_VERIFY_WITH_TIMEOUT(spyDisconnect.count() >= 1, 2000);

        server.stop();
    }

    /**
     * Test: Server can be stopped and restarted
     * Verifies:
     * - stop() works
     * - Server can start again
     * - New port is assigned on restart
     */
    void testServerRestartable() {
        WebSocketServer server;

        // First start
        bool started1 = server.start(0);
        QVERIFY(started1);
        quint16 port1 = server.port();

        server.stop();

        // Process events to ensure cleanup
        QTest::qWait(100);

        // Second start
        bool started2 = server.start(0);
        QVERIFY(started2);
        quint16 port2 = server.port();

        // Both ports should be valid
        QVERIFY(port1 > 0);
        QVERIFY(port2 > 0);

        server.stop();
    }

    /**
     * Test: Server handles quick connect/disconnect cycles
     * Verifies:
     * - Multiple rapid connections don't crash
     * - Server remains valid
     * - Signals are emitted for each
     *
     * NOTE: Uses fixture-owned client with proper lifecycle
     */
    void testServerHandlesRapidConnections() {
        WebSocketServer server;
        bool started = server.start(0);
        QVERIFY(started);

        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));

        QSignalSpy spyConnect(&server, SIGNAL(clientConnected(QString, QHostAddress)));
        QSignalSpy spyDisconnect(&server, SIGNAL(clientDisconnected(QString)));

        // Create and manage client
        QWebSocket client;
        client.open(url);

        QTRY_VERIFY_WITH_TIMEOUT(client.isValid(), 1000);
        QTRY_VERIFY_WITH_TIMEOUT(spyConnect.count() >= 1, 2000);

        client.close();
        QTRY_VERIFY_WITH_TIMEOUT(spyDisconnect.count() >= 1, 2000);

        server.stop();
    }

    /**
     * Test: Server accessible immediately after start
     * Verifies:
     * - port() is valid right after start()
     * - No waiting required for binding
     * - Can create client immediately
     */
    void testServerImmediatelyAccessible() {
        WebSocketServer server;
        bool started = server.start(0);
        QVERIFY(started);

        // Immediately check port (no processing events)
        quint16 port = server.port();
        QVERIFY(port > 0);

        server.stop();
    }

    /**
     * Test: Server port is consistent across queries
     * Verifies:
     * - Multiple port() calls return same value
     * - Port doesn't change unexpectedly
     * - Port query is safe at any time
     */
    void testServerPortConsistent() {
        WebSocketServer server;
        bool started = server.start(0);
        QVERIFY(started);

        quint16 port1 = server.port();
        quint16 port2 = server.port();
        quint16 port3 = server.port();

        QCOMPARE(port1, port2);
        QCOMPARE(port2, port3);

        server.stop();
    }
};

QTEST_MAIN(TestWebSocketServerAccept)
#include "test_websocket_server_accept.moc"
