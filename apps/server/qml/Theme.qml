import QtQuick 2.15

/// Theme: Centralized styling definitions for the UI.
///
/// Provides consistent colors, font sizes, spacing, animations, and drop shadow effects
/// across all components. Supports light and dark themes that can be toggled at runtime.
/// Modify here to change the app-wide appearance.
///
Item {
    id: theme

    // ===== Theme Mode =====
    /// Current theme mode: "light" or "dark" - can be toggled at runtime
    property string themeMode: "light"

    // ===== Light Theme Colors =====
    readonly property color lightBackgroundColor: "#ffffff"
    readonly property color lightAccentColor: "#0078d4"
    readonly property color lightErrorColor: "#d13438"
    readonly property color lightWarningColor: "#ffb900"
    readonly property color lightInfoColor: "#0078d4"
    readonly property color lightSuccessColor: "#107c10"
    readonly property color lightNeutralColor: "#e1e1e1"
    readonly property color lightTextColor: "#000000"
    readonly property color lightTextMutedColor: "#666666"
    readonly property color lightButtonBackgroundColor: "#f3f3f3"
    readonly property color lightButtonHoverColor: "#ebebeb"
    readonly property color lightButtonPressedColor: "#d0d0d0"
    readonly property color lightStatusBarBackgroundNormal: "#f0f0f0"
    readonly property color lightStatusBarBackgroundError: "#ffcccc"
    readonly property color lightVideoBackgroundColor: '#c0c0c0'
    readonly property color lightPlaceholderBackgroundColor: '#c4c4c4'
    readonly property color lightPanelBackgroundColor: "#f8f8f8"
    readonly property color lightBorderColor: "#d0d0d0"
    readonly property color lightPanelAccentColor: '#fffc34'
    readonly property color lightTextOnAccentColor: "#000000"
    readonly property color lightTextOnAccentColorSecondary: '#272727'

    // ===== Dark Theme Colors =====
    readonly property color darkBackgroundColor: "#1e1e1e"
    readonly property color darkAccentColor: "#4a9eff"
    readonly property color darkErrorColor: "#ff6b6b"
    readonly property color darkWarningColor: "#ffc107"
    readonly property color darkInfoColor: "#4a9eff"
    readonly property color darkSuccessColor: "#51cf66"
    readonly property color darkNeutralColor: "#3a3a3a"
    readonly property color darkTextColor: "#f0f0f0"
    readonly property color darkTextMutedColor: "#a0a0a0"
    readonly property color darkButtonBackgroundColor: "#2d2d2d"
    readonly property color darkButtonHoverColor: "#3a3a3a"
    readonly property color darkButtonPressedColor: "#1a1a1a"
    readonly property color darkStatusBarBackgroundNormal: "#252525"
    readonly property color darkStatusBarBackgroundError: "#4d2020"
    readonly property color darkVideoBackgroundColor: "#000000"
    readonly property color darkPlaceholderBackgroundColor: "#1a1a1a"
    readonly property color darkPanelBackgroundColor: "#2a2a2a"
    readonly property color darkBorderColor: "#404040"
    readonly property color darkPanelAccentColor: '#98a04f'
    readonly property color darkTextOnAccentColor: "#000000"
    readonly property color darkTextOnAccentColorSecondary: '#272727'

    // ===== Active Colors (bound to theme mode) =====
    property color backgroundColor: themeMode === "dark" ? darkBackgroundColor : lightBackgroundColor
    property color accentColor: themeMode === "dark" ? darkAccentColor : lightAccentColor
    property color errorColor: themeMode === "dark" ? darkErrorColor : lightErrorColor
    property color warningColor: themeMode === "dark" ? darkWarningColor : lightWarningColor
    property color infoColor: themeMode === "dark" ? darkInfoColor : lightInfoColor
    property color successColor: themeMode === "dark" ? darkSuccessColor : lightSuccessColor
    property color neutralColor: themeMode === "dark" ? darkNeutralColor : lightNeutralColor
    property color textColor: themeMode === "dark" ? darkTextColor : lightTextColor
    property color textMutedColor: themeMode === "dark" ? darkTextMutedColor : lightTextMutedColor
    property color buttonBackgroundColor: themeMode === "dark" ? darkButtonBackgroundColor : lightButtonBackgroundColor
    property color buttonHoverColor: themeMode === "dark" ? darkButtonHoverColor : lightButtonHoverColor
    property color buttonPressedColor: themeMode === "dark" ? darkButtonPressedColor : lightButtonPressedColor
    property color statusBarBackgroundNormal: themeMode === "dark" ? darkStatusBarBackgroundNormal : lightStatusBarBackgroundNormal
    property color statusBarBackgroundError: themeMode === "dark" ? darkStatusBarBackgroundError : lightStatusBarBackgroundError
    property color videoBackgroundColor: themeMode === "dark" ? darkVideoBackgroundColor : lightVideoBackgroundColor
    property color placeholderBackgroundColor: themeMode === "dark" ? darkPlaceholderBackgroundColor : lightPlaceholderBackgroundColor
    property color panelBackgroundColor: themeMode === "dark" ? darkPanelBackgroundColor : lightPanelBackgroundColor
    property color borderColor: themeMode === "dark" ? darkBorderColor : lightBorderColor
    property color panelAccentColor: themeMode === "dark" ? darkPanelAccentColor : lightPanelAccentColor
    property color textOnAccentColor: themeMode === "dark" ? darkTextOnAccentColor : lightTextOnAccentColor
    property color textOnAccentColorSecondary: themeMode === "dark" ? darkTextOnAccentColorSecondary : lightTextOnAccentColorSecondary

    // Separator color used between logical control groups
    property color groupSeparatorColor: themeMode === "dark" ? darkBorderColor : lightBorderColor

    // ===== Typography =====
    readonly property int fontSizeTitle: 16
    readonly property int fontSizeBody: 12
    readonly property int fontSizeSmall: 10
    readonly property string fontFamily: "Segoe UI, system-ui, sans-serif"

    // ===== Spacing & Margins =====
    readonly property int padding: 12
    readonly property int spacing: 12
    readonly property int spacingSmall: 6
    readonly property int spacingLarge: 20
    readonly property int margin: 12

    // ===== Sizing =====
    readonly property int buttonHeight: 36
    readonly property int statusBarHeight: 40
    readonly property int controlPanelHeight: 50
    readonly property int iconSizeSmall: 16
    readonly property int iconSizeMedium: 24
    readonly property int windowMinWidth: 640
    readonly property int windowMinHeight: 480

    // ===== Borders & Corners =====
    readonly property int borderRadius: 4
    readonly property int borderWidth: 1

    // ===== Drop Shadow Properties =====
    /// Shadow radius for buttons and panels
    readonly property int shadowRadius: 8
    /// Shadow color (platform-dependent, adjusted for theme)
    property color shadowColor: themeMode === "dark" ? "#00000066" : "#00000033"
    /// Horizontal offset for shadow
    readonly property int shadowHOffset: 0
    /// Vertical offset for shadow
    readonly property int shadowVOffset: 2
    /// Shadow blur radius
    readonly property int shadowBlur: 6

    // ===== Animations =====
    /// Standard animation duration (milliseconds)
    readonly property int animationDuration: 200
    /// Long animation duration (for state transitions)
    readonly property int animationDurationLong: 400

    // ===== Button Press Feedback =====
    /// Scale factor when button is pressed (e.g., 0.97 for slight shrink)
    readonly property real buttonPressScale: 0.97
}
