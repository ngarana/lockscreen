// SlideAnimation.qml - Slide Transition Animation
//
// Provides configurable slide animations for UI elements.
// Supports directional sliding with configurable distance and easing.
//
// Usage:
// SlideAnimation {
//     target: myPanel
//     direction: "left"
//     duration: 300
// }
//
// Properties:
// - target: Item - The item to animate
// - direction: string - Slide direction: "left", "right", "up", "down" (default: "left")
// - distance: int - Slide distance in pixels (0 = target width/height)
// - duration: int - Animation duration in ms (default: 300)
// - easing: int - Easing curve type (default: Easing.OutQuad)
// - slideIn: bool - Animate into view (default: true)
// - slideOut: bool - Animate out of view (default: false)
//
// Signals:
// - started(): Emitted when animation starts
// - finished(): Emitted when animation completes

import QtQuick
import "../theme" as Theme

Item {
    id: root

    property Item target: null
    property string direction: "left"
    property int distance: 0
    property int duration: Theme.ThemeEngine.animation.medium
    property int easing: Easing.OutQuad
    property bool slideIn: true
    property bool slideOut: false

    signal started()
    signal finished()

    property bool _isRunning: false

    readonly property int _slideDistance: {
        if (distance > 0) return distance
        if (!target) return 100
        switch (direction) {
            case "left":
            case "right":
                return target.width
            case "up":
            case "down":
                return target.height
        }
        return 100
    }

    readonly property var _offsets: {
        if (!target) return ({ x: 0, y: 0 })
        switch (direction) {
            case "left":
                return { x: slideIn ? -_slideDistance : 0, y: 0 }
            case "right":
                return { x: slideIn ? _slideDistance : 0, y: 0 }
            case "up":
                return { x: 0, y: slideIn ? -_slideDistance : 0 }
            case "down":
                return { x: 0, y: slideIn ? _slideDistance : 0 }
        }
        return { x: 0, y: 0 }
    }

    function start() {
        if (!target) return
        _isRunning = true
        root.started()

        if (direction === "left" || direction === "right") {
            xAnimation.from = _offsets.x
            xAnimation.to = slideIn ? 0 : (direction === "left" ? -_slideDistance : _slideDistance)
            xAnimation.start()
        } else {
            yAnimation.from = _offsets.y
            yAnimation.to = slideIn ? 0 : (direction === "up" ? -_slideDistance : _slideDistance)
            yAnimation.start()
        }
    }

    function stop() {
        xAnimation.stop()
        yAnimation.stop()
        _isRunning = false
    }

    NumberAnimation {
        id: xAnimation
        target: root.target
        property: "x"
        duration: root.duration
        easing.type: root.easing
        onFinished: {
            root._isRunning = false
            root.finished()
        }
    }

    NumberAnimation {
        id: yAnimation
        target: root.target
        property: "y"
        duration: root.duration
        easing.type: root.easing
        onFinished: {
            root._isRunning = false
            root.finished()
        }
    }

    Component.onCompleted: {
        if (target && slideIn && target.visible) {
            start()
        }
    }
}
