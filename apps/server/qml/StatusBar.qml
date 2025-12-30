import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15
import ImageServerBridge 1.0

/// StatusBar: Component for displaying server status information and metrics.
///
/// Shows:
/// - Connection status icon (animated indicator)
/// - Current status message with smooth transitions
/// - Frame count and last update timestamp
/// - Visual feedback with color transitions for error conditions
/// Uses Theme for consistent colors and animations with smooth transitions.
///
Rectangle {
    id: statusBar
    
    // Theme shared from parent, with fallback to local Theme
    property string themeMode: "light"

    Theme {
        id: activeTheme
        themeMode: statusBar.themeMode
    }
    
    // Smooth color transitions when theme changes
    color: imageSocket.serverState === ImageServerBridge.Error ? activeTheme.statusBarBackgroundError : activeTheme.statusBarBackgroundNormal

    // Metrics: track frames and last update time
    property int frameCount: 0
    property var lastUpdateTime: null
    
    /// Calculate time elapsed since last frame in readable format.
    function getTimeElapsed() {
        if (lastUpdateTime === null) return "--"
        const now = new Date()
        const elapsed = Math.floor((now - lastUpdateTime) / 1000)
        if (elapsed < 1) return "now"
        if (elapsed < 60) return elapsed + "s ago"
        if (elapsed < 3600) return Math.floor(elapsed / 60) + "m ago"
        return Math.floor(elapsed / 3600) + "h ago"
    }
    
    /// Reset metrics (called when connection is lost).
    function reset() {
        frameCount = 0
        lastUpdateTime = null
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: activeTheme.padding
        spacing: activeTheme.spacing
        
        /// Connection status icon: animated indicator
        Item {
            id: statusIcon
            width: activeTheme.iconSizeSmall
            height: activeTheme.iconSizeSmall
            Layout.alignment: Qt.AlignVCenter
            
            Rectangle {
                id: connectionDot
                anchors.centerIn: parent
                width: activeTheme.iconSizeSmall
                height: activeTheme.iconSizeSmall
                radius: width / 2
                color: (imageSocket.serverState === ImageServerBridge.Running && imageSocket.connectionState === ImageServerBridge.ReceivingFrames) ? activeTheme.successColor : (imageSocket.serverState === ImageServerBridge.Error ? activeTheme.errorColor : activeTheme.infoColor)
                
                // Smooth color transitions
                Behavior on color {
                    ColorAnimation { duration: activeTheme.animationDuration }
                }
                
                // Pulsing animation when server is running but no clients (waiting)
                SequentialAnimation {
                    id: pulseAnimation
                    running: (imageSocket.serverState === ImageServerBridge.Running && imageSocket.connectionState === ImageServerBridge.NoClients)
                    loops: Animation.Infinite
                    
                    NumberAnimation {
                        target: connectionDot
                        property: "opacity"
                        from: 1.0
                        to: 0.5
                        duration: 600
                    }
                    NumberAnimation {
                        target: connectionDot
                        property: "opacity"
                        from: 0.5
                        to: 1.0
                        duration: 600
                    }
                }
                
                DropShadow {
                    anchors.fill: connectionDot
                    horizontalOffset: 0
                    verticalOffset: 0
                    radius: 4
                    color: connectionDot.color
                    opacity: 0.6
                    source: connectionDot
                }
            }
            
            ToolTip {
                visible: iconMouseArea.containsMouse
                text: (imageSocket.serverState !== ImageServerBridge.Running) ? "Server not running" : (imageSocket.connectionState === ImageServerBridge.NoClients ? "Waiting for clients..." : "Connected")
                delay: 500
            }
            
            MouseArea {
                id: iconMouseArea
                anchors.fill: parent
                hoverEnabled: true
            }
        }
        
        /// Status message (left-aligned, flexible width) with smooth transitions
        Label {
            id: statusLabel
            text: imageSocket.statusMessage ? imageSocket.statusMessage : "--"
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            color: imageSocket.serverState === ImageServerBridge.Error ? activeTheme.errorColor : activeTheme.textColor
            font.bold: imageSocket.serverState === ImageServerBridge.Error
            font.pointSize: activeTheme.fontSizeBody
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            
            // Smooth color and font transitions
            Behavior on color {
                ColorAnimation { duration: activeTheme.animationDuration }
            }
            Behavior on font.bold {
                enabled: false  // Skip animation for boolean
            }
        }
        
        /// Separator with smooth transitions
        Rectangle {
            width: activeTheme.borderWidth
            height: activeTheme.iconSizeMedium
            color: activeTheme.borderColor
            Layout.alignment: Qt.AlignVCenter
            
            Behavior on color {
                ColorAnimation { duration: activeTheme.animationDuration }
            }
        }
        
        /// Frame count and last update metrics
        Label {
            text: "📊 " + frameCount + " | ⏱ " + getTimeElapsed()
            verticalAlignment: Text.AlignVCenter
            color: activeTheme.textMutedColor
            font.pointSize: activeTheme.fontSizeSmall
            Layout.alignment: Qt.AlignVCenter
            
            Behavior on color {
                ColorAnimation { duration: activeTheme.animationDuration }
            }
        }
    }
    
    /// Timer to update elapsed time display every second.
    Timer {
        interval: 1000
        running: lastUpdateTime !== null
        repeat: true
        onTriggered: statusBar.update()
    }

    Behavior on color {
        ColorAnimation { duration: activeTheme.animationDurationLong }
    }
}
