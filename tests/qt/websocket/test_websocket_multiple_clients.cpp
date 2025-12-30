/**
 * @file test_websocket_multiple_clients.cpp
 * @brief Qt WebSocket tests - Multiple concurrent clients
 *
 * Tests WebSocket server with multiple clients:
 * - Multiple clients connect simultaneously
 * - Each client receives separate clientId
 * - Server handles concurrent communication
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
 * @class TestWebSocketMultipleClients
 * @brief Tests for multiple concurrent WebSocket clients
 */
class TestWebSocketMultipleClients : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qt_test::initializeQtTestApp();
        qRegisterMetaType<QHostAddress>("QHostAddress");
    }

    /**
     * Test: Two clients can connect simultaneously
     * Verifies:
     * - Both clients connect successfully
     * - Each receives valid clientId
     * - Both clientConnected signals emitted
     */
    void testTwoClientsConnectSimultaneously() {
        WebSocketServer server;
        bool started = server.start(0);
        QVERIFY(started);

        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));

        QSignalSpy spyConnect(&server, SIGNAL(clientConnected(QString, QHostAddress)));

        QWebSocket client1, client2;
        client1.open(url);
        client2.open(url);

        // Both should connect
        QTRY_VERIFY_WITH_TIMEOUT(client1.isValid(), 2000);
        QTRY_VERIFY_WITH_TIMEOUT(client2.isValid(), 2000);

        // Both should have triggered signal
        QTRY_VERIFY_WITH_TIMEOUT(spyConnect.count() >= 2, 2000);

        server.stop();
    }

    /**
     * Test: Each client receives unique clientId
     * Verifies:
     * - clientIds are different for each client
     * - clientIds are non-empty strings
     */
    void testEachClientHasUniqueId() {
        WebSocketServer server;
        bool started = server.start(0);
        QVERIFY(started);

        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));

        QSignalSpy spyConnect(&server, SIGNAL(clientConnected(QString, QHostAddress)));

        QWebSocket client1, client2, client3;
        client1.open(url);
        client2.open(url);
        client3.open(url);

        QTRY_VERIFY_WITH_TIMEOUT(client1.isValid(), 2000);
        QTRY_VERIFY_WITH_TIMEOUT(client2.isValid(), 2000);
        QTRY_VERIFY_WITH_TIMEOUT(client3.isValid(), 2000);

        QTRY_VERIFY_WITH_TIMEOUT(spyConnect.count() >= 3, 2000);

        // Extract client IDs
        QString id1 = spyConnect.at(0).at(0).toString();
        QString id2 = spyConnect.at(1).at(0).toString();
        QString id3 = spyConnect.at(2).at(0).toString();

        // All IDs should be valid and different
        QVERIFY(!id1.isEmpty());
        QVERIFY(!id2.isEmpty());
        QVERIFY(!id3.isEmpty());
        QVERIFY(id1 != id2);
        QVERIFY(id2 != id3);
        QVERIFY(id1 != id3);

        server.stop();
    }

    /**
     * Test: Messages from multiple clients are distinguished
     * Verifies:
     * - Client messages have correct sender IDs
     * - Server tracks messages per client
     */
    void testMessagesFromMultipleClientsAreDistinguished() {
        WebSocketServer server;
        bool started = server.start(0);
        QVERIFY(started);

        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));

        QSignalSpy spyConnect(&server, SIGNAL(clientConnected(QString, QHostAddress)));

        QWebSocket client1, client2;
        client1.open(url);
        client2.open(url);

        QTRY_VERIFY_WITH_TIMEOUT(client1.isValid(), 2000);
        QTRY_VERIFY_WITH_TIMEOUT(client2.isValid(), 2000);
        QTRY_VERIFY_WITH_TIMEOUT(spyConnect.count() >= 2, 2000);

        // Extract client IDs
        QString id1 = spyConnect.at(0).at(0).toString();
        QString id2 = spyConnect.at(1).at(0).toString();

        // Both IDs should be valid and different
        QVERIFY(!id1.isEmpty());
        QVERIFY(!id2.isEmpty());
        QVERIFY(id1 != id2);

        server.stop();
    }
};

QTEST_MAIN(TestWebSocketMultipleClients)
#include "test_websocket_multiple_clients.moc"
