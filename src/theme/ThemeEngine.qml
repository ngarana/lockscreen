// ThemeEngine.qml - Centralized Theme Management
//
// Singleton providing all visual styling tokens for the Qypr shell system.
// Aggregates color, typography, spacing, effects, and animation presets.
//
// Usage:
//   import "../theme"
//
//   Rectangle {
//       color: Theme.colors.background
//       Text {
//           font.pixelSize: Theme.typography.sizeLg
//           font.family: Theme.typography.fontFamily
//           color: Theme.colors.textPrimary
//       }
//   }
//
// Future: Will support runtime theme switching via JSON theme files.

pragma Singleton
import QtQuick

QtObject {
    id: root

    // ========================================================================
    // Theme Metadata
    // ========================================================================

    // Current theme variant name
    property string currentTheme: "catppuccin-mocha"

    // Whether the current theme is dark mode
    readonly property bool isDark: true

    // Theme version (for caching purposes)
    property string themeVersion: "1.0.0"

    // ========================================================================
    // Token Groups
    // ========================================================================

    // Color palette
    readonly property var colors: ColorPalette {}

    // Typography definitions
    readonly property var typography: Typography {}

    // Spacing scale
    readonly property var spacing: Spacing {}

    // Visual effects (blur, shadow, glassmorphism)
    readonly property var effects: Effects {}

    // Icon glyphs (Unicode symbols)
    readonly property var icons: IconTokens {}

    // Animation durations and easing
    readonly property var animation: AnimationTokens {}

    // ========================================================================
    // Legacy Aliases (for backward compatibility during migration)
    // ========================================================================

    // Font aliases (map to typography)
    readonly property var fonts: QtObject {
        readonly property int textSize: root.typography.sizeLg
        readonly property int textSizeLarge: root.typography.sizeXl
        readonly property int textSizeClock: root.typography.sizeClock
        readonly property int textSizeDate: root.typography.size2xl
        readonly property string fontFamily: root.typography.fontFamily
        readonly property string iconFontFamily: root.typography.iconFontFamily
    }

    // Radius presets
    readonly property var radius: QtObject {
        readonly property int small: 4
        readonly property int medium: 8
        readonly property int large: 16
        readonly property int xlarge: 24
        readonly property int round: 9999
    }

    // Audio controller sizing (legacy)
    readonly property var audio: QtObject {
        readonly property int buttonSize: 48
        readonly property int buttonIconSize: 20
        readonly property int minWidth: 320
        readonly property int maxWidth: 420
        readonly property int progressHeight: 4
        readonly property int volumeSliderWidth: 150
        readonly property int spacing: 12
        readonly property int panelPadding: 16
    }

    // ========================================================================
    // Future: Dynamic Theme Loading
    // ========================================================================

    // loadTheme(name: string) - Load theme from JSON file
    // switchTheme(name: string) - Switch to different theme at runtime
    // getThemeList() - Return available themes
    // watchThemeChanges() - Auto-switch on time of day
}
