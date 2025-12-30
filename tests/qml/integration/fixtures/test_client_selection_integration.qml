import QtQuick 2.15
import ImageSocket 1.0

/**
 * Client Selection Integration Test Harness
 *
 * Tests that client selection propagates to:
 * - InfoOverlay (client information display)
 * - VideoDisplayArea (shows selected client's video)
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
            statusText: "Client: " + backend.activeClientId
        }

        // Main content area
        Row {
            width: parent.width
            height: parent.height - 40 - 8
            spacing: 8

            // ClientsPanel on left - can select clients
            ClientsPanel {
                width: 250
                height: parent.height
                onClientSelected: {
                    backend.setActiveClientId(clientId)
                }
            }

            // Content area with InfoOverlay and VideoDisplayArea
            Column {
                width: parent.width - 250 - 8
                height: parent.height
                spacing: 8

                // InfoOverlay shows selected client info
                InfoOverlay {
                    width: parent.width
                    height: 80
                    visible: backend.activeClientId !== ""
                    clientInfo: "Client: " + backend.activeClientId
                }

                // VideoDisplayArea shows selected client video
                VideoDisplayArea {
                    width: parent.width
                    height: parent.height - 80 - 8
                    clientId: backend.activeClientId
                }
            }
        }
    }
}
