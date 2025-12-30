import QtQuick 2.15

/**
 * test_button_state.qml
 * 
 * Test harness for button enabled/disabled state tests
 * Demonstrates button responding to backend server state changes
 */

Rectangle {
    id: root
    width: 400
    height: 600
    color: "#f8f8f8"

    // Mock backend - provides serverState
    MockBackend {
        id: mockBackend
        objectName: "mockBackend"
    }

    // Test button with enabled state bound to serverState
    Rectangle {
        id: testButton
        objectName: "testButton"
        
        width: 100
        height: 40
        color: {
            if (!enabled) return "#cccccc"
            return "#0078d4"
        }
        radius: 4
        
        // Enabled when server is Idle or Running
        property bool enabled: mockBackend.serverState === "Idle" || mockBackend.serverState === "Running"

        Text {
            anchors.centerIn: parent
            color: "white"
            text: parent.enabled ? "Ready" : "Busy"
            font.bold: true
        }
    }
}
