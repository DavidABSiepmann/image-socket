import QtQuick 2.15
import ImageSocket 1.0

/**
 * Theme Integration Test Harness
 *
 * Tests that theme changes propagate to all components:
 * - ControlPanel respects theme
 * - StatusBar colors change
 * - InfoOverlay theme updates
 * - VideoDisplayArea background
 * - All text colors update
 */

Rectangle {
    id: root
    width: 1024
    height: 768
    color: Theme.backgroundColor

    // Theme button at top-right for testing
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 8
        width: 100
        height: 40
        color: Theme.primaryColor
        radius: 4

        Text {
            anchors.centerIn: parent
            text: backend.themeName === "dark" ? "🌙" : "☀️"
            font.pixelSize: 24
            color: Theme.textColor
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                backend.setThemeName(backend.themeName === "dark" ? "light" : "dark")
            }
        }
    }

    Column {
        anchors.fill: parent
        anchors.topMargin: 60
        spacing: 8

        // StatusBar with theme colors
        StatusBar {
            width: parent.width
            height: 40
            serverState: backend.serverState
            statusText: "Theme: " + backend.themeName
        }

        // Main content area
        Row {
            width: parent.width
            height: parent.height - 60 - 40 - 8
            spacing: 8

            // ControlPanel respects theme
            ControlPanel {
                width: 250
                height: parent.height
                serverState: backend.serverState
            }

            // Main area with theme-aware components
            Column {
                width: parent.width - 250 - 8
                height: parent.height
                spacing: 8

                // InfoOverlay with theme
                Rectangle {
                    width: parent.width
                    height: 80
                    color: Theme.panelColor
                    border.color: Theme.borderColor
                    border.width: 1
                    radius: 4

                    Text {
                        anchors.centerIn: parent
                        text: "InfoOverlay (Theme: " + backend.themeName + ")"
                        color: Theme.textColor
                        font.pixelSize: 14
                    }
                }

                // VideoDisplayArea with theme
                VideoDisplayArea {
                    width: parent.width
                    height: parent.height - 80 - 8
                }
            }
        }
    }
}
