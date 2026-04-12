// ColorPalette.qml - Catppuccin Mocha Color Definitions
//
// Provides all color tokens for the Qypr shell system.
// Uses Catppuccin Mocha palette with glassmorphism extensions.

import QtQuick

QtObject {
    id: root

    // ========================================================================
    // Base Colors
    // ========================================================================

    readonly property color crust: "#11111b"
    readonly property color mantle: "#181825"
    readonly property color base: "#1e1e2e"
    readonly property color surface0: "#313244"
    readonly property color surface1: "#45475a"
    readonly property color surface2: "#585b70"
    readonly property color overlay0: "#6c7086"
    readonly property color overlay1: "#7f849c"
    readonly property color overlay2: "#9399b2"
    readonly property color subtext0: "#a6adc8"
    readonly property color subtext1: "#bac2de"
    readonly property color text: "#cdd6f4"
    readonly property color lavender: "#b4befe"
    readonly property color blue: "#89b4fa"
    readonly property color sapphire: "#74c7ec"
    readonly property color sky: "#89dceb"
    readonly property color teal: "#94e2d5"
    readonly property color green: "#a6e3a1"
    readonly property color yellow: "#f9e2af"
    readonly property color peach: "#fab387"
    readonly property color maroon: "#eba0ac"
    readonly property color red: "#f38ba8"
    readonly property color mauve: "#cba6f7"
    readonly property color pink: "#f5c2e7"
    readonly property color flamingo: "#f2cdcd"
    readonly property color rosewater: "#f5e0dc"

    // ========================================================================
    // Semantic Colors (for UI usage)
    // ========================================================================

    // Background & Surface
    readonly property color background: root.base
    readonly property color surface: root.surface0
    readonly property color surfaceHover: root.surface1

    // Glassmorphism surfaces (semi-transparent)
    readonly property color glass: "#40313244"
    readonly property color glassHover: "#60454a5a"
    readonly property color glassBorder: "#30cdd6f4"
    readonly property color glassActive: "#50899b4fa"

    // Accent colors
    readonly property color primary: root.blue
    readonly property color primaryGlow: "#4089b4fa"
    readonly property color secondary: root.mauve
    readonly property color tertiary: root.teal

    // Text colors
    readonly property color textPrimary: root.text
    readonly property color textSecondary: root.subtext0
    readonly property color textMuted: root.overlay0

    // Semantic colors
    readonly property color error: root.red
    readonly property color success: root.green
    readonly property color warning: root.yellow
    readonly property color info: root.blue
}
