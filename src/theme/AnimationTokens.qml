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

    readonly property var easeInOut: Easing.InOutQuad
    readonly property var easeIn: Easing.InQuad
    readonly property var easeOut: Easing.OutQuad
    readonly property var easeOutBack: Easing.OutBack
    readonly property var easeInBack: Easing.InBack
    readonly property var easeOutElastic: Easing.OutElastic
    readonly property var easeInOutCubic: Easing.InOutCubic

    // ========================================================================
    // Animation Presets (preconfigured combinations)
    // ========================================================================

    readonly property var fade: QtObject {
        readonly property int duration: root.medium
        readonly property var easing: root.easeInOut
    }

    readonly property var slide: QtObject {
        readonly property int duration: root.slow
        readonly property var easing: root.easeOut
    }

    readonly property var scale: QtObject {
        readonly property int duration: root.fast
        readonly property var easing: root.easeOut
    }

    readonly property var reveal: QtObject {
        readonly property int duration: root.reveal
        readonly property var easing: root.easeInOut
    }

    readonly property var spring: QtObject {
        readonly property int duration: root.medium
        readonly property var easing: root.easeOutBack
    }
}
