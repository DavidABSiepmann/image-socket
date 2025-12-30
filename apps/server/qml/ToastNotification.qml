import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0

Item {
    id: root
    property string message: ""
    property int duration: 3000
    property var activeTheme: Theme {}
    width: toastRect.width
    height: toastRect.height
    visible: false

    DropShadow {
        id: shadow
        anchors.fill: toastRect
        source: toastRect
        horizontalOffset: activeTheme.shadowHOffset
        verticalOffset: activeTheme.shadowVOffset
        radius: activeTheme.shadowBlur
        samples: 16
        color: activeTheme.shadowColor
        visible: toastRect.visible
    }

    Rectangle {
        id: toastRect
        objectName: "toastRect"
        width: 320
        height: 56
        radius: 8
        color: activeTheme.panelBackgroundColor
        border.color: activeTheme.borderColor
        border.width: 1
        opacity: 0
        visible: false
        z: 999

        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 20
        anchors.bottomMargin: 24

        Label {
            anchors.centerIn: parent
            text: root.message
            color: activeTheme.textColor
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Rectangle {
            id: closeBtnBg
            width: 22
            height: 22
            radius: 11
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 6
            anchors.rightMargin: 6
            color: activeTheme.neutralColor
            opacity: 0.9
            border.color: activeTheme.borderColor
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: "✕"
                color: activeTheme.textColor
                font.pixelSize: 12
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.hide()
                hoverEnabled: true
            }
        }

        Timer {
            id: hideTimer
            interval: root.duration
            onTriggered: root.hide()
            repeat: false
        }

        NumberAnimation on opacity {
            id: fadeAnim
            duration: 250
            onRunningChanged: {
                if (!running && opacity === 0) {
                    toastRect.visible = false
                    root.visible = false
                    if (fadeEndTimer.running) fadeEndTimer.stop()
                }
            }
        }

        Timer {
            id: fadeEndTimer
            interval: fadeAnim.duration
            onTriggered: {
                toastRect.visible = false
                root.visible = false
            }
            repeat: false
        }
    }

    function show(msg, durationMs) {
        root.message = msg
        root.duration = durationMs || 3000
        hideTimer.restart()
        if (fadeEndTimer.running) fadeEndTimer.stop()
        root.visible = true
        toastRect.visible = true
        toastRect.opacity = 1
    }

    function hide() {
        toastRect.opacity = 0
        fadeEndTimer.restart()
    }
}