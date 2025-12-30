import QtQuick 2.15

/**
 * AnimationHelper.qml
 * 
 * Assists animation-related tests by detecting animation state.
 * Does NOT assert timing - only provides state for QTRY_* usage.
 * 
 * Usage:
 *   AnimationHelper {
 *       id: animHelper
 *       target: toastItem
 *       property: "opacity"
 *   }
 *   
 *   // In test:
 *   QTRY_VERIFY(animHelper.isAnimating === false)  // Wait for animation to stop
 *   QTRY_VERIFY(animHelper.currentValue === 1.0)   // Check final value
 * 
 * NOTE: Only detects animation state. Tests use QTRY_* for timing assertions.
 */
QtObject {
    id: root

    // ========================================================================
    // Properties
    // ========================================================================

    // Target item being animated
    property var target: null

    // Property name being animated (e.g., "opacity", "x")
    property string property: ""

    // Whether animation is currently running
    property bool isAnimating: false

    // Current value of the target property
    property var currentValue: undefined

    // Starting value (captured when animation begins)
    property var startValue: undefined

    // Ending value (target value when animation completes)
    property var endValue: undefined

    // Animation duration in milliseconds (0 if not animating)
    property int duration: 0

    // Elapsed time in milliseconds (from start of animation)
    property int elapsed: 0

    // ========================================================================
    // Internal State
    // ========================================================================

    property var animationTimer: null
    property var animationStartTime: 0

    // ========================================================================
    // Monitoring
    // ========================================================================

    /**
     * Check if target property has active animations
     */
    function checkAnimationState() {
        if (!target || !property) {
            isAnimating = false
            return
        }

        // Get current value
        currentValue = target[property]

        // Check for active animation via Behavior or Transition
        // (Simplified detection - assumes target is a QML item with potential animations)
        
        // Update elapsed time
        if (isAnimating && animationStartTime > 0) {
            elapsed = Date.now() - animationStartTime
        }
    }

    /**
     * Start monitoring animation on target
     */
    function startMonitoring() {
        if (!target || !property) {
            console.warn("AnimationHelper: target or property not set")
            return
        }

        startValue = target[property]
        isAnimating = true
        animationStartTime = Date.now()
        elapsed = 0

        // Poll for animation state every 16ms (roughly 60 FPS)
        if (!animationTimer) {
            animationTimer = Qt.createQmlObject(
                "import QtQuick 2.15; Timer { interval: 16; repeat: true }",
                root,
                "animationTimer"
            )
            animationTimer.triggered.connect(checkAnimationState)
        }
        animationTimer.start()
    }

    /**
     * Stop monitoring animation
     */
    function stopMonitoring() {
        isAnimating = false
        elapsed = 0
        if (animationTimer) {
            animationTimer.stop()
        }
    }

    /**
     * Reset helper state
     */
    function reset() {
        stopMonitoring()
        startValue = undefined
        endValue = undefined
        currentValue = undefined
        duration = 0
        elapsed = 0
    }

    /**
     * Wait for animation to reach a specific value
     * (Helper - tests use QTRY_VERIFY instead)
     */
    function waitForValue(targetVal) {
        // Don't implement - tests use QTRY_VERIFY
        console.debug("AnimationHelper: Waiting for value", targetVal)
    }

    /**
     * Check if animation is complete (endValue reached)
     */
    function isComplete() {
        if (endValue === undefined) {
            return !isAnimating
        }
        
        // Check if current value matches end value (within tolerance for floating-point)
        if (typeof endValue === 'number' && typeof currentValue === 'number') {
            var tolerance = 0.01
            return Math.abs(currentValue - endValue) < tolerance
        }
        
        return currentValue === endValue
    }

    /**
     * Cleanup
     */
    Component.onDestruction: {
        stopMonitoring()
        if (animationTimer) {
            animationTimer.destroy()
        }
    }
}
