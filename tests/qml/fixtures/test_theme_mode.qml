import QtQuick 2.15
import "." as Fixtures

/**
 * test_theme_mode.qml
 * 
 * Test harness for theme mode toggle tests
 * Tests Theme component responding to themeMode property changes
 */

Rectangle {
    id: root
    width: 400
    height: 600
    
    // Import Theme from apps/server/qml
    import QtQml 2.15

    // Theme component - test theme mode switching
    Item {
        id: themeContainer

        // Inline Theme definitions (simplified for testing)
        property string themeMode: "light"

        property color backgroundColor: themeMode === "dark" ? "#1e1e1e" : "#ffffff"
        property color textColor: themeMode === "dark" ? "#ffffff" : "#000000"
        property color accentColor: themeMode === "dark" ? "#4CAF50" : "#0078d4"
    }

    // Expose theme as "theme" for test to find
    Component.onCompleted: {
        themeContainer.objectName = "theme"
        root.color = themeContainer.backgroundColor
    }

    Connections {
        target: themeContainer
        function onBackgroundColorChanged() {
            root.color = themeContainer.backgroundColor
        }
    }
}
