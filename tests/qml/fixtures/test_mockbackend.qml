import QtQuick 2.15

/**
 * test_mockbackend.qml
 * 
 * Test harness for MockBackend signal emission tests
 * Simple harness that just provides MockBackend
 */

Rectangle {
    id: root
    width: 400
    height: 600
    color: "#ffffff"

    // Mock backend - for testing signal emission
    MockBackend {
        id: mockBackend
        objectName: "mockBackend"
    }
}
