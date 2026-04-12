// SpringAnimation.qml - Physics-based Spring Animation
//
// Provides physics-based spring animations with configurable
// spring constants, damping, and mass.
//
// Usage:
// SpringAnimation {
//     target: myItem
//     property: "y"
//     from: 100
//     to: 0
// }
//
// Properties:
// - target: Item - The item to animate
// - property: string - The property to animate
// - from: real - Starting value
// - to: real - Ending value
// - spring: real - Spring stiffness (default: 1.5)
// - damping: real - Damping coefficient (default: 0.15)
// - mass: real - Mass of the object (default: 1.0)
// - epsilon: real - Convergence threshold (default: 0.01)
// - duration: int - Maximum duration in ms (default: 500)
//
// Signals:
// - started(): Emitted when animation starts
// - finished(): Emitted when animation completes

import QtQuick
import "../theme" as Theme

Item {
    id: root

    property Item target: null
    property string property: ""
    property real from: 0
    property real to: 0
    property real spring: 1.5
    property real damping: 0.15
    property real mass: 1.0
    property real epsilon: 0.01
    property int duration: Theme.ThemeEngine.animation.slow

    signal started()
    signal finished()

    property bool _isRunning: false
    property real _currentValue: from
    property real _velocity: 0

    function start() {
        if (!target || property === "") return
        _isRunning = true
        _currentValue = from
        _velocity = 0
        root.started()
        animationTimer.start()
    }

    function stop() {
        animationTimer.stop()
        _isRunning = false
        root.finished()
    }

    function reset() {
        animationTimer.stop()
        _isRunning = false
        _currentValue = from
        _velocity = 0
        if (target && property !== "") {
            target[property] = from
        }
    }

    Timer {
        id: animationTimer
        interval: 16
        running: false
        repeat: true

        property real lastTime: 0
        property int elapsed: 0

        onTriggered: {
            if (!root.target) {
                stop()
                return
            }

            var dt = 0.016
            elapsed += interval

            var displacement = root._currentValue - root.to
            var springForce = -root.spring * displacement
            var dampingForce = -root.damping * root._velocity

            var acceleration = (springForce + dampingForce) / root.mass
            root._velocity += acceleration * dt
            root._currentValue += root._velocity * dt

            root.target[root.property] = root._currentValue

            var speed = Math.abs(root._velocity)
            var distance = Math.abs(displacement)

            if ((speed < root.epsilon && distance < root.epsilon) || elapsed >= root.duration) {
                root.target[root.property] = root.to
                stop()
                root._isRunning = false
                root.finished()
            }
        }
    }

    Component.onCompleted: {
        if (target && property !== "") {
            target[property] = from
        }
    }
}
