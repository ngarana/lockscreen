// Animation.qml - Animation Duration and Easing Presets
//
// Provides consistent animation tokens for smooth transitions.

import QtQuick

QtObject {
    id: root

    // ========================================================================
    // Duration Presets
    // ========================================================================

    readonly property int none: 0
    readonly property int fast: 150
    readonly property int medium: 300
    readonly property int slow: 500
    readonly property int reveal: 600
    readonly property int slower: 800

    // ========================================================================
    // Easing Curves
    // ========================================================================

    readonly property int easeInOut: Easing.InOutQuad
    readonly property int easeIn: Easing.InQuad
    readonly property int easeOut: Easing.OutQuad
    readonly property int easeOutBack: Easing.OutBack
    readonly property int easeInBack: Easing.InBack
    readonly property int easeOutElastic: Easing.OutElastic
    readonly property int easeInOutCubic: Easing.InOutCubic

    // ========================================================================
    // Animation Presets (preconfigured combinations)
    // ========================================================================

    readonly property var fade: QtObject {
        readonly property int duration: root.medium
        readonly property int easing: root.easeInOut
    }

    readonly property var slide: QtObject {
        readonly property int duration: root.slow
        readonly property int easing: root.easeOut
    }

    readonly property var scale: QtObject {
        readonly property int duration: root.fast
        readonly property int easing: root.easeOut
    }

    readonly property var revealPreset: QtObject {
        readonly property int duration: root.reveal
        readonly property int easing: root.easeInOut
    }

    readonly property var spring: QtObject {
        readonly property int duration: root.medium
        readonly property int easing: root.easeOutBack
    }
}
