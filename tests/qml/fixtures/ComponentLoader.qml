import QtQuick 2.15

/**
 * ComponentLoader.qml
 * 
 * Helper component to load QML components under test.
 * Captures load errors and exposes status.
 * 
 * Usage:
 *   ComponentLoader {
 *       id: loader
 *       source: "path/to/Component.qml"
 *       onStatusChanged: {
 *           if (loader.status === "Loaded") { /* test */ }
 *       }
 *   }
 * 
 * NOTE: No assertions. Only exposes state for tests to verify.
 */
Loader {
    id: root

    // Status constants (match Loader.status values)
    readonly property string STATUS_NULL: "Null"      // sourceComponent/source not set
    readonly property string STATUS_READY: "Ready"    // Loaded successfully
    readonly property string STATUS_LOADING: "Loading" // In progress
    readonly property string STATUS_ERROR: "Error"    // Load failed

    // Mapped status string (human-readable)
    readonly property string statusString: {
        switch (status) {
            case Loader.Null: return STATUS_NULL
            case Loader.Ready: return STATUS_READY
            case Loader.Loading: return STATUS_LOADING
            case Loader.Error: return STATUS_ERROR
            default: return "Unknown"
        }
    }

    // Capture load errors (exposed for test assertions)
    readonly property var loadedErrors: []

    // Convenience properties
    readonly property bool isLoaded: status === Loader.Ready
    readonly property bool hasError: status === Loader.Error
    readonly property string errorString: item ? "" : (sourceComponent ? sourceComponent.errorString : "")

    // Monitor for errors
    onStatusChanged: {
        if (status === Loader.Error) {
            // Capture error details
            if (sourceComponent && sourceComponent.errorString) {
                var errors = []
                errors.push({
                    description: sourceComponent.errorString,
                    line: 0,
                    column: 0
                })
                // Note: loadedErrors is read-only, so we just log for now
                console.warn("ComponentLoader error:", sourceComponent.errorString)
            }
        }
    }

    /**
     * Clear/reset the loader (useful between tests)
     */
    function reset() {
        sourceComponent = undefined
        source = ""
    }
}
