// Effects.qml - Visual Effects Definitions
//
// Provides blur, shadow, and other visual effect presets.

import QtQuick

QtObject {
    id: root

    // ========================================================================
    // Blur Effects
    // ========================================================================

    readonly property int blurNone: 0
    readonly property int blurSm: 4
    readonly property int blurMd: 8
    readonly property int blurLg: 16
    readonly property int blurXl: 24
    readonly property int blur2xl: 40
    readonly property int blur3xl: 64

    // ========================================================================
    // Shadow Presets
    // ========================================================================

    readonly property var shadowSm: QtObject {
        readonly property int blur: 4
        readonly property int offset: 1
        readonly property real opacity: 0.3
        readonly property color color: "#000000"
    }

    readonly property var shadowMd: QtObject {
        readonly property int blur: 8
        readonly property int offset: 2
        readonly property real opacity: 0.4
        readonly property color color: "#000000"
    }

    readonly property var shadowLg: QtObject {
        readonly property int blur: 16
        readonly property int offset: 4
        readonly property real opacity: 0.5
        readonly property color color: "#000000"
    }

    readonly property var shadowXl: QtObject {
        readonly property int blur: 32
        readonly property int offset: 8
        readonly property real opacity: 0.6
        readonly property color color: "#000000"
    }

    // ========================================================================
    // Glassmorphism Presets
    // ========================================================================

    readonly property var glassSm: QtObject {
        readonly property int blur: 16
        readonly property color background: "#30313244"
        readonly property color border: "#20cdd6f4"
        readonly property int borderRadius: 8
    }

    readonly property var glassMd: QtObject {
        readonly property int blur: 24
        readonly property color background: "#40313244"
        readonly property color border: "#30cdd6f4"
        readonly property int borderRadius: 12
    }

    readonly property var glassLg: QtObject {
        readonly property int blur: 40
        readonly property color background: "#50313244"
        readonly property color border: "#40cdd6f4"
        readonly property int borderRadius: 16
    }

    // ========================================================================
    // Legacy Aliases
    // ========================================================================

    readonly property int blurRadius: root.blur2xl
    readonly property real shadowOpacity: root.shadowMd.opacity
    readonly property int shadowOffset: root.shadowMd.offset
}
