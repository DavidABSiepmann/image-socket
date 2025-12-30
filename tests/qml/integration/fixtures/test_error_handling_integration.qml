import QtQuick 2.15
import ImageSocket 1.0

/**
 * Error Handling Integration Test Harness
 *
 * Tests that error state triggers:
 * - DiagnosticsPanel displays error details
 * - ToastNotification shows error message
 * - Components clear when error is resolved
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
            statusText: backend.errorMessage.length > 0 ? "ERROR: " + backend.errorMessage : backend.statusMessage
        }

        // Main content area
        Row {
            width: parent.width
            height: parent.height - 40 - 8 - (toastHeight)
            spacing: 8

            // Left panel: ControlPanel or DiagnosticsPanel based on error
            Item {
                width: 250
                height: parent.height

                ControlPanel {
                    anchors.fill: parent
                    serverState: backend.serverState
                    visible: backend.errorMessage === ""
                }

                DiagnosticsPanel {
                    anchors.fill: parent
                    visible: backend.errorMessage !== ""
                    diagnosticText: backend.errorMessage
                }
            }

            // Right side: main content area
            VideoDisplayArea {
                width: parent.width - 250 - 8
                height: parent.height
                enabled: backend.errorMessage === ""
                opacity: enabled ? 1.0 : 0.5
            }
        }

        // Toast notification at bottom
        ToastNotification {
            id: toast
            width: parent.width
            height: toastHeight
            visible: backend.errorMessage !== ""
            message: backend.errorMessage
            type: "error"
        }
    }

    property int toastHeight: backend.errorMessage !== "" ? 60 : 0

    Behavior on toastHeight {
        NumberAnimation { duration: 200 }
    }
}
