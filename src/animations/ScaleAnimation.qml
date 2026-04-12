// ScaleAnimation.qml - Scale Effect Animation
//
// Provides configurable scale animations for UI elements.
// Supports scale in/out with configurable origin point.
//
// Usage:
// ScaleAnimation {
//     target: myItem
//     scaleFrom: 0.5
//     scaleTo: 1.0
//     duration: 200
// }
//
// Properties:
// - target: Item - The item to animate
// - scaleFrom: real - Starting scale value (default: 0.8)
// - scaleTo: real - Ending scale value (default: 1.0)
// - duration: int - Animation duration in ms (default: 200)
// - easing: int - Easing curve type (default: Easing.OutBack)
// - originX: real - Transform origin X (default: 0.5 = center)
// - originY: real - Transform origin Y (default: 0.5 = center)
// - scaleIn: bool - Animate into view (default: true)
// - scaleOut: bool - Animate out of view (default: false)
//
// Signals:
// - started(): Emitted when animation starts
// - finished(): Emitted when animation completes

import QtQuick
import "../theme" as Theme

Item {
    id: root

    property Item target: null
    property real scaleFrom: 0.8
    property real scaleTo: 1.0
    property int duration: Theme.ThemeEngine.animation.fast
    property int easing: Easing.OutBack
    property real originX: 0.5
    property real originY: 0.5
    property bool scaleIn: true
    property bool scaleOut: false

    signal started()
    signal finished()

    property bool _isRunning: false

    function start() {
        if (!target) return
        _isRunning = true
        root.started()

        target.transformOrigin = getTransformOrigin()

        scaleAnimation.from = scaleIn ? scaleFrom : scaleTo
        scaleAnimation.to = scaleIn ? scaleTo : scaleFrom
        scaleAnimation.start()
    }

    function stop() {
        scaleAnimation.stop()
        _isRunning = false
    }

    function getTransformOrigin() {
        if (originX === 0.5 && originY === 0.5) return Item.Center
        if (originX === 0 && originY === 0) return Item.TopLeft
        if (originX === 1 && originY === 0) return Item.TopRight
        if (originX === 0 && originY === 1) return Item.BottomLeft
        if (originX === 1 && originY === 1) return Item.BottomRight
        if (originX === 0.5 && originY === 0) return Item.Top
        if (originX === 0.5 && originY === 1) return Item.Bottom
        if (originX === 0 && originY === 0.5) return Item.Left
        if (originX === 1 && originY === 0.5) return Item.Right
        return Item.Center
    }

    NumberAnimation {
        id: scaleAnimation
        target: root.target
        property: "scale"
        duration: root.duration
        easing.type: root.easing

        onFinished: {
            root._isRunning = false
            root.finished()
        }
    }

    onTargetChanged: {
        if (target) {
            target.transformOrigin = getTransformOrigin()
            if (scaleIn && !scaleOut) {
                target.scale = scaleFrom
            }
        }
    }

    Component.onCompleted: {
        if (target && scaleIn && target.visible) {
            start()
        }
    }
}
