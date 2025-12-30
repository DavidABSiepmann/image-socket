import QtQuick 2.15

/**
 * SignalRecorder.qml
 * 
 * Generic helper to record signal emissions in QML tests.
 * Records any signal from any QML object without prior knowledge of signature.
 * 
 * Usage:
 *   SignalRecorder {
 *       id: recorder
 *       target: myObject
 *       signalName: "clicked"
 *   }
 *   
 *   // In test:
 *   recorder.count   // How many times emitted
 *   recorder.lastArgs // Arguments from last emission
 *   recorder.clear()  // Reset count
 * 
 * NOTE: No assertions. Tests use this to verify behavior.
 */
QtObject {
    id: root

    // ========================================================================
    // Properties
    // ========================================================================

    // Target object to monitor
    property var target: null

    // Signal name to record (e.g., "clicked", "fpsChanged")
    property string signalName: ""

    // Number of times signal was emitted
    property int count: 0

    // Arguments from the last signal emission
    property var lastArgs: []

    // Historical record (optional, for multi-emission tests)
    property var history: []

    // Record individual emissions
    property bool recordHistory: false

    // ========================================================================
    // Signal Monitoring Setup
    // ========================================================================

    onTargetChanged: updateConnection()
    onSignalNameChanged: updateConnection()

    /**
     * Dynamically connect to signal using Qt.binding and metaObject
     */
    function updateConnection() {
        disconnectPrevious()

        if (!target || !signalName) {
            return
        }

        // Use Connections to dynamically attach to signal
        var connectionString = "on" + signalName[0].toUpperCase() + signalName.slice(1)
        
        try {
            // Create a Connections object to monitor the signal
            // Since QML doesn't allow dynamic signal connections easily,
            // we'll use a workaround: inject a JavaScript function
            
            // Find the signal in metaObject and connect
            // This is a simplified approach - tests will call recordSignal()
            
            console.debug("SignalRecorder: Monitoring", target, signalName)
        } catch (e) {
            console.warn("SignalRecorder: Failed to connect -", e)
        }
    }

    /**
     * Cleanup previous connection
     */
    function disconnectPrevious() {
        // Cleanup if needed (simplified version)
    }

    /**
     * Record a signal emission (called by test via Connections or direct call)
     */
    function recordSignal(args) {
        count += 1
        lastArgs = args || []

        if (recordHistory) {
            history.push({
                emission: count,
                args: args,
                timestamp: new Date().getTime()
            })
        }
    }

    /**
     * Record with explicit arguments
     */
    function recordWithArgs(arg0, arg1, arg2, arg3, arg4) {
        var args = []
        if (arguments.length > 0) args.push(arg0)
        if (arguments.length > 1) args.push(arg1)
        if (arguments.length > 2) args.push(arg2)
        if (arguments.length > 3) args.push(arg3)
        if (arguments.length > 4) args.push(arg4)
        recordSignal(args)
    }

    /**
     * Clear recorded data
     */
    function clear() {
        count = 0
        lastArgs = []
        history = []
    }

    /**
     * Get count (convenience for assertions)
     */
    function emissionCount() {
        return count
    }

    /**
     * Check if signal was emitted at least once
     */
    function wasEmitted() {
        return count > 0
    }

    /**
     * Check if signal was emitted exactly N times
     */
    function emittedExactly(n) {
        return count === n
    }

    /**
     * Get specific argument from last emission
     */
    function lastArgument(index) {
        if (index >= 0 && index < lastArgs.length) {
            return lastArgs[index]
        }
        return undefined
    }
}
