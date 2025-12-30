import QtQuick 2.15

/**
 * TestRoot.qml
 * 
 * Provides a controlled root item for QML tests.
 * Ensures consistent sizing and parent hierarchy.
 * 
 * Usage:
 *   TestRoot {
 *       width: 800; height: 600
 *       YourComponent { anchors.fill: parent }
 *   }
 * 
 * NOTE: No visual styling. Tests control child components directly.
 */
Rectangle {
    id: root

    // Default test window size (overridable)
    width: 800
    height: 600

    // Transparent background (no visual assumptions)
    color: "#00000000"
    border.width: 0

    // Expose a container for test component
    default property alias content: contentContainer.children

    Rectangle {
        id: contentContainer
        anchors.fill: parent
        color: "transparent"
    }
}
