import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: panel
    width: 420
    height: 320
    color: activeTheme.panelBackgroundColor
    visible: diagnostics ? diagnostics.panelVisible : false // hidden by default

    // Theme shared from parent, with fallback to local Theme
    property string themeMode: "light"

    Theme {
        id: activeTheme
        themeMode: panel.themeMode
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8
        anchors.margins: 8

        RowLayout {
            Layout.fillWidth: true
            z: 2 // above listview
            Label { 
                text: "Diagnostics"; 
                font.bold: true 
                color: activeTheme.textColor
            }
            Item { Layout.fillWidth: true } // spacer

            // Demo logs generator
            StyledButton {
                text: "Generate Demo"
                theme: activeTheme
                implicitWidth: activeTheme.buttonHeight * 2
                onClicked: {
                    diagnostics.generateDemoLogs()
                }
            }

            StyledButton {
                text: "⟵"
                implicitWidth: activeTheme.buttonHeight
                onClicked: diagnostics.clear()
                theme: activeTheme
            }

            StyledButton {
                text: "✖"
                implicitWidth: activeTheme.buttonHeight
                onClicked: diagnostics.panelVisible = false
                theme: activeTheme
            }
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            z: 1
            model: diagnostics.diagnosticsModel
            delegate: Rectangle {
                width: listView.width
                height: 48
                color: index % 2 === 0 ? "transparent" : "#11111122"
                Row {
                    anchors.fill: parent
                    spacing: 8
                    Label { 
                        text: panel.iconForSeverity(severity);
                        color: panel.colorForSeverity(severity)
                    }
                    Column {
                        Text { 
                            text: message; 
                            font.bold: true 
                            color: panel.colorForSeverity(severity)
                        }
                        Text {
                            text: "count: " + count + " • last: " + lastTimestamp; 
                            font.pixelSize: 12; 
                            color: activeTheme.textMutedColor
                        }
                    }
                }
            }
        }
    }

    function colorForSeverity(severity) {
        switch (severity) {
            case 3: return activeTheme.errorColor;
            case 2: return activeTheme.warningColor;
            case 1: return activeTheme.infoColor;
            default: return activeTheme.textColor;
        }
    }

    function iconForSeverity(severity) {
        switch (severity) {
            case 3: return "❗";
            case 2: return "⚠️";
            case 1: return "ℹ️";
            default: return "•";
        }
    }

    Behavior on color {
        ColorAnimation { duration: activeTheme.animationDurationLong }
    }
}
