import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15

/// StyledButton: Reusable button with drop shadow, animations, and press feedback.
///
/// Features:
/// - Drop shadow for depth
/// - Smooth color transitions (hover, pressed states)
/// - Press feedback (scale animation)
/// - Uses Theme for consistent styling
/// - Accessible with proper text and feedback
///
Button {
    id: styledButton
    
    // Theme shared from parent, with fallback to local Theme
    property var theme
    Theme {
        id: defaultTheme
    }
    readonly property var activeTheme: theme ? theme : defaultTheme
    
    implicitHeight: activeTheme.buttonHeight
    font.pointSize: activeTheme.fontSizeBody
    
    // Container for shadow effect
    background: Rectangle {
        id: buttonBackground
        radius: activeTheme.borderRadius
        color: styledButton.pressed ? activeTheme.buttonPressedColor : 
               (styledButton.hovered ? activeTheme.buttonHoverColor : activeTheme.buttonBackgroundColor)
        border.color: styledButton.enabled ? activeTheme.borderColor : activeTheme.neutralColor
        border.width: activeTheme.borderWidth
        
        // Drop shadow effect
        DropShadow {
            anchors.fill: buttonBackground
            horizontalOffset: activeTheme.shadowHOffset
            verticalOffset: activeTheme.shadowVOffset
            radius: activeTheme.shadowBlur
            color: activeTheme.shadowColor
            opacity: styledButton.enabled ? (styledButton.pressed ? 0.3 : 0.5) : 0.2
            source: buttonBackground
        }

        // Smooth color transitions
        Behavior on color {
            ColorAnimation { duration: activeTheme.animationDuration }
        }
    }
    
    // Press feedback animation (scale)
    scale: styledButton.pressed ? activeTheme.buttonPressScale : 1.0
    
    // Text color behavior
    contentItem: Text {
        text: styledButton.text
        color: styledButton.enabled ? activeTheme.textColor : activeTheme.textMutedColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font: styledButton.font
        wrapMode: Text.WordWrap
        
        Behavior on color {
            ColorAnimation { duration: activeTheme.animationDuration }
        }
    }

    Behavior on scale {
        NumberAnimation { duration: activeTheme.animationDuration }
    }
}
