import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import ImageServerBridge 1.0

/// ControlPanel: UI component for server control buttons.
///
/// Manages the Start, Stop, and Reset buttons with styled appearance and feedback.
/// Button states (enabled/disabled) are automatically derived current state.
/// Actions on button clicks are delegated to the C++ imageSocket object.
/// Uses Theme for consistent styling and StyledButton for enhanced visual feedback.
///
Rectangle {
    id: controlPanel
    
    // Theme shared from parent, with fallback to local Theme
    property string themeMode: "light"

    signal toogleTheme()

    color: activeTheme.panelBackgroundColor

    Theme {
        id: activeTheme
        themeMode: controlPanel.themeMode
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: activeTheme.padding
        spacing: activeTheme.spacing
        
        // Start/Stop toggle: shows "Start" when idle, "Stop" when running
        StyledButton {
            id: btnStartStop
            theme: activeTheme
            // Enabled when server is Idle or Running (disallow actions while starting/stopping if desired)
            enabled: imageSocket.serverState === ImageServerBridge.Idle || imageSocket.serverState === ImageServerBridge.Running
            // Icon-only: monochrome play (▶) when idle, filled square (■) when running — no emoji variants
            text: imageSocket.serverState === ImageServerBridge.Idle ? "▶" : "■"
            implicitWidth: activeTheme.buttonHeight // square icon button
            font.pointSize: activeTheme.fontSizeBody + 2
            font.bold: true
            onClicked: {
                if (imageSocket.serverState === ImageServerBridge.Idle) {
                    imageSocket.start()
                } else {
                    imageSocket.stop()
                }
            }

            ToolTip {
                visible: btnStartStop.hovered
                text: imageSocket.serverState === ImageServerBridge.Idle ? "Start server" : "Stop server"
                delay: 300
            }
        }

        /// Reset Button: Reset the server (stop and restart).
        StyledButton {
            id: btnReset
            theme: activeTheme
            enabled: imageSocket.serverState === ImageServerBridge.Running
            // Icon-only reset (mono glyph). Keep tooltip for clarity.
            text: "↻"
            implicitWidth: activeTheme.buttonHeight
            font.pointSize: activeTheme.fontSizeBody + 1
            font.bold: true
            onClicked: {
                imageSocket.stop()
                imageSocket.start()
            }

            ToolTip {
                visible: btnReset.hovered
                text: "Reset server"
                delay: 300
            }
        }

        Item {
            Layout.fillWidth: true
        }

        // Separator between server controls and client controls
        Rectangle {
            width: 1
            height: activeTheme.buttonHeight * 0.7
            color: (activeTheme.groupSeparatorColor !== undefined ? activeTheme.groupSeparatorColor : (activeTheme.themeMode === "light" ? "#d0d0d0" : "#3a3a3a"))
            opacity: 0.6
            Layout.alignment: Qt.AlignVCenter
        }

        // Client selector (simple Popup for wider compatibility)
        StyledButton {
            id: clientsBtn
            theme: activeTheme
            // Added icon (👥) before label
            text: "👥 (" + imageSocket.clientModel.count + ")"
            implicitWidth: activeTheme.buttonHeight * 3
            onClicked: {
                // Close diagnostics if open (panels are mutually exclusive)
                if (diagnostics && diagnostics.panelVisible)
                    diagnostics.panelVisible = false
                clients.panelVisible = !clients.panelVisible
            }


            ToolTip {
                visible: clientsBtn.hovered
                text: "Show connected clients"
                delay: 300
            }
        }

        // Separator between client controls and FPS control
        Rectangle {
            width: 1
            height: activeTheme.buttonHeight * 0.7
            color: (activeTheme.groupSeparatorColor !== undefined ? activeTheme.groupSeparatorColor : (activeTheme.themeMode === "light" ? "#d0d0d0" : "#3a3a3a"))
            opacity: 0.6
            Layout.alignment: Qt.AlignVCenter
        }

        // FPS control (ComboBox with predefined values)
        RowLayout {
            spacing: activeTheme.spacingSmall
            ComboBox {
                id: fpsCombo
                model: [5, 10, 15, 24, 30, 60]
                // Bind to configuredFps from backend; fallback index 2 -> 15
                currentIndex: (function() {
                    var idx = model.indexOf(imageSocket.configuredFps)
                    return idx >= 0 ? idx : 2
                })()
                displayText: (model[currentIndex] + " FPS")
                enabled: imageSocket.connectionState !== ImageServerBridge.NoClients
                onCurrentIndexChanged: {
                    var currentValue = model[currentIndex]
                    if (currentValue !== undefined) {
                        imageSocket.setConfiguredFps(currentValue)
                    }
                }
                //implicitWidth: 160

                ToolTip {
                    visible: fpsCombo.hovered
                    text: "Set target FPS for connected clients"
                    delay: 300
                }
            }
        }
        
        // Separator between FPS control and Diagnostics
        Rectangle {
            width: 1
            height: activeTheme.buttonHeight * 0.7
            color: (activeTheme.groupSeparatorColor !== undefined ? activeTheme.groupSeparatorColor : (activeTheme.themeMode === "light" ? "#d0d0d0" : "#3a3a3a"))
            opacity: 0.6
            Layout.alignment: Qt.AlignVCenter
        }

        // Diagnostics toggle button
        StyledButton {
            id: btnDiag
            theme: activeTheme
            // Added icon (🧰) before label
            text: "🧰"
            implicitWidth: activeTheme.buttonHeight * 1
            onClicked: {
                // Ensure clients panel is closed when diagnostics toggles
                if (clients && clients.panelVisible)
                    clients.panelVisible = false
                if (diagnostics !== undefined && diagnostics !== null) {
                    diagnostics.toggleVisibility()
                } else {
                    console.log("Diagnostics object not found")
                }
            }

            ToolTip {
                visible: btnDiag.hovered
                text: "Show diagnostics panel"
                delay: 300
            }
        }
        
        /// Theme toggle button
        StyledButton {
            id: btnTheme
            theme: activeTheme
            text: activeTheme.themeMode === "light" ? "🌙" : "☀️"
            implicitWidth: activeTheme.buttonHeight
            onClicked: {
                toogleTheme()
            }

            ToolTip {
                visible: btnTheme.hovered
                text: activeTheme.themeMode === "light" ? "Switch to dark theme" : "Switch to light theme"
                delay: 300
            }
        }
    }
    
    Behavior on color {
        ColorAnimation { duration: activeTheme.animationDurationLong }
    }
}
