// Slider.qml - Custom Styled Slider Component
//
// Slider with theme integration, step increments, and value display.
// Supports both horizontal and vertical orientations.
//
// Usage:
//   Slider {
//       from: 0
//       to: 100
//       value: 50
//       stepSize: 5
//       onValueChanged: console.log(value)
//   }
//
// Properties:
//   - value: real - Current slider value
//   - from: real - Minimum value (default: 0)
//   - to: real - Maximum value (default: 100)
//   - stepSize: real - Step increment (default: 1)
//   - orientation: int - Qt.Horizontal or Qt.Vertical
//   - showValue: bool - Show value label
//   - valueFormat: string - Format string for value display

import QtQuick
import "../theme" as Theme

Rectangle {
    id: root

    // Public API
    property real value: 50
    property real from: 0
    property real to: 100
    property real stepSize: 1
    property int orientation: Qt.Horizontal
    property bool showValue: false
    property string valueFormat: "%1"

    // signal valueChanged(real value) - Implicitly provided by property real value
    signal moved()

    // Internal
    property bool _isPressed: false
    property bool _isHovered: false

    // Layout
    implicitWidth: orientation === Qt.Horizontal ? 120 : 24
    implicitHeight: orientation === Qt.Horizontal ? 24 : 120
    color: "transparent"

    // Value calculations
    readonly property real _normalizedValue: (value - from) / (to - from)
    readonly property real _trackLength: orientation === Qt.Horizontal ?
                                          trackArea.width - handle.width : trackArea.height - handle.height

    // Components
    Item {
        id: trackArea
        anchors {
            left: parent.left
            right: showValue ? valueLabel.left : parent.right
            rightMargin: showValue ? 8 : 0
            verticalCenter: parent.verticalCenter
        }
        height: 24

        // Background track
        Rectangle {
            id: trackBg
            anchors.centerIn: parent
            width: orientation === Qt.Horizontal ? parent.width : 4
            height: orientation === Qt.Horizontal ? 4 : parent.height
            radius: 2
            color: Theme.ThemeEngine.colors.surface1
        }

        // Filled track
        Rectangle {
            id: trackFill
            anchors {
                left: trackBg.left
                top: trackBg.top
            }
            width: orientation === Qt.Horizontal ? handle.x + handle.width / 2 : trackBg.width
            height: orientation === Qt.Horizontal ? trackBg.height : handle.y + handle.height / 2
            radius: 2
            color: Theme.ThemeEngine.colors.primary
        }

        // Handle
        Rectangle {
            id: handle
            x: orientation === Qt.Horizontal ? _normalizedValue * _trackLength : (parent.width - width) / 2
            y: orientation === Qt.Horizontal ? (parent.height - height) / 2 : (1 - _normalizedValue) * _trackLength
            width: 16
            height: 16
            radius: 8
            color: root._isPressed ? Theme.ThemeEngine.colors.primary.darker(1.2) : Theme.ThemeEngine.colors.primary
            border.width: 2
            border.color: root._isHovered || root._isPressed ? Theme.ThemeEngine.colors.textPrimary : "transparent"

            Behavior on color {
                ColorAnimation { duration: Theme.ThemeEngine.animation.fast }
            }

            // Glow effect
            Rectangle {
                anchors.centerIn: parent
                width: parent.width + 8
                height: parent.height + 8
                radius: width / 2
                color: Theme.ThemeEngine.colors.primaryGlow
                z: -1
                visible: root._isHovered || root._isPressed
            }
        }

        // Mouse interaction
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true

            onPressed: {
                root._isPressed = true
                updateValue(mouse.x, mouse.y)
            }

            onPositionChanged: {
                if (root._isPressed) {
                    updateValue(mouse.x, mouse.y)
                }
            }

            onReleased: {
                root._isPressed = false
                root.moved()
            }

            onEntered: root._isHovered = true
            onExited: root._isHovered = false

            onWheel: function(wheel) {
                if (wheel.angleDelta.y === 0) return;
                
                let delta = wheel.angleDelta.y > 0 ? root.stepSize : -root.stepSize
                let newValue = root.value + delta
                
                // Clamp to range
                newValue = Math.max(root.from, Math.min(root.to, newValue))
                
                if (newValue !== root.value) {
                    root.value = newValue
                    root.moved()
                }
                wheel.accepted = true
            }

            function updateValue(mouseX, mouseY) {
                let normalized
                if (root.orientation === Qt.Horizontal) {
                    normalized = Math.max(0, Math.min(1, mouseX / trackArea.width))
                } else {
                    normalized = Math.max(0, Math.min(1, 1 - (mouseY / trackArea.height)))
                }

                let newValue = root.from + normalized * (root.to - root.from)

                // Apply step size
                if (root.stepSize > 0) {
                    newValue = Math.round(newValue / root.stepSize) * root.stepSize
                }

                // Clamp to range
                newValue = Math.max(root.from, Math.min(root.to, newValue))

                if (newValue !== root.value) {
                    root.value = newValue
                }
            }
        }
    }

    // Value label
    Label {
        id: valueLabel
        visible: root.showValue
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        text: root.valueFormat.arg(Math.round(root.value))
        fontSize: Theme.ThemeEngine.typography.sizeSm
        color: Theme.ThemeEngine.colors.textSecondary
        width: 30
        horizontalAlignment: Text.AlignRight
    }
}
