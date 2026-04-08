// Theme.qml - Centralized Theme Configuration
//
// This singleton provides a single source of truth for all visual styling.
// Uses the Catppuccin Mocha color palette for a modern, dark aesthetic.
//
// Color Palette: Catppuccin Mocha
// - Background: Dark blue-gray base
// - Surface: Elevated surface colors
// - Text: High contrast text colors
// - Accent: Blue primary accent
//
// Usage:
//   color: Theme.colors.background
//   font.pixelSize: Theme.fonts.textSize
//   radius: Theme.radius.medium
//
// Benefits:
// - Single location for all theme values
// - Easy to modify entire look from one file
// - Consistent spacing, fonts, and colors across components
// - Type-safe property access

pragma Singleton
import Quickshell
import QtQuick

QtObject {
    id: theme

    // ========================================================================
    // Color Palette (Catppuccin Mocha)
    // ========================================================================
    // All colors are defined as readonly for immutability
    // Access via: Theme.colors.<name>

    property var colors: QtObject {
        // Base colors - Background layers
        readonly property color background: "#1e1e2e"    // Main background (base)
        readonly property color surface: "#313244"      // Elevated surfaces (surface0)
        readonly property color surfaceHover: "#45475a" // Hover state (surface1)

        // Accent colors
        readonly property color primary: "#89b4fa"      // Primary accent (blue)

        // Text colors - Different emphasis levels
        readonly property color text: "#cdd6f4"         // Primary text (text)
        readonly property color textSubtle: "#a6adc8"   // Secondary text (subtext0)
        readonly property color textMuted: "#6c7086"    // Tertiary text (overlay0)

        // Semantic colors - Status indicators
        readonly property color error: "#f38ba8"        // Error/danger (red)
        readonly property color success: "#a6e3a1"      // Success (green)
        readonly property color warning: "#f9e2af"      // Warning (yellow)
    }

    // ========================================================================
    // Typography
    // ========================================================================
    // Font sizes in pixels
    // Access via: Theme.fonts.<name>

    property var fonts: QtObject {
        readonly property int textSize: 16       // Standard text
        readonly property int textSizeLarge: 24  // Large text (headers, buttons)
        readonly property int textSizeClock: 72  // Clock display
        readonly property int textSizeDate: 24   // Date display
    }

    // ========================================================================
    // Spacing Scale
    // ========================================================================
    // Consistent spacing values for margins and padding
    // Access via: Theme.spacing.<name>

    property var spacing: QtObject {
        readonly property int small: 8    // Tight spacing
        readonly property int medium: 16  // Standard spacing
        readonly property int large: 24   // Generous spacing
        readonly property int xlarge: 40  // Extra spacing (section breaks)
    }

    // ========================================================================
    // Border Radius
    // ========================================================================
    // Rounded corner values
    // Access via: Theme.radius.<name>

    property var radius: QtObject {
        readonly property int small: 4     // Subtle rounding
        readonly property int medium: 8    // Standard rounding
        readonly property int large: 12    // Prominent rounding
        readonly property int round: 9999  // Fully rounded (pills)
    }

    // ========================================================================
    // Animation Durations
    // ========================================================================
    // Transition timing in milliseconds
    // Access via: Theme.animation.<name>

    property var animation: QtObject {
        readonly property int fast: 150    // Quick interactions
        readonly property int medium: 250  // Standard transitions
        readonly property int slow: 400    // Deliberate animations
    }
}
