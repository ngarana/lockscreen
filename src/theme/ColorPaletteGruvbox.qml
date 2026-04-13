// ColorPalette.qml - Gruvbox Dark Color Definitions
//
// Provides all color tokens for the Qypr shell system.
// Uses Gruvbox Dark palette with glassmorphism extensions.

import QtQuick

QtObject {
    id: root

    // ========================================================================
    // Base Colors (Gruvbox Dark)
    // ========================================================================

    readonly property color gruvboxBlack: "#282828"
    readonly property color gruvboxRed: "#cc241d"
    readonly property color gruvboxGreen: "#98971a"
    readonly property color gruvboxYellow: "#d79921"
    readonly property color gruvboxBlue: "#458588"
    readonly property color gruvboxPurple: "#b16286"
    readonly property color gruvboxAqua: "#689d6a"
    readonly property color gruvboxOrange: "#d65d0e"
    readonly property color gruvboxGray: "#928372"

    // Bright variants
    readonly property color brightBlack: "#928372"
    readonly property color brightRed: "#fb4934"
    readonly property color brightGreen: "#b8bb26"
    readonly property color brightYellow: "#fabd2f"
    readonly property color brightBlue: "#83a598"
    readonly property color brightPurple: "#d3869b"
    readonly property color brightAqua: "#8ec07c"
    readonly property color brightOrange: "#fe8019"

    // Background shades
    readonly property color bg0: "#282828"
    readonly property color bg1: "#3c3836"
    readonly property color bg2: "#504945"
    readonly property color bg3: "#665c54"
    readonly property color bg4: "#7c6f64"

    // Foreground shades
    readonly property color fg0: "#fbf1c7"
    readonly property color fg1: "#ebdbb2"
    readonly property color fg2: "#d5c4a1"
    readonly property color fg3: "#bdae93"
    readonly property color fg4: "#a89984"

    // ========================================================================
    // Semantic Colors (for UI usage)
    // ========================================================================

    // Background & Surface
    readonly property color background: root.bg0
    readonly property color surface: root.bg1
    readonly property color surfaceHover: root.bg2

    // Glassmorphism surfaces (more opaque, darker - less milky)
    readonly property color glass: "#d5282828"
    readonly property color glassHover: "#e03c3836"
    readonly property color glassBorder: "#40a89984"
    readonly property color glassActive: "#5083a598"

    // Accent colors
    readonly property color primary: root.brightBlue
    readonly property color primaryGlow: "#4083a598"
    readonly property color secondary: root.brightOrange
    readonly property color tertiary: root.brightAqua

    // Text colors
    readonly property color textPrimary: root.fg1
    readonly property color textSecondary: root.fg3
    readonly property color textMuted: root.bg4

    // Semantic colors
    readonly property color error: root.brightRed
    readonly property color success: root.brightGreen
    readonly property color warning: root.brightYellow
    readonly property color info: root.brightBlue

    // ========================================================================
    // Backward Compatibility Aliases (Catppuccin color names)
    // ========================================================================
    // These aliases allow components written for Catppuccin to work with Gruvbox
    // without modification.

    readonly property color crust: root.bg0
    readonly property color mantle: root.bg1
    readonly property color base: root.bg0
    readonly property color surface0: root.bg1
    readonly property color surface1: root.bg2
    readonly property color surface2: root.bg3
    readonly property color overlay0: root.bg4
    readonly property color overlay1: root.fg4
    readonly property color overlay2: root.fg3
    readonly property color subtext0: root.fg3
    readonly property color subtext1: root.fg2
    readonly property color text: root.fg1
    readonly property color lavender: root.brightBlue
    readonly property color blue: root.gruvboxBlue
    readonly property color sapphire: root.brightAqua
    readonly property color sky: root.gruvboxAqua
    readonly property color teal: root.gruvboxGreen
    readonly property color green: root.gruvboxGreen
    readonly property color yellow: root.gruvboxYellow
    readonly property color peach: root.gruvboxOrange
    readonly property color maroon: root.gruvboxPurple
    readonly property color red: root.gruvboxRed
    readonly property color mauve: root.brightPurple
    readonly property color pink: root.brightRed
    readonly property color flamingo: root.brightOrange
    readonly property color rosewater: root.fg0
}
