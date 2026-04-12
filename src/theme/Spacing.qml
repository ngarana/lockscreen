// Spacing.qml - Spacing Scale and Layout Tokens
//
// Provides consistent spacing values across the shell system.

import QtQuick

QtObject {
    id: root

    // ========================================================================
    // Spacing Scale
    // ========================================================================

    readonly property int none: 0
    readonly property int xs: 2
    readonly property int sm: 4
    readonly property int md: 8
    readonly property int lg: 12
    readonly property int xl: 16
    readonly property int xxl: 24
    readonly property int xxxl: 32
    readonly property int xlarge: 48
    readonly property int xxlarge: 64
    readonly property int xxxlarge: 80

    // ========================================================================
    // Component-Specific Spacing
    // ========================================================================

    // Button spacing
    readonly property int buttonPaddingX: 16
    readonly property int buttonPaddingY: 8
    readonly property int buttonIconGap: 8

    // Input spacing
    readonly property int inputPaddingX: 12
    readonly property int inputPaddingY: 10
    readonly property int inputIconGap: 8

    // Card spacing
    readonly property int cardPadding: 16
    readonly property int cardGap: 12

    // Panel spacing
    readonly property int panelPaddingX: 24
    readonly property int panelPaddingY: 16
    readonly property int panelGap: 16

    // Legacy aliases (for backward compatibility)
    readonly property int small: root.md
    readonly property int medium: root.xl
    readonly property int large: root.xxl
}
