import QtQuick 2.15

/**
 * test_responsive_binding.qml
 * 
 * Test harness for component state responsiveness tests
 * Shows component responding to backend property changes via signal binding
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

    // Status label bound to backend property
    Text {
        id: statusLabel
        objectName: "statusLabel"
        
        anchors.centerIn: parent
        color: "#333333"
        text: mockBackend.statusMessage || "Ready"
        font.pixelSize: 14
        font.bold: true
    }
}
