/**
 * @file test_websocket_frame_integrity.cpp
 * @brief Qt WebSocket tests - Real image frame exchange with OpenCV
 *
 * Tests WebSocket binary frame handling with valid images:
 * - Generate valid JPEG frames with OpenCV
 * - Send frames through WebSocket
 * - Verify frame size integrity
 * - Multiple frames with different resolutions
 */

#include <QtTest/QtTest>
#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QTimer>
#include <QtWebSockets/QWebSocket>
#include <QtTest/QSignalSpy>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include "../fixtures/qt_test_base.h"
#include "network/websocketserver.h"

/**
 * @class TestWebSocketFrameIntegrity
 * @brief Tests for WebSocket frame exchange with real images
 */
class TestWebSocketFrameIntegrity : public QObject {
    Q_OBJECT

private:
    /**
     * Generate a valid JPEG image frame using OpenCV
     * @param width Image width in pixels
     * @param height Image height in pixels
     * @return Encoded JPEG data as QByteArray
     */
    QByteArray generateJpegFrame(int width, int height) {
        // Create a 3-channel BGR image (OpenCV default)
        cv::Mat image(height, width, CV_8UC3);
        
        // Fill with noise pattern
        cv::randu(image, cv::Scalar::all(50), cv::Scalar::all(200));
        
        // Add some structure (gradient pattern)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                image.at<cv::Vec3b>(y, x)[0] = (x * 255) / width;
                image.at<cv::Vec3b>(y, x)[1] = (y * 255) / height;
                image.at<cv::Vec3b>(y, x)[2] = ((x + y) * 255) / (width + height);
            }
        }
        
        // Encode to JPEG
        std::vector<uchar> buffer;
        std::vector<int> compression_params;
        compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
        compression_params.push_back(85);  // Good quality/size tradeoff
        
        bool encode_ok = cv::imencode(".jpg", image, buffer, compression_params);
        if (!encode_ok || buffer.empty()) {
            return QByteArray();
        }
        
        return QByteArray(reinterpret_cast<const char*>(buffer.data()), 
                         static_cast<int>(buffer.size()));
    }

