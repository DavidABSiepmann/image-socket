import QtQuick 2.15

/**
 * test_statusbar_state.qml
 * 
 * Test harness for StatusBar text binding tests
 * Demonstrates text binding to backend statusMessage property
 */

Rectangle {
    id: root
    width: 400
    height: 600
    color: "#ffffff"

    // Mock backend - provides statusMessage
    MockBackend {
        id: mockBackend
        objectName: "mockBackend"
    }

    // Status bar with text binding
    Rectangle {
        id: statusBar
        width: parent.width
        height: 40
        color: "#f0f0f0"
        border.color: "#d0d0d0"
        border.width: 1

        Text {
            id: statusText
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.margins: 10
            color: "#333333"
            text: mockBackend.statusMessage || "(no status)"
            font.pixelSize: 12
        }
    }
}
