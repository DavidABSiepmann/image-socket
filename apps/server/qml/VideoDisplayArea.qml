import QtQuick 2.15
import QtQuick.Controls 2.15
import ImageServerBridge 1.0

/// VideoDisplayArea: Component for displaying the live video stream.
///
/// Displays the latest image from the server's ImageSocketServer.
/// Automatically reloads and refreshes when new images arrive via the imageChanged signal.
/// Shows a placeholder when no image is available.
///
Rectangle {
    id: videoArea
    
    // Theme mode
    property string themeMode: "light"

    Theme {
        id: activeTheme
        themeMode: videoArea.themeMode
    }
    
    color: activeTheme.videoBackgroundColor
    
    // External references expected to be passed from parent.
    property var statusBar
    
    // Track whether we've received at least one image
    property bool hasImage: false
    
    /// Placeholder: displayed when no image is available.
    Rectangle {
        id: placeholderArea
        anchors.fill: parent
        color: activeTheme.placeholderBackgroundColor
        // Visible when backend is NOT actively receiving frames
        visible: imageSocket.connectionState !== ImageServerBridge.ReceivingFrames

        Column {
            anchors.centerIn: parent
            spacing: activeTheme.spacingLarge

            /// Placeholder image (simple SVG or text-based icon).
            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                width: 120
                height: 120
                color: activeTheme.neutralColor
                radius: activeTheme.borderRadius

                Text {
                    anchors.centerIn: parent
                    text: "📷"
                    font.pixelSize: 64
                    color: activeTheme.textMutedColor
                }

                Behavior on color {
                    ColorAnimation { duration: activeTheme.animationDurationLong }
                }
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "No image available"
                color: activeTheme.textMutedColor
                font.pointSize: activeTheme.fontSizeBody

                Behavior on color {
                    ColorAnimation { duration: activeTheme.animationDuration }
                }
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Waiting for incoming frames..."
                color: activeTheme.textMutedColor
                font.pointSize: activeTheme.fontSizeSmall

                Behavior on color {
                    ColorAnimation { duration: activeTheme.animationDuration }
                }
            }
        }

        Behavior on color {
            ColorAnimation { duration: activeTheme.animationDurationLong }
        }
    }

    // Info overlay showing active client and FPS (minimal for now)
    InfoOverlay {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 8
        visible: imageSocket.connectionState === ImageServerBridge.ReceivingFrames
        z: 2
    }
    
    /// Live image display.
    Image {
        id: img
        asynchronous: false
        source: "image://live/image"
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        cache: false
        visible: videoArea.hasImage
        z: 1
    }
    
    /// Update metrics when a new frame is available from the backend bridge.
    Connections {
        target: imageSocket
        function onFrameIdChanged(newId) {
            // Force image provider to reload by changing the id query parameter
            img.source = "image://live/image?id=" + newId
            videoArea.hasImage = true

            if (statusBar !== undefined && statusBar !== null) {
                statusBar.frameCount++
                statusBar.lastUpdateTime = new Date()
            }
        }
    }

    // Also listen to newFrameReady for compatibility (frame object is ignored in QML)
    Connections {
        target: imageSocket
        function onNewFrameReady(frame) {
            img.source = "image://live/image?id=" + frame
            videoArea.hasImage = true

            if (statusBar !== undefined && statusBar !== null) {
                statusBar.frameCount++
                statusBar.lastUpdateTime = new Date()
            }
        }
    }
    
    /// Listen for connection lost to reset UI state.
    Connections {
        target: imageSocket
        function onConnectionLost() {
            videoArea.hasImage = false
            if (statusBar !== undefined && statusBar !== null) {
                statusBar.reset()
            }
            // reset image source to placeholder
            img.source = "image://live/image?id=0"
        }
    }

    Behavior on color {
        ColorAnimation { duration: activeTheme.animationDurationLong }
    }
}
