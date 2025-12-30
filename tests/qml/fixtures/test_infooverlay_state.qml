import QtQuick 2.15

/**
 * test_infooverlay_state.qml
 * 
 * Test harness for InfoOverlay state and visibility tests
 * Sets up MockBackend and simulated InfoOverlay for testing
 */

Rectangle {
    id: root
    width: 400
    height: 600
    color: "#f0f0f0"

    // Mock backend - provides activeClientAlias and signals
    MockBackend {
        id: mockBackend
        objectName: "mockBackend"
    }

    // Simulated InfoOverlay display
    // (Real InfoOverlay requires ImageSocket C++ module)
    Rectangle {
        id: testOverlay
        objectName: "testOverlay"
        
        width: 200
        height: 40
        color: "#000000"
        opacity: 0.7
        radius: 4
        visible: mockBackend.activeClientAlias !== ""

        Text {
            id: overlayText
            anchors.centerIn: parent
            color: "white"
            text: "Client: " + mockBackend.activeClientAlias
            font.bold: true
        }
    }
}
