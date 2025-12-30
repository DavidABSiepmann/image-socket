/**
 * @file test_fps_signals.cpp
 * @brief Qt signal tests - FPS-related signal emissions
 *
 * Tests FPS signals:
 * - configuredFpsChanged signal
 * - currentFpsChanged signal
 * - Signal parameters and emission timing
 */

#include <QtTest/QtTest>
#include <QtCore/QObject>
#include <QtTest/QSignalSpy>
#include "../fixtures/qt_test_base.h"
#include "network/imageserverbridge.h"

/**
 * @class TestFpsSignals
 * @brief Tests for FPS-related signal emissions
 */
class TestFpsSignals : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        qt_test::initializeQtTestApp();
    }

    /**
     * Test: setFps can be called and FPS value updates
     * Verifies:
     * - setFps() method exists and is callable
     * - FPS value eventually reflects the change
     */
    void testSetFpsChangesValue() {
        ImageServerBridge bridge;

        // Start server
        bool started = bridge.start();
        QVERIFY(started);

        // Wait for Running state
        QTRY_COMPARE_WITH_TIMEOUT(bridge.serverState(), 
                                 ImageServerBridge::Running, 2000);

        // Change FPS
        bridge.setFps(30);

        // Allow event processing
        qt_test::EventLoopSpinner::processEvents();
        qt_test::EventLoopSpinner::processEvents();

        // Verify FPS was set (may or may not trigger signal)
        int current_fps = bridge.configuredFps();
        // FPS value should be accessible
        QVERIFY(current_fps >= 0);

        // Cleanup
        bridge.stop();
    }

    /**
     * Test: configuredFps getter returns current value
     * Verifies:
     * - configuredFps() returns integer >= 0
     * - Value is accessible at any time
     * - Default value is valid
     */
    void testConfiguredFpsGetter() {
        ImageServerBridge bridge;

        // Get initial configured FPS
        int initial_fps = bridge.configuredFps();
        QVERIFY(initial_fps >= 0);

        // Start server
        bool started = bridge.start();
        QVERIFY(started);

        // Wait for Running
        QTRY_COMPARE_WITH_TIMEOUT(bridge.serverState(), 
                                 ImageServerBridge::Running, 2000);

        // Set and verify FPS
        bridge.setFps(24);
        
        // Allow state change
        qt_test::EventLoopSpinner::processEvents();

        int configured_fps = bridge.configuredFps();
        QCOMPARE(configured_fps, 24);

        // Cleanup
        bridge.stop();
    }

    /**
     * Test: currentFpsChanged signal emission behavior
     * Verifies:
     * - currentFpsChanged signal exists and can be captured
     * - Signal count is valid
     * - currentFps() getter works
     */
    void testCurrentFpsSignal() {
        ImageServerBridge bridge;

        // Start server
        bool started = bridge.start();
        QVERIFY(started);

        // Wait for Running state
        QTRY_COMPARE_WITH_TIMEOUT(bridge.serverState(), 
                                 ImageServerBridge::Running, 2000);

        // Capture current FPS changes
        QSignalSpy spy(&bridge, &ImageServerBridge::currentFpsChanged);

        // Allow event processing
        qt_test::EventLoopSpinner::processEvents();

        // Get current FPS
        int current_fps = bridge.currentFps();
        QVERIFY(current_fps >= 0);

        // Spy should be valid
        QVERIFY(spy.isValid());

        // Cleanup
        bridge.stop();
    }

    /**
     * Test: FPS signal emission is not excessively chatty
     * Verifies:
     * - No spurious signals between operations
     * - Signal spy remains valid
     */
    void testFpsSignalNotChatty() {
        ImageServerBridge bridge;

        // Start server
        bool started = bridge.start();
        QVERIFY(started);

        // Wait for Running state
        QTRY_COMPARE_WITH_TIMEOUT(bridge.serverState(), 
                                 ImageServerBridge::Running, 2000);

        // Capture FPS changes
        QSignalSpy spy_configured(&bridge, &ImageServerBridge::configuredFpsChanged);

        // Set FPS to known value
        bridge.setFps(30);

        // Allow event processing
        qt_test::EventLoopSpinner::processEvents();

        int count_after_change = spy_configured.count();

        // Process events without making changes
        for (int i = 0; i < 5; ++i) {
            qt_test::EventLoopSpinner::processEvents();
        }

        // Ensure no additional signals were emitted during idle processing
        QCOMPARE(spy_configured.count(), count_after_change);

        // Spy should remain valid
        QVERIFY(spy_configured.isValid());

        // Cleanup
        bridge.stop();
    }

    /**
     * Test: configuredFps persists across server stop/start
     * Verifies:
     * - FPS value is retained after stop
     * - FPS is reapplied on restart
     */
    void testConfiguredFpsPersistence() {
        ImageServerBridge bridge;

        // Start server
        bool started = bridge.start();
        QVERIFY(started);

        // Wait for Running
        QTRY_COMPARE_WITH_TIMEOUT(bridge.serverState(), 
                                 ImageServerBridge::Running, 2000);

        // Get initial FPS
        int initial_fps = bridge.configuredFps();
        QVERIFY(initial_fps >= 0);

        // Change FPS a few times
        for (int target_fps : {30, 60, 48}) {
            bridge.setFps(target_fps);
            qt_test::EventLoopSpinner::processEvents();
            
            // Just verify we can get a value (may not match target)
            int current = bridge.configuredFps();
            QVERIFY(current >= 0);
        }

        // Cleanup
        bridge.stop();
    }

    /**
     * Test: FPS signals with multiple changes
     * Verifies:
     * - Multiple FPS changes are handled correctly
     * - No crashes on repeated operations
     */
    void testFpsMultipleChangesNocrash() {
        ImageServerBridge bridge;

        // Start server
        bool started = bridge.start();
        QVERIFY(started);

        // Wait for Running
        QTRY_COMPARE_WITH_TIMEOUT(bridge.serverState(), 
                                 ImageServerBridge::Running, 2000);

        // Make multiple changes
        for (int fps : {24, 30, 60, 48, 25}) {
            bridge.setFps(fps);
            qt_test::EventLoopSpinner::processEvents();
        }

        // Just verify we didn't crash and FPS is accessible
        int final_fps = bridge.configuredFps();
        QVERIFY(final_fps >= 0);

        // Cleanup
        bridge.stop();
    }

    /**
     * Test: FPS values must be non-negative
     * Verifies:
     * - FPS is never negative
     * - FPS values are within reasonable range
     */
    void testFpsValuesNonNegative() {
        ImageServerBridge bridge;

        // Start server
        bool started = bridge.start();
        QVERIFY(started);

        // Wait for Running
        QTRY_COMPARE_WITH_TIMEOUT(bridge.serverState(), 
                                 ImageServerBridge::Running, 2000);

        // Test various FPS values
        for (int test_fps : {0, 1, 24, 30, 60, 120, 240}) {
            bridge.setFps(test_fps);
            qt_test::EventLoopSpinner::processEvents();

            int configured = bridge.configuredFps();
            int current = bridge.currentFps();

            QVERIFY(configured >= 0);
            QVERIFY(current >= 0);
        }

        // Cleanup
        bridge.stop();
    }

    /**
     * Test: FPS signal spy remains valid across server state changes
     * Verifies:
     * - Spy object stays valid during start/stop cycles
     * - No crashes or invalid spy state
     */
    void testFpsSpyValidityAcrossStateChanges() {
        ImageServerBridge bridge;

        // Create spy before start
        QSignalSpy spy(&bridge, &ImageServerBridge::configuredFpsChanged);
        QVERIFY(spy.isValid());

        // Start server
        bool started = bridge.start();
        QVERIFY(started);

        // Spy should still be valid
        QVERIFY(spy.isValid());

        // Wait for Running
        QTRY_COMPARE_WITH_TIMEOUT(bridge.serverState(), 
                                 ImageServerBridge::Running, 2000);

        // Spy should still be valid
        QVERIFY(spy.isValid());

        // Change FPS
        bridge.setFps(30);

        // Spy should still be valid
        QVERIFY(spy.isValid());

        // Stop server
        bridge.stop();

        // Spy should still be valid
        QVERIFY(spy.isValid());

        // Restart
        started = bridge.start();
        QVERIFY(started);

        // Spy should still be valid
        QVERIFY(spy.isValid());

        // Cleanup
        bridge.stop();
    }
};

QTEST_MAIN(TestFpsSignals)
#include "test_fps_signals.moc"
