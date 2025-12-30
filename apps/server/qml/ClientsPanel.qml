import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: clientsPanel
    width: 420
    height: 320
    color: activeTheme.panelBackgroundColor
    visible: clients ? clients.panelVisible : false

    // Theme shared from parent, with fallback to local Theme
    property string themeMode: "light"

    Theme {
        id: activeTheme
        themeMode: clientsPanel.themeMode
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8
        anchors.margins: 8

        RowLayout {
            Layout.fillWidth: true
            z: 2
            Label { 
                text: "Clients"; 
                font.bold: true 
                color: activeTheme.textColor
            }
            Item { Layout.fillWidth: true }

            StyledButton {
                text: "✖"
                implicitWidth: activeTheme.buttonHeight
                onClicked: clients.panelVisible = false
                theme: activeTheme
            }
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            z: 1
            model: imageSocket.clientModel
            delegate: Rectangle {
                width: listView.width
                height: 52
                color: clientId === imageSocket.activeClient ? activeTheme.panelAccentColor : (index % 2 === 0 ? "transparent" : "#11111122")
                border.color: clientId === imageSocket.activeClient ? activeTheme.panelAccentColor : "transparent"
                border.width: clientId === imageSocket.activeClient ? 1 : 0

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8

                    // Left: client name + status
                    ColumnLayout {
                        Layout.fillWidth: true
                        Text {
                            text: alias;
                            font.bold: true;
                            color: clientId === imageSocket.activeClient ? activeTheme.textOnAccentColor : activeTheme.textColor
                            elide: Text.ElideRight
                        }
                        Text {
                            text: status + " • " + clientId;
                            font.pixelSize: 12;
                            color: clientId === imageSocket.activeClient ? activeTheme.textOnAccentColorSecondary : activeTheme.textMutedColor
                            elide: Text.ElideRight
                        }
                    }

                    // Right: FPS info (measured and configured)
                    ColumnLayout {
                        Layout.alignment: Qt.AlignRight
                        width: 140
                        spacing: 2
                        Text {
                            text: (measuredFps > 0 ? (measuredFps + " FPS") : "--")
                            font.pixelSize: 13
                            color: clientId === imageSocket.activeClient ? activeTheme.textOnAccentColor : activeTheme.textColor
                            horizontalAlignment: Text.AlignRight
                        }
                        Text {
                            text: (configuredFps > 0 ? ("set " + configuredFps + " FPS") : "")
                            font.pixelSize: 11
                            color: clientId === imageSocket.activeClient ? activeTheme.textOnAccentColorSecondary : activeTheme.textMutedColor
                            horizontalAlignment: Text.AlignRight
                            elide: Text.ElideRight
                        }
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        imageSocket.setActiveClient(clientId)
                        clients.panelVisible = false
                    }
                }
            }
        }
    }

    function colorCell(index)
    {
        if(index === imageSocket.activeClientIndex) {
            return activeTheme.panelAccentColor
        }
        return index % 2 === 0 ? "transparent" : "#11111122"
    }

    Behavior on color {
        ColorAnimation { duration: activeTheme.animationDurationLong }
    }
}
