/**
 * @file test_websocket_messages.cpp
 * @brief Qt WebSocket tests - Binary message exchange with protobuf
 *
 * Tests WebSocket message communication:
 * - Client sends binary control messages
 * - Server receives and processes messages
 * - controlMessageReceived signal emitted
 * - Message parameters preserved correctly
 */

#include <QtTest/QtTest>
#include <QtCore/QObject>
#include <QtTest/QSignalSpy>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include "../fixtures/qt_test_base.h"
#include "network/websocketserver.h"
#include "control.pb.h"

/**
 * @class TestWebSocketMessages
 * @brief Tests for WebSocket message exchange
 */
class TestWebSocketMessages : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qt_test::initializeQtTestApp();
        qRegisterMetaType<QHostAddress>("QHostAddress");
    }

    /**
     * Test: Server accepts single control message
     * Verifies:
     * - Client connects successfully
     * - Control message (protobuf) sent with 0x01 prefix
     * - Server receives and emits controlMessageReceived signal
     * - Message parsed correctly
     */
    void testServerAcceptsControlMessages() {
        WebSocketServer server;
        bool started = server.start(0);
        QVERIFY(started);

        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));

        QSignalSpy spyMessage(&server, SIGNAL(controlMessageReceived(QString, QByteArray)));

        QWebSocket client;
        client.open(url);

        QTRY_VERIFY_WITH_TIMEOUT(client.isValid(), 2000);

        // Prepare protobuf message
        imagesocket::control::ControlMessage msg;
        msg.set_type(imagesocket::control::SET_FPS);
        msg.set_fps(30);
        msg.set_client_id(1);

        std::string serialized;
        bool serializeOk = msg.SerializeToString(&serialized);
        QVERIFY(serializeOk);

        // Send with 0x01 control prefix
        QByteArray msgData;
        msgData.append(char(0x01));
        msgData.append(serialized.data(), serialized.size());

        client.sendBinaryMessage(msgData);

        // Wait for server to receive and process
        QTRY_VERIFY_WITH_TIMEOUT(spyMessage.count() >= 1, 2000);

        // Verify message was parsed
        QVariantList args = spyMessage.takeFirst();
        QByteArray received = args.at(1).toByteArray();

        imagesocket::control::ControlMessage parsed;
        bool parseOk = parsed.ParseFromArray(received.data(), received.size());
        QVERIFY(parseOk);
        QCOMPARE(parsed.fps(), 30);

        client.close();
        server.stop();
    }

    /**
     * Test: Multiple sequential messages from single client
     * Verifies:
     * - Client sends 3+ messages sequentially
     * - All messages received by server
     * - Messages processed in order
     * - No message loss or corruption
     *
     * This test specifically validates multi-message robustness
     * which was a known failure point in previous implementation.
     */
    void testServerAcceptsMultipleMessages() {
        WebSocketServer server;
        bool started = server.start(0);
        QVERIFY(started);

        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));

        QSignalSpy spyMessage(&server, SIGNAL(controlMessageReceived(QString, QByteArray)));

        QWebSocket client;
        client.open(url);

        QTRY_VERIFY_WITH_TIMEOUT(client.isValid(), 2000);

        // Send 3 messages sequentially with different FPS values
        const QList<int> fpsValues = {24, 30, 60};

        for (int fps : fpsValues) {
            imagesocket::control::ControlMessage msg;
            msg.set_type(imagesocket::control::SET_FPS);
            msg.set_fps(fps);
            msg.set_client_id(1);

            std::string serialized;
            QVERIFY(msg.SerializeToString(&serialized));

            QByteArray msgData;
            msgData.append(char(0x01));
            msgData.append(serialized.data(), serialized.size());

            client.sendBinaryMessage(msgData);

            // Small delay between messages to ensure processing
            QTest::qWait(50);
        }

        // Wait for all 3 messages to arrive
        QTRY_VERIFY_WITH_TIMEOUT(spyMessage.count() >= 3, 3000);

        // Verify all messages received and values preserved
        QList<int> receivedFps;
        for (int i = 0; i < spyMessage.count(); ++i) {
            QVariantList args = spyMessage.at(i);
            QByteArray received = args.at(1).toByteArray();

            imagesocket::control::ControlMessage parsed;
            QVERIFY(parsed.ParseFromArray(received.data(), received.size()));
            receivedFps.append(parsed.fps());
        }

        QCOMPARE(receivedFps.count(), 3);
        QCOMPARE(receivedFps.at(0), 24);
        QCOMPARE(receivedFps.at(1), 30);
        QCOMPARE(receivedFps.at(2), 60);

        client.close();
        server.stop();
    }

    /**
     * Test: Protobuf SET_FPS message parameters preserved
     * Verifies:
     * - FPS value preserved through protobuf serialization/parsing
     * - Message type accessible
     * - Client ID accessible
     */
    void testSetFpsMessageParameters() {
        WebSocketServer server;
        bool started = server.start(0);
        QVERIFY(started);

        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));

        QSignalSpy spyMessage(&server, SIGNAL(controlMessageReceived(QString, QByteArray)));

        QWebSocket client;
        client.open(url);

        QTRY_VERIFY_WITH_TIMEOUT(client.isValid(), 2000);

        imagesocket::control::ControlMessage msg;
        msg.set_type(imagesocket::control::SET_FPS);
        msg.set_fps(24);
        msg.set_client_id(1);

        std::string serialized;
        QVERIFY(msg.SerializeToString(&serialized));

        QByteArray msgData;
        msgData.append(char(0x01));
        msgData.append(serialized.data(), serialized.size());

        client.sendBinaryMessage(msgData);

        QTRY_VERIFY_WITH_TIMEOUT(spyMessage.count() >= 1, 2000);

        QVariantList args = spyMessage.takeFirst();
        QByteArray received = args.at(1).toByteArray();

        imagesocket::control::ControlMessage parsed;
        QVERIFY(parsed.ParseFromArray(received.data(), received.size()));

        QCOMPARE(parsed.fps(), 24);
        QCOMPARE(parsed.type(), imagesocket::control::SET_FPS);
        QCOMPARE(parsed.client_id(), 1);

        client.close();
        server.stop();
    }

    /**
     * Test: Client disconnect and cleanup without crashes
     * Verifies:
     * - Client can be disconnected cleanly
     * - No memory corruption or double-free
     * - Server properly handles clientDisconnected signal
     */
    void testClientDisconnectCleanup() {
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

        // Explicitly disconnect client
        client.close();

        // Server should emit clientDisconnected signal
        QTRY_VERIFY_WITH_TIMEOUT(spyDisconnect.count() >= 1, 2000);

        // Verify no crashes occurred
        server.stop();
    }
};

QTEST_MAIN(TestWebSocketMessages)
#include "test_websocket_messages.moc"
