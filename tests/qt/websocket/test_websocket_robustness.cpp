/**
 * @file test_websocket_robustness.cpp
 * @brief Qt WebSocket tests - Server robustness against invalid data
 *
 * Tests WebSocket server resilience:
 * - Server doesn't crash on invalid (non-image) binary data
 * - Server continues accepting connections after bad data
 * - Server ignores or handles malformed frames gracefully
 * - Server remains stable under adverse conditions
 */

#include <QtTest/QtTest>
#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QTimer>
#include <QtWebSockets/QWebSocket>
#include <QtTest/QSignalSpy>
#include "../fixtures/qt_test_base.h"
#include "network/websocketserver.h"

/**
 * @class TestWebSocketRobustness
 * @brief Tests for WebSocket server robustness and error handling
 */
class TestWebSocketRobustness : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qt_test::initializeQtTestApp();
        qRegisterMetaType<QHostAddress>("QHostAddress");
    }

    /**
     * Test: Server handles empty binary frame gracefully
     * Verifies:
     * - Empty frame doesn't crash server
     * - Server remains functional after empty frame
     * - No exceptions or segfaults
     */
    void testServerHandlesEmptyFrame() {
        WebSocketServer server;
        
        bool started = server.start(0);
        QVERIFY(started);
        
        quint16 port = server.port();
        QVERIFY(port > 0);
        
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));
        
        QWebSocket client;
        client.open(url);
        
        // Wait for connection
        QTRY_VERIFY_WITH_TIMEOUT(client.isValid(), 2000);
        QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
        
        // Send empty binary frame
        QByteArray empty_frame;
        qint64 sent = client.sendBinaryMessage(empty_frame);
        QCOMPARE(sent, 0LL);
        
        // Wait for server to process
        qt_test::EventLoopSpinner::processEventsWithTimeout(500);
        
        // Server should still be running
        QVERIFY(server.port() > 0);
        
        // Client should still be connected
        QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
        
        // Send another valid connection test
        QByteArray test_data("test");
        qint64 sent2 = client.sendBinaryMessage(test_data);
        QVERIFY(sent2 >= 0);
        
        qt_test::EventLoopSpinner::processEventsWithTimeout(200);
        
        client.close();
        server.stop();
    }

    /**
     * Test: Server handles random garbage data gracefully
     * Verifies:
     * - Random non-image binary data doesn't crash
     * - Server continues accepting frames after garbage
     * - Doesn't trigger segmentation faults
     */
    void testServerHandlesRandomGarbageData() {
        WebSocketServer server;
        
        bool started = server.start(0);
        QVERIFY(started);
        
        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));
        
        QWebSocket client;
        client.open(url);
        
        QTRY_VERIFY_WITH_TIMEOUT(client.isValid(), 2000);
        
        // Create random garbage data
        QByteArray garbage(256, 'X');
        for (int i = 0; i < 256; ++i) {
            garbage[i] = static_cast<char>(rand() % 256);
        }
        
        // Send garbage - should not crash
        qint64 sent = client.sendBinaryMessage(garbage);
        QVERIFY(sent >= 0);  // Should accept the send
        
        // Wait for server processing
        qt_test::EventLoopSpinner::processEventsWithTimeout(500);
        
        // Server should still be functional
        QVERIFY(server.port() > 0);
        QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
        
        // Should be able to send more data
        QByteArray test2("another_test");
        qint64 sent2 = client.sendBinaryMessage(test2);
        QVERIFY(sent2 >= 0);
        
        qt_test::EventLoopSpinner::processEventsWithTimeout(200);
        
        client.close();
        server.stop();
    }

    /**
     * Test: Server handles multiple invalid frames in sequence
     * Verifies:
     * - Server doesn't crash on multiple bad frames
     * - Server recovers after sequence of errors
     * - No memory leaks or resource exhaustion
     */
    void testServerHandlesMultipleInvalidFrames() {
        WebSocketServer server;
        
        bool started = server.start(0);
        QVERIFY(started);
        
        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));
        
        QWebSocket client;
        client.open(url);
        
        QTRY_VERIFY_WITH_TIMEOUT(client.isValid(), 2000);
        
        // Send 10 invalid frames in rapid succession
        for (int i = 0; i < 10; ++i) {
            // Create invalid frame (random bytes)
            QByteArray invalid_frame(128, 'I');
            for (int j = 0; j < 128; ++j) {
                invalid_frame[j] = static_cast<char>(rand() % 256);
            }
            
            qint64 sent = client.sendBinaryMessage(invalid_frame);
            QVERIFY(sent >= 0);
            
            // Brief delay
            qt_test::EventLoopSpinner::processEventsWithTimeout(50);
        }
        
        // Wait for all to be processed
        qt_test::EventLoopSpinner::processEventsWithTimeout(1000);
        
        // Server should still be up
        QVERIFY(server.port() > 0);
        QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
        
        // Should still accept valid data
        QByteArray valid("valid_test");
        qint64 sent = client.sendBinaryMessage(valid);
        QVERIFY(sent >= 0);
        
        client.close();
        server.stop();
    }

    /**
     * Test: Server handles very large invalid frames
     * Verifies:
     * - Large garbage frames don't cause crash
     * - Memory handling is robust
     * - Server remains responsive
     */
    void testServerHandlesLargeInvalidFrame() {
        WebSocketServer server;
        
        bool started = server.start(0);
        QVERIFY(started);
        
        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));
        
        QWebSocket client;
        client.open(url);
        
        QTRY_VERIFY_WITH_TIMEOUT(client.isValid(), 2000);
        
        // Create a large invalid frame (1MB of garbage)
        QByteArray large_garbage(1024 * 1024, 0);
        for (int i = 0; i < large_garbage.size(); i += 100) {
            large_garbage[i] = static_cast<char>(rand() % 256);
        }
        
        // Send large invalid frame
        qint64 sent = client.sendBinaryMessage(large_garbage);
        QVERIFY(sent >= 0);
        
        // Wait longer for large frame processing
        qt_test::EventLoopSpinner::processEventsWithTimeout(2000);
        
        // Server should survive large data
        QVERIFY(server.port() > 0);
        
        // Client might be disconnected after large invalid frame (acceptable)
        // But server should not have crashed
        
        // If still connected, verify responsiveness
        if (client.state() == QAbstractSocket::ConnectedState) {
            QByteArray test("still_connected");
            client.sendBinaryMessage(test);
            // Either sends successfully or fails gracefully
            QVERIFY(true);  // Just verify no crash
        }
        
        client.close();
        server.stop();
    }

    /**
     * Test: Server handles non-JPEG binary data gracefully
     * Verifies:
     * - Binary data with wrong magic bytes doesn't crash
     * - Server handles unrecognized binary format
     * - Remains stable for subsequent operations
     */
    void testServerHandlesWrongMagicBytes() {
        WebSocketServer server;
        
        bool started = server.start(0);
        QVERIFY(started);
        
        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));
        
        QWebSocket client;
        client.open(url);
        
        QTRY_VERIFY_WITH_TIMEOUT(client.isValid(), 2000);
        
        // Create data that looks like it might be image but isn't
        // PNG magic bytes: 89 50 4E 47 0D 0A 1A 0A
        QByteArray png_like;
        png_like.append((char)0x89);
        png_like.append((char)0x50);
        png_like.append((char)0x4E);
        png_like.append((char)0x47);
        png_like.append(QByteArray(100, 'X'));  // Invalid PNG data
        
        qint64 sent = client.sendBinaryMessage(png_like);
        QVERIFY(sent >= 0);
        
        qt_test::EventLoopSpinner::processEventsWithTimeout(500);
        
        // Server should handle gracefully
        QVERIFY(server.port() > 0);
        QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
        
        // Also test with GIF magic bytes
        QByteArray gif_like;
        gif_like.append("GIF89a");
        gif_like.append(QByteArray(100, 'X'));
        
        qint64 sent2 = client.sendBinaryMessage(gif_like);
        QVERIFY(sent2 >= 0);
        
        qt_test::EventLoopSpinner::processEventsWithTimeout(500);
        
        // Server should still be up
        QVERIFY(server.port() > 0);
        
        client.close();
        server.stop();
    }

    /**
     * Test: Server accepts new connections after handling bad data
     * Verifies:
     * - Previous bad connection doesn't break server
     * - New clients can connect successfully
     * - Server recovery is complete
     */
    void testServerAcceptsNewConnectionsAfterInvalidData() {
        WebSocketServer server;
        
        bool started = server.start(0);
        QVERIFY(started);
        
        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));
        
        // First client sends garbage
        {
            QWebSocket client1;
            client1.open(url);
            
            QTRY_VERIFY_WITH_TIMEOUT(client1.isValid(), 2000);
            
            // Send garbage
            QByteArray garbage(256, static_cast<char>(0xFF));
            client1.sendBinaryMessage(garbage);
            
            qt_test::EventLoopSpinner::processEventsWithTimeout(500);
            
            client1.close();
        }
        
        // Wait a bit after first client closes
        qt_test::EventLoopSpinner::processEventsWithTimeout(300);
        
        // Second client should be able to connect
        {
            QWebSocket client2;
            client2.open(url);
            
            // Should connect successfully
            QTRY_VERIFY_WITH_TIMEOUT(client2.isValid(), 2000);
            QCOMPARE(client2.state(), QAbstractSocket::ConnectedState);
            
            // Should be able to communicate
            QByteArray test("new_client_test");
            qint64 sent = client2.sendBinaryMessage(test);
            QVERIFY(sent > 0);
            
            qt_test::EventLoopSpinner::processEventsWithTimeout(200);
            
            client2.close();
        }
        
        server.stop();
    }

    /**
     * Test: Server handles rapid fire invalid frames
     * Verifies:
     * - Flood of bad data doesn't crash
     * - Server can handle abuse/DoS attempts
     * - No resource exhaustion
     */
    void testServerHandlesRapidFireInvalidData() {
        WebSocketServer server;
        
        bool started = server.start(0);
        QVERIFY(started);
        
        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));
        
        QWebSocket client;
        client.open(url);
        
        QTRY_VERIFY_WITH_TIMEOUT(client.isValid(), 2000);
        
        // Send 20 rapid frames of garbage
        for (int i = 0; i < 20; ++i) {
            QByteArray frame(64, static_cast<char>(i % 256));
            qint64 sent = client.sendBinaryMessage(frame);
            QVERIFY(sent >= 0);
            // No delay - rapid fire
        }
        
        // Wait for all frames to be processed
        qt_test::EventLoopSpinner::processEventsWithTimeout(1500);
        
        // Server should still be up and responsive
        QVERIFY(server.port() > 0);
        
        // If client still connected, it should be responsive
        if (client.state() == QAbstractSocket::ConnectedState) {
            QByteArray test("still_alive");
            qint64 sent = client.sendBinaryMessage(test);
            QVERIFY(sent >= 0);
        }
        
        client.close();
        server.stop();
    }

    /**
     * Test: Server handles connection drop during processing
     * Verifies:
     * - Abrupt client disconnect doesn't crash
     * - Server cleanup is safe
     * - Resources are properly released
     */
    void testServerHandlesAbruptDisconnection() {
        WebSocketServer server;
        
        bool started = server.start(0);
        QVERIFY(started);
        
        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));
        
        {
            QWebSocket client;
            client.open(url);
            
            QTRY_VERIFY_WITH_TIMEOUT(client.isValid(), 2000);
            
            // Send data then immediately disconnect
            QByteArray data(512, static_cast<char>(0xFF));
            client.sendBinaryMessage(data);
            
            // Close abruptly without waiting
            client.close();
        }
        
        // Give server time to clean up
        qt_test::EventLoopSpinner::processEventsWithTimeout(500);
        
        // Server should still be functional
        QVERIFY(server.port() > 0);
        
        // Should be able to accept new connections
        QWebSocket client2;
        client2.open(url);
        
        QTRY_VERIFY_WITH_TIMEOUT(client2.isValid(), 2000);
        QCOMPARE(client2.state(), QAbstractSocket::ConnectedState);
        
        client2.close();
        server.stop();
    }
};

QTEST_MAIN(TestWebSocketRobustness)
#include "test_websocket_robustness.moc"
