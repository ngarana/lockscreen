// FadeAnimation.qml - Fade In/Out Animation
//
// Provides configurable fade animations for UI elements.
// Supports fade in, fade out, and cross-fade transitions.
//
// Usage:
// FadeAnimation {
//     target: myItem
//     duration: 300
//     fadeIn: true
// }
//
// Properties:
// - target: Item - The item to animate
// - duration: int - Animation duration in ms (default: 300)
// - fadeIn: bool - Animate to visible (default: true)
// - fadeOut: bool - Animate to hidden (default: false)
// - fromOpacity: real - Starting opacity (default: 0 for fade in, 1 for fade out)
// - toOpacity: real - Ending opacity (default: 1 for fade in, 0 for fade out)
// - easing: int - Easing curve type (default: Easing.InOutQuad)
//
// Signals:
// - started(): Emitted when animation starts
// - finished(): Emitted when animation completes

import QtQuick
import "../theme" as Theme

Item {
    id: root

    property Item target: null
    property int duration: Theme.ThemeEngine.animation.medium
    property bool fadeIn: true
    property bool fadeOut: false
    property real fromOpacity: fadeIn ? 0 : 1
    property real toOpacity: fadeIn ? 1 : 0
    property int easing: Easing.InOutQuad

    signal started()
    signal finished()

    property bool _isRunning: false

    function start() {
        if (!target) return
        _isRunning = true
        root.started()
        opacityAnimation.start()
    }

    function stop() {
        opacityAnimation.stop()
        _isRunning = false
    }

    onTargetChanged: {
        if (target) {
            if (fadeIn && !fadeOut) {
                target.opacity = 0
            }
        }
    }

    NumberAnimation {
        id: opacityAnimation
        target: root.target
        property: "opacity"
        from: root.fromOpacity
        to: root.toOpacity
        duration: root.duration
        easing.type: root.easing

        onFinished: {
            root._isRunning = false
            root.finished()
        }
    }

    // Convenience: Apply fade in on component completion
    Connections {
        enabled: root.fadeIn && root.target
        target: root.target

        function onVisibleChanged() {
            if (root.target.visible) {
                root.start()
            }
        }
    }

    Component.onCompleted: {
        if (target && fadeIn && target.visible) {
            start()
        }
    }
}
