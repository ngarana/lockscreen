// Theme.qml - Centralized Theme Configuration
//
// Singleton providing all visual styling constants.
// Uses Catppuccin Mocha palette with glassmorphism extensions
// for modern lockscreen aesthetics over video backgrounds.

pragma Singleton
import Quickshell
import QtQuick

QtObject {
    id: theme

    // ========================================================================
    // Color Palette (Catppuccin Mocha + Glassmorphism)
    // ========================================================================

    property var colors: QtObject {
        // Base colors
        readonly property color background: "#1e1e2e"
        readonly property color surface: "#313244"
        readonly property color surfaceHover: "#45475a"

        // Glassmorphism surfaces (semi-transparent)
        readonly property color glass: "#40313244"
        readonly property color glassHover: "#60454a5a"
        readonly property color glassBorder: "#30cdd6f4"
        readonly property color glassActive: "#50899b4fa"

        // Accent colors
        readonly property color primary: "#89b4fa"
        readonly property color primaryGlow: "#4089b4fa"

        // Text colors
        readonly property color text: "#cdd6f4"
        readonly property color textSubtle: "#a6adc8"
        readonly property color textMuted: "#6c7086"

        // Semantic colors
        readonly property color error: "#f38ba8"
        readonly property color success: "#a6e3a1"
        readonly property color warning: "#f9e2af"
    }

    // ========================================================================
    // Typography
    // ========================================================================

    property var fonts: QtObject {
        readonly property int textSize: 16
        readonly property int textSizeLarge: 24
        readonly property int textSizeClock: 96
        readonly property int textSizeDate: 28
        readonly property string fontFamily: "Inter"
    }

    // ========================================================================
    // Spacing Scale
    // ========================================================================

    property var spacing: QtObject {
        readonly property int small: 8
        readonly property int medium: 16
        readonly property int large: 24
        readonly property int xlarge: 48
        readonly property int xxlarge: 80
    }

    // ========================================================================
    // Border Radius
    // ========================================================================

    property var radius: QtObject {
        readonly property int small: 4
        readonly property int medium: 8
        readonly property int large: 16
        readonly property int xlarge: 24
        readonly property int round: 9999
    }

    // ========================================================================
    // Animation Durations
    // ========================================================================

    property var animation: QtObject {
        readonly property int fast: 150
        readonly property int medium: 300
        readonly property int slow: 500
        readonly property int reveal: 600
    }

    // ========================================================================
    // Shadow & Effects
    // ========================================================================

    property var effects: QtObject {
        readonly property int blurRadius: 40
        readonly property real shadowOpacity: 0.6
        readonly property int shadowOffset: 2
    }
}
