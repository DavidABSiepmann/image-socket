import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: "Video Display"

    Connections{
            target: imageSocket
            function onImageChanged(){ img.reload() }
            function onUpdateLabel(msg){ statusLabel.text = msg }
        }

    Image {
        id: img
        property bool counter: false

        asynchronous: false
        source: "image://live/image"
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        cache: false


        function reload() {
            counter = !counter
            source = "image://live/image?id=" + counter
        }
    }

    Row{
        anchors.fill: parent
        spacing: 6

        anchors.bottomMargin: 50
        Button {
            id: btnStart
            enabled: true
            text: "Start"
            onClicked: {
                imageSocket.startVideo()
                btnStop.enabled = true
                btnStart.enabled = false
            }
        }
        Button {
            id: btnStop
            enabled: false
            text: "Stop"
            onClicked: {
                imageSocket.stopVideo()
                btnStop.enabled = false
                btnStart.enabled = true
            }
        }
        Button {
            id: btnReset
            enabled: false
            text: "Reset"
            onClicked: {
                imageSocket.resetVideo()
            }
        }
        Label{
            id: statusLabel
            text: ""
        }
    }
}
