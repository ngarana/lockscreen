// Tooltip.qml - Tooltip Component
//
// Tooltip that automatically positions itself relative to a target.
// Supports fade animations and multiple positions.
//
// Usage:
//   Tooltip {
//       target: myButton
//       text: "Click me!"
//       position: "bottom"
//   }
//
// Properties:
//   - target: Item - Target item to position relative to
//   - text: string - Tooltip text
//   - position: string - "top", "bottom", "left", "right" (default: "top")
//   - delay: int - Show delay in ms (default: 500)
//   - visible: bool - Control visibility

import QtQuick
import "../theme" as Theme

Rectangle {
    id: root

    // Public API
    property Item target: null
    property string text: ""
    property string position: "top" // top, bottom, left, right
    property int delay: 500

    // Internal
    property bool _shouldShow: false

    // Layout
    implicitWidth: textLabel.implicitWidth + 16
    implicitHeight: textLabel.implicitHeight + 10
    radius: Theme.Theme.radius.small
    color: Theme.Theme.colors.surface0
    border.width: 1
    border.color: Theme.Theme.colors.glassBorder

    opacity: 0
    scale: 0.9
    visible: opacity > 0

    // Shadow
    Rectangle {
        anchors.fill: parent
        anchors.margins: -2
        radius: parent.radius + 2
        color: Theme.Theme.colors.shadowMd.color
        opacity: Theme.Theme.colors.shadowMd.opacity
        z: -1
    }

    // Text
    Label {
        id: textLabel
        anchors.centerIn: parent
        text: root.text
        fontSize: Theme.Theme.typography.sizeSm
        color: Theme.Theme.colors.textPrimary
    }

    // Positioning
    onTargetChanged: updatePosition()
    onPositionChanged: updatePosition()

    function updatePosition() {
        if (!target || !target.parent) return

        // Convert target position to tooltip parent coordinates
        var targetPos = target.mapToItem(target.parent, 0, 0)
        var offset = 8

        switch (position) {
            case "top":
                x = targetPos.x + (target.width - width) / 2
                y = targetPos.y - height - offset
                break
            case "bottom":
                x = targetPos.x + (target.width - width) / 2
                y = targetPos.y + target.height + offset
                break
            case "left":
                x = targetPos.x - width - offset
                y = targetPos.y + (target.height - height) / 2
                break
            case "right":
                x = targetPos.x + target.width + offset
                y = targetPos.y + (target.height - height) / 2
                break
        }

        // Keep within parent bounds
        if (x < 0) x = 4
        if (x + width > target.parent.width) x = target.parent.width - width - 4
        if (y < 0) y = 4
        if (y + height > target.parent.height) y = target.parent.height - height - 4
    }

    // Show/hide animations
    states: [
        State {
            name: "visible"
            when: root._shouldShow && root.text !== ""
            PropertyChanges {
                target: root
                opacity: 1
                scale: 1.0
            }
        }
    ]

    transitions: [
        Transition {
            from: ""
            to: "visible"
            NumberAnimation {
                properties: "opacity,scale"
                duration: Theme.Theme.animation.fast
                easing.type: Easing.OutQuad
            }
        },
        Transition {
            from: "visible"
            to: ""
            NumberAnimation {
                properties: "opacity,scale"
                duration: Theme.Theme.animation.fast
                easing.type: Easing.InQuad
            }
        }
    ]

    // Delay timer
    Timer {
        id: showTimer
        interval: root.delay
        onTriggered: {
            if (root._shouldShow) {
                updatePosition()
            }
        }
    }

    // Public methods
    function show() {
        root._shouldShow = true
        showTimer.start()
    }

    function hide() {
        root._shouldShow = false
        showTimer.stop()
    }
}
