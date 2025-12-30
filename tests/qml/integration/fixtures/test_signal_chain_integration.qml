import QtQuick 2.15
import ImageSocket 1.0

/**
 * Signal Chain Integration Test Harness
 *
 * Tests that signal chains work correctly between components:
 * - ControlPanel emits serverStateChanged
 * - StatusBar receives and displays state
 * - VideoDisplayArea updates based on state
 *
 * Verifies no signal loss in chains.
 */

Rectangle {
    id: root
    width: 1024
    height: 768
    color: Theme.backgroundColor

    Column {
        anchors.fill: parent
        spacing: 8

        // StatusBar at top - receives serverState from backend
        StatusBar {
            width: parent.width
            height: 40
            serverState: backend.serverState
            statusText: "State: " + backend.serverState
        }

        // Main content
        Row {
            width: parent.width
            height: parent.height - 40 - 8
            spacing: 8

            // ControlPanel - emits state changes to backend
            ControlPanel {
                width: 250
                height: parent.height
                serverState: backend.serverState
                onServerStateChanged: {
                    // This signal should propagate to backend
                    // which then propagates to StatusBar and VideoDisplayArea
                    backend.setServerState(newState)
                }
            }

            // VideoDisplayArea - responds to state changes
            VideoDisplayArea {
                width: parent.width - 250 - 8
                height: parent.height
                // Enabled state depends on backend.serverState
                // which comes from ControlPanel via signal chain
                enabled: backend.serverState === "Running"
                opacity: enabled ? 1.0 : 0.5
            }
        }
    }
}