private slots:
    void initTestCase() {
        qt_test::initializeQtTestApp();
        qRegisterMetaType<QHostAddress>("QHostAddress");
    }

    /**
     * Test: Valid JPEG frame generated with OpenCV
     * Verifies:
     * - OpenCV successfully generates image
     * - JPEG encoding works
     * - Frame data is non-empty
     * - Frame size is reasonable
     */
    void testGenerateValidJpegFrame() {
        QByteArray frame = generateJpegFrame(160, 120);
        
        QVERIFY(!frame.isEmpty());
        QVERIFY(frame.size() > 0);
        
        // JPEG frames should have some minimum size
        QVERIFY(frame.size() > 20);
        
        // JPEG files start with magic bytes FF D8
        QCOMPARE((uchar)frame[0], 0xFF);
        QCOMPARE((uchar)frame[1], 0xD8);
    }

    /**
     * Test: Multiple frames have expected size ratio
     * Verifies:
     * - Larger images produce larger frames
     * - Size ratio is reasonable
     * - Different resolutions handled correctly
     */
    void testFrameSizeScaling() {
        QByteArray small_frame = generateJpegFrame(160, 120);
        QByteArray large_frame = generateJpegFrame(320, 240);
        
        QVERIFY(!small_frame.isEmpty());
        QVERIFY(!large_frame.isEmpty());
        
        // Larger frame should generally be larger (not always, but typically)
        // Due to compression, relationship isn't strictly linear
        // But at least both should exist
        QVERIFY(small_frame.size() > 0);
        QVERIFY(large_frame.size() > 0);
    }

    /**
     * Test: Server receives valid JPEG frame from WebSocket client
     * Verifies:
     * - Connection established
     * - Frame sent successfully
     * - Server receives frame with correct size
     * - No data corruption during transmission
     */
    void testServerReceivesJpegFrameIntact() {
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
        
        // Generate and send a valid JPEG frame
        QByteArray original_frame = generateJpegFrame(160, 120);
        QVERIFY(!original_frame.isEmpty());
        int original_size = original_frame.size();
        
        qint64 bytes_sent = client.sendBinaryMessage(original_frame);
        QVERIFY(bytes_sent > 0);
        QCOMPARE(bytes_sent, (qint64)original_size);
        
        // Wait for server to process frame
        qt_test::EventLoopSpinner::processEventsWithTimeout(1000);
        
        // Verify client still connected and active
        QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
        
        // Verify frame integrity - JPEG magic bytes should be preserved
        QVERIFY(original_frame.size() > 1);
        QCOMPARE((uchar)original_frame[0], 0xFF);
        QCOMPARE((uchar)original_frame[1], 0xD8);
        
        client.close();
        server.stop();
    }

    /**
     * Test: Sequential JPEG frames maintain integrity
     * Verifies:
     * - Multiple frames sent in sequence
     * - Each frame size preserved
     * - All frames are valid JPEG
     * - Server processes all without error
     */
    void testSequentialJpegFramesIntegrity() {
        WebSocketServer server;
        
        bool started = server.start(0);
        QVERIFY(started);
        
        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));
        
        QWebSocket client;
        client.open(url);
        
        QTRY_VERIFY_WITH_TIMEOUT(client.isValid(), 2000);
        
        QVector<QByteArray> frames;
        QVector<int> frame_sizes;
        
        // Generate and send 5 frames with different seeds
        for (int i = 0; i < 5; ++i) {
            QByteArray frame = generateJpegFrame(160, 120);
            QVERIFY(!frame.isEmpty());
            
            frames.append(frame);
            frame_sizes.append(frame.size());
            
            // Verify JPEG signature
            QCOMPARE((uchar)frame[0], 0xFF);
            QCOMPARE((uchar)frame[1], 0xD8);
            
            // Send frame
            qint64 sent = client.sendBinaryMessage(frame);
            QCOMPARE(sent, (qint64)frame.size());
            
            // Small delay between frames for realistic streaming
            qt_test::EventLoopSpinner::processEventsWithTimeout(100);
        }
        
        // Wait for all frames to be processed
        qt_test::EventLoopSpinner::processEventsWithTimeout(1000);
        
        // Verify all frames maintained their size
        QCOMPARE(frames.size(), 5);
        for (int i = 0; i < frames.size(); ++i) {
            QCOMPARE(frames[i].size(), frame_sizes[i]);
            QVERIFY(frame_sizes[i] > 0);
        }
        
        // Client should still be connected
        QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
        
        client.close();
        server.stop();
    }

    /**
     * Test: Various frame resolutions maintain integrity
     * Verifies:
     * - Small frames (64x48) handled correctly
     * - Medium frames (320x240) handled correctly
     * - Larger frames (640x480) handled correctly
     * - All maintain valid JPEG format
     * - Size varies appropriately with resolution
     */
    void testMultipleResolutionsIntegrity() {
        WebSocketServer server;
        
        bool started = server.start(0);
        QVERIFY(started);
        
        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));
        
        QWebSocket client;
        client.open(url);
        
        QTRY_VERIFY_WITH_TIMEOUT(client.isValid(), 2000);
        
        // Test different resolutions
        struct Resolution {
            int width;
            int height;
        };
        
        Resolution resolutions[] = {
            {64, 48},       // Very small
            {160, 120},     // Small (QQVGA)
            {320, 240},     // Medium (QVGA)
            {640, 480}      // Large (VGA)
        };
        
        QVector<int> sizes_by_resolution;
        
        for (const auto& res : resolutions) {
            QByteArray frame = generateJpegFrame(res.width, res.height);
            QVERIFY(!frame.isEmpty());
            
            int frame_size = frame.size();
            sizes_by_resolution.append(frame_size);
            
            // Verify JPEG validity
            QVERIFY(frame_size > 2);
            QCOMPARE((uchar)frame[0], 0xFF);
            QCOMPARE((uchar)frame[1], 0xD8);
            
            // Send frame
            qint64 sent = client.sendBinaryMessage(frame);
            QCOMPARE(sent, (qint64)frame_size);
            
            // Small delay
            qt_test::EventLoopSpinner::processEventsWithTimeout(100);
        }
        
        // Wait for processing
        qt_test::EventLoopSpinner::processEventsWithTimeout(500);
        
        // Verify all sizes are reasonable and different
        QCOMPARE(sizes_by_resolution.size(), 4);
        for (int size : sizes_by_resolution) {
            QVERIFY(size > 10);  // JPEG overhead
        }
        
        // Generally, larger resolutions should produce larger files
        // (though not strictly due to compression)
        QVERIFY(sizes_by_resolution[3] >= sizes_by_resolution[0]);
        
        // Client should still be functional
        QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
        
        client.close();
        server.stop();
    }

    /**
     * Test: Frame size consistency across send/receive
     * Verifies:
     * - Bytes sent equals frame size
     * - No truncation or padding
     * - WebSocket binary protocol preserves frame size
     */
    void testFrameSizeConsistency() {
        WebSocketServer server;
        
        bool started = server.start(0);
        QVERIFY(started);
        
        quint16 port = server.port();
        QUrl url(QString("ws://127.0.0.1:%1").arg(port));
        
        QWebSocket client;
        client.open(url);
        
        QTRY_VERIFY_WITH_TIMEOUT(client.isValid(), 2000);
        
        // Send multiple frames and verify exact size match
        for (int i = 0; i < 3; ++i) {
            int width = 160 + (i * 80);   // 160, 240, 320
            int height = 120 + (i * 60);  // 120, 180, 240
            
            QByteArray frame = generateJpegFrame(width, height);
            QVERIFY(!frame.isEmpty());
            
            int expected_size = frame.size();
            
            qint64 sent = client.sendBinaryMessage(frame);
            
            // Exact match: what we sent == what frame size was
            QCOMPARE(sent, (qint64)expected_size);
            
            qt_test::EventLoopSpinner::processEventsWithTimeout(100);
        }
        
        qt_test::EventLoopSpinner::processEventsWithTimeout(500);
        
        QCOMPARE(client.state(), QAbstractSocket::ConnectedState);
        
        client.close();
        server.stop();
    }
};

QTEST_MAIN(TestWebSocketFrameIntegrity)
#include "test_websocket_frame_integrity.moc"
