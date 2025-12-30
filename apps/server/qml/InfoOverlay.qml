import QtQuick 2.15
import QtQuick.Controls 2.15


Rectangle {
    id: overlay
    width: contentRow.implicitWidth + 20
    height: contentRow.implicitHeight + 20
    color: "#000000"
    radius: 8
    opacity: 0.7

    Row {
        id: contentRow
        anchors.fill: parent
        anchors.margins: 10
        spacing: 4

        Label {
            text: "📡 " + (imageSocket.activeClient ? imageSocket.activeClientAlias : "--")
            color: "white"
            font.bold: true
            elide: Text.ElideRight
        }

        Rectangle {
            width: 1
            height: overlay.height - 20
            color: "white"
            opacity: 0.5
        }

        Label {
            text: "⚡ " + (imageSocket.activeClientMeasuredFps > 0 ? (imageSocket.activeClientMeasuredFps + " FPS") : "--") + (imageSocket.currentFps > 0 ? (" (set " + imageSocket.currentFps + " FPS)") : "")
            color: "white"
            font.pixelSize: 13
            font.bold: true
            elide: Text.ElideRight
        }

        Rectangle {
            width: 1
            height: overlay.height - 20
            color: "white"
            opacity: 0.5
        }

        Label {
            text: "🌐 WebSocket"
            color: "white"
            font.pixelSize: 11
        }
    }
}