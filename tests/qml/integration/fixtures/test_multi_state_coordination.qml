import QtQuick 2.15
import ImageSocket 1.0

/**
 * Multi-State Coordination Integration Test Harness
 *
 * Tests that multiple state changes coordinate correctly:
 * 1. Server running state
 * 2. Client selection
 * 3. Status message
 * 4. Theme changes
 *
 * Verifies components stay in sync and handle state sequences.
 */

Rectangle {
    id: root
    width: 1024
    height: 768
    color: Theme.backgroundColor

    Column {
        anchors.fill: parent
        spacing: 8
        padding: 8

        // Header with state display
        Rectangle {
            width: parent.width - 16
            height: 60
            color: Theme.panelColor
            border.color: Theme.borderColor
            border.width: 1
            radius: 4

            Column {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4

                Text {
                    color: Theme.textColor
                    font.pixelSize: 12
                    text: "Server: " + backend.serverState + " | Client: " + (backend.activeClientId || "none") + " | Theme: " + backend.themeName
                }

                Text {
                    color: Theme.textColor
                    font.pixelSize: 12
                    text: "Status: " + backend.statusMessage
                }
            }
        }

        // StatusBar
        StatusBar {
            width: parent.width - 16
            height: 40
            serverState: backend.serverState
            statusText: backend.statusMessage
        }

        // Main content
        Row {
            width: parent.width - 16
            height: parent.height - 60 - 40 - 8 - 16
            spacing: 8

            // Left: ControlPanel + ClientsPanel
            Column {
                width: 250
                height: parent.height
                spacing: 8

                ControlPanel {
                    width: parent.width
                    height: parent.height / 2
                    serverState: backend.serverState
                    onServerStateChanged: {
                        backend.setServerState(newState)
                        backend.setStatusMessage("Server state changed to: " + newState)
                    }
                }

                ClientsPanel {
                    width: parent.width
                    height: parent.height / 2
                    onClientSelected: {
                        backend.setActiveClientId(clientId)
                        backend.setStatusMessage("Connected to: " + clientId)
                    }
                }
            }

            // Right: InfoOverlay + VideoDisplayArea
            Column {
                width: parent.width - 250 - 8
                height: parent.height
                spacing: 8

                InfoOverlay {
                    width: parent.width
                    height: 80
                    visible: backend.activeClientId !== ""
                    clientInfo: "Client: " + backend.activeClientId
                }

                VideoDisplayArea {
                    width: parent.width
                    height: parent.height - 80 - 8
                    enabled: backend.serverState === "Running"
                }
            }
        }
    }
}
