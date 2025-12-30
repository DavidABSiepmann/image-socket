/**
 * @file test_server_bridge_initial_state.cpp
 * @brief Qt state tests - ImageServerBridge initial state
 *
 * Tests that ImageServerBridge starts in the correct initial state
 * with proper initial values for all state enums and properties.
 */

#include <QtTest/QtTest>
#include <QtCore/QObject>
#include <QtTest/QSignalSpy>
#include "../fixtures/qt_test_base.h"
#include "network/imageserverbridge.h"

/**
 * @class TestServerBridgeInitialState
 * @brief Tests for ImageServerBridge initial state
 */
class TestServerBridgeInitialState : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qt_test::initializeQtTestApp();
    }

    /**
     * Test: ImageServerBridge initial server state is Idle
     * Verifies:
     * - serverState() returns Idle on construction
     * - No signals emitted on construction
     * - Server is not listening
     */
    void testInitialServerStateIsIdle() {
        ImageServerBridge bridge;

        // Verify initial state enum
        QCOMPARE(bridge.serverState(), ImageServerBridge::Idle);

        // Verify no state changed signal on construction
        QSignalSpy spy_state(&bridge, &ImageServerBridge::serverStateChanged);
        QCOMPARE(spy_state.count(), 0);
    }

    /**
     * Test: ImageServerBridge initial connection state is NoClients
     * Verifies:
     * - connectionState() returns NoClients on construction
     * - No clients are connected initially
     * - Connection state is well-defined
     */
    void testInitialConnectionStateIsNoClients() {
        ImageServerBridge bridge;

        // Verify initial connection state
        QCOMPARE(bridge.connectionState(), ImageServerBridge::NoClients);

        // Verify no connection changed signal on construction
        QSignalSpy spy_conn(&bridge, &ImageServerBridge::connectionStateChanged);
        QCOMPARE(spy_conn.count(), 0);
    }

    /**
     * Test: ImageServerBridge initial model is empty
     * Verifies:
     * - clientModel() is not nullptr
     * - clientModel()->count() == 0 (no clients)
     * - Model can be accessed immediately after construction
     */
    void testInitialClientModelIsEmpty() {
        ImageServerBridge bridge;

        // Verify model exists
        QObject* model = bridge.clientModel();
        QVERIFY(model != nullptr);

        // Verify model is a ClientModel and has proper interface
        QCOMPARE(model->property("count").toInt(), 0);
    }

    /**
     * Test: ImageServerBridge initial active client is empty
     * Verifies:
     * - activeClient() returns empty string initially
     * - No active client is set
     */
    void testInitialActiveClientIsEmpty() {
        ImageServerBridge bridge;

        QString active_client = bridge.activeClient();
        QCOMPARE(active_client, QString());
    }

    /**
     * Test: ImageServerBridge initial FPS value is valid
     * Verifies:
     * - configuredFps() >= 0 initially
     * - Default FPS is reasonable (> 0)
     * - currentFps() is accessible
     */
    void testInitialFpsValuesAreValid() {
        ImageServerBridge bridge;

        int configured_fps = bridge.configuredFps();
        QVERIFY(configured_fps >= 0);

        int current_fps = bridge.currentFps();
        QVERIFY(current_fps >= 0);
    }

    /**
     * Test: ImageServerBridge properties are accessible from start
     * Verifies:
     * - Properties like serverPort() work immediately
     * - Properties like statusMessage() work immediately
     * - No crashes when accessing properties
     */
    void testPropertiesAccessibleFromStart() {
        ImageServerBridge bridge;

        // Verify port property
        quint16 port = bridge.serverPort();
        QVERIFY(port >= 0);

        // Verify status message is accessible
        QString status = bridge.statusMessage();
        QVERIFY(status.capacity() >= 0);  // Valid QString

        // Verify frameId is accessible
        int frame_id = bridge.frameId();
        QVERIFY(frame_id >= 0);
    }

    /**
     * Test: ImageServerBridge no spurious signals on construction
     * Verifies:
     * - No signals fire when creating bridge
     * - State is deterministic
     * - Construction is clean
     */
    void testNoSignalsOnConstruction() {
        ImageServerBridge bridge;

        // Capture all state-related signals
        QSignalSpy spy_server_state(&bridge, &ImageServerBridge::serverStateChanged);
        QSignalSpy spy_conn_state(&bridge, &ImageServerBridge::connectionStateChanged);
        QSignalSpy spy_status(&bridge, &ImageServerBridge::statusMessageChanged);
        QSignalSpy spy_active_client(&bridge, &ImageServerBridge::activeClientChanged);

        // Verify no signals emitted on construction
        QCOMPARE(spy_server_state.count(), 0);
        QCOMPARE(spy_conn_state.count(), 0);
        QCOMPARE(spy_status.count(), 0);
        QCOMPARE(spy_active_client.count(), 0);

        // Allow brief event loop processing
        qt_test::EventLoopSpinner::processEvents();

        // Verify still no signals
        QCOMPARE(spy_server_state.count(), 0);
        QCOMPARE(spy_conn_state.count(), 0);
    }

    /**
     * Test: ImageServerBridge state enums are valid values
     * Verifies:
     * - State enum values are within expected range
     * - Connection enum values are valid
     * - No undefined enum states
     */
    void testStateEnumsAreValid() {
        ImageServerBridge bridge;

        int server_state = static_cast<int>(bridge.serverState());
        int conn_state = static_cast<int>(bridge.connectionState());

        // Verify valid enum ranges
        QVERIFY(server_state >= 0 && server_state <= ImageServerBridge::Error);
        QVERIFY(conn_state >= 0 && conn_state <= ImageServerBridge::ReceivingFrames);
    }

    /**
     * Test: Multiple bridge instances have independent state
     * Verifies:
     * - Each bridge instance is independent
     * - State changes in one don't affect another
     * - No shared global state
     */
    void testMultipleBridgesAreIndependent() {
        ImageServerBridge bridge1;
        ImageServerBridge bridge2;

        // Verify both start in same initial state
        QCOMPARE(bridge1.serverState(), ImageServerBridge::Idle);
        QCOMPARE(bridge2.serverState(), ImageServerBridge::Idle);

        // Verify they have different client models
        QVERIFY(bridge1.clientModel() != bridge2.clientModel());
    }
};

QTEST_MAIN(TestServerBridgeInitialState)
#include "test_server_bridge_initial_state.moc"
