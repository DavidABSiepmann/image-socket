import QtQuick 2.15
import ImageSocket 1.0

/**
 * Running State Integration Test Harness
 *
 * Tests that server running state propagates to:
 * - ControlPanel (start/stop button state)
 * - StatusBar (running indicator)
 * - VideoDisplayArea (enabled/disabled state)
 */

Rectangle {
    id: root
    width: 1024
    height: 768
    color: Theme.backgroundColor

    Column {
        anchors.fill: parent
        spacing: 8

        // StatusBar at top
        StatusBar {
            width: parent.width
            height: 40
            serverState: backend.serverState
            statusText: backend.statusMessage
        }

        // Main content area
        Row {
            width: parent.width
            height: parent.height - 40 - 8
            spacing: 8

            // ControlPanel on left
            ControlPanel {
                width: 250
                height: parent.height
                serverState: backend.serverState
                onServerStateChanged: {
                    backend.setServerState(newState)
                }
            }

            // VideoDisplayArea on right
            VideoDisplayArea {
                width: parent.width - 250 - 8
                height: parent.height
                enabled: backend.serverState === "Running"
                opacity: enabled ? 1.0 : 0.5
            }
        }
    }
}
