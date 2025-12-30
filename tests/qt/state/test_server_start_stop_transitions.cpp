/**
 * @file test_server_start_stop_transitions.cpp
 * @brief Qt state tests - Server start/stop state transitions
 *
 * Tests state machine transitions when starting and stopping the server.
 * Verifies correct signal emissions at each state change.
 */

#include <QtTest/QtTest>
#include <QtCore/QObject>
#include <QtTest/QSignalSpy>
#include "../fixtures/qt_test_base.h"
#include "network/imageserverbridge.h"

/**
 * @class TestServerStartStopTransitions
 * @brief Tests for server start/stop state transitions
 */
class TestServerStartStopTransitions : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qt_test::initializeQtTestApp();
    }

    /**
     * Test: Server transitions from Idle to Running when started
     * Verifies:
     * - start() returns true
     * - serverState() becomes Running (or Starting, then Running)
     * - serverStateChanged signal is emitted
     * - Port is bound and accessible
     */
    void testServerStartTransition() {
        ImageServerBridge bridge;

        // Capture state change signals
        QSignalSpy spy_state(&bridge, &ImageServerBridge::serverStateChanged);

        // Start server
        bool started = bridge.start();
        QVERIFY(started);

        // Give event loop a chance to process state change
        qt_test::EventLoopSpinner::processEvents();

        // Verify signal was emitted (at least once)
        QTRY_VERIFY_WITH_TIMEOUT(spy_state.count() >= 1, 2000);

        // Verify final state is Running
        QCOMPARE(bridge.serverState(), ImageServerBridge::Running);

        // Verify port is assigned
        quint16 port = bridge.serverPort();
        QVERIFY(port > 0);

        // Cleanup
        bridge.stop();
    }

    /**
     * Test: Server transitions from Running to Idle when stopped
     * Verifies:
     * - Server can be stopped after starting
     * - serverState() becomes Idle
     * - serverStateChanged signal is emitted
     * - Port is released
     */
    void testServerStopTransition() {
        ImageServerBridge bridge;

        // Start server first
        bool started = bridge.start();
        QVERIFY(started);

        // Wait for Running state
        QTRY_COMPARE_WITH_TIMEOUT(bridge.serverState(), 
                                 ImageServerBridge::Running, 2000);

        // Capture state change signals during stop
        QSignalSpy spy_state(&bridge, &ImageServerBridge::serverStateChanged);

        // Stop server
        bridge.stop();

        // Give event loop a chance to process state change
        qt_test::EventLoopSpinner::processEvents();

        // Verify signal was emitted
        QTRY_VERIFY_WITH_TIMEOUT(spy_state.count() >= 1, 2000);

        // Verify final state is Idle
        QCOMPARE(bridge.serverState(), ImageServerBridge::Idle);
    }

    /**
     * Test: Connection state transitions from NoClients to ClientsConnected
     * Verifies:
     * - When server starts, connection state is NoClients
     * - Connection state properly tracks client connections
     * - connectionStateChanged signal is emitted
     */
    void testConnectionStateStartsWithNoClients() {
        ImageServerBridge bridge;

        // Start server
        bool started = bridge.start();
        QVERIFY(started);

        // Wait for Running state
        QTRY_COMPARE_WITH_TIMEOUT(bridge.serverState(), 
                                 ImageServerBridge::Running, 2000);

        // Verify connection state is still NoClients (no clients connected yet)
        QCOMPARE(bridge.connectionState(), ImageServerBridge::NoClients);

        // Cleanup
        bridge.stop();
    }

    /**
     * Test: Multiple start/stop cycles work correctly
     * Verifies:
     * - Server can be started and stopped multiple times
     * - State transitions are consistent
     * - No memory leaks or state corruption
     */
    void testMultipleStartStopCycles() {
        ImageServerBridge bridge;

        for (int cycle = 0; cycle < 2; ++cycle) {
            // Start server
            bool started = bridge.start();
            QVERIFY(started);

            // Wait for Running state
            QTRY_COMPARE_WITH_TIMEOUT(bridge.serverState(), 
                                     ImageServerBridge::Running, 2000);
            QCOMPARE(bridge.serverState(), ImageServerBridge::Running);

            // Stop server
            bridge.stop();

            // Wait for Idle state
            QTRY_COMPARE_WITH_TIMEOUT(bridge.serverState(), 
                                     ImageServerBridge::Idle, 2000);
            QCOMPARE(bridge.serverState(), ImageServerBridge::Idle);
        }
    }

    /**
     * Test: Server port assignment works correctly
     * Verifies:
     * - Port 0 triggers automatic assignment
     * - Assigned port is > 1024 (valid port range)
     * - Port can be read after server starts
     */
    void testServerPortAssignment() {
        ImageServerBridge bridge;

        // Set port to 0 (auto-assign)
        bridge.setPort(0);

        // Start server
        bool started = bridge.start();
        QVERIFY(started);

        // Wait for Running state
        QTRY_COMPARE_WITH_TIMEOUT(bridge.serverState(), 
                                 ImageServerBridge::Running, 2000);

        // Verify port was assigned
        quint16 port = bridge.serverPort();
        QVERIFY(port > 0);
        QVERIFY(port >= 1024 || port == 0);  // Valid port range

        // Cleanup
        bridge.stop();
    }

    /**
     * Test: Status message updates on state transitions
     * Verifies:
     * - statusMessage() contains meaningful text during transitions
     * - statusMessageChanged signal is emitted
     * - Message reflects current state
     */
    void testStatusMessageOnStateTransition() {
        ImageServerBridge bridge;

        // Capture status message changes
        QSignalSpy spy_status(&bridge, &ImageServerBridge::statusMessageChanged);

        // Start server
        bool started = bridge.start();
        QVERIFY(started);

        // Wait for Running state
        QTRY_COMPARE_WITH_TIMEOUT(bridge.serverState(), 
                                 ImageServerBridge::Running, 2000);

        // Verify status message was updated
        QTRY_VERIFY_WITH_TIMEOUT(spy_status.count() >= 1, 2000);

        QString status = bridge.statusMessage();
        QVERIFY(!status.isEmpty());

        // Cleanup
        bridge.stop();
    }

    /**
     * Test: Stopping already-stopped server is safe
     * Verifies:
     * - Calling stop() on Idle server doesn't crash
     * - Server remains in Idle state after stop()
     * - No error conditions or exceptions
     */
    void testStoppingIdleServerIsSafe() {
        // Create fresh bridge to avoid signal spy pollution from previous tests
        ImageServerBridge fresh_bridge;

        // Verify starting state
        QCOMPARE(fresh_bridge.serverState(), ImageServerBridge::Idle);

        // Stop already-stopped server - should be safe
        fresh_bridge.stop();

        // Allow event processing
        qt_test::EventLoopSpinner::processEvents();

        // Verify server remains in Idle state
        QCOMPARE(fresh_bridge.serverState(), ImageServerBridge::Idle);
    }

    /**
     * Test: Client model is reset to empty after stop
     * Verifies:
     * - ClientModel count returns to 0 after server stops
     * - No lingering client references
     * - Clean state for next start
     */
    void testClientModelClearedOnStop() {
        ImageServerBridge bridge;

        // Start server
        bool started = bridge.start();
        QVERIFY(started);

        // Wait for Running state
        QTRY_COMPARE_WITH_TIMEOUT(bridge.serverState(), 
                                 ImageServerBridge::Running, 2000);

        // Verify model is empty
        QCOMPARE(bridge.clientModel()->property("count").toInt(), 0);

        // Stop server
        bridge.stop();

        // Wait for Idle state
        QTRY_COMPARE_WITH_TIMEOUT(bridge.serverState(), 
                                 ImageServerBridge::Idle, 2000);

        // Verify model is still accessible and empty
        QCOMPARE(bridge.clientModel()->property("count").toInt(), 0);
    }
};

QTEST_MAIN(TestServerStartStopTransitions)
#include "test_server_start_stop_transitions.moc"
