// ProgressBar.qml - Progress Indicator Component
//
// Linear progress indicator with determinate and indeterminate modes.
// Supports theme colors and animated transitions.
//
// Usage:
//   ProgressBar {
//       value: 0.75  // 75%
//       indeterminate: false
//   }
//
// Properties:
//   - value: real - Progress value 0.0 to 1.0 (for determinate)
//   - indeterminate: bool - Show indeterminate animation
//   - height: int - Bar height in pixels (default: 4)
//   - color: color - Fill color (default: primary)
//   - backgroundColor: color - Background track color
//   - radius: int - Corner radius

import QtQuick
import "../theme" as Theme

Rectangle {
    id: root

    // Public API
    property real value: 0.0
    property bool indeterminate: false
    property int barHeight: 4
    property color fillColor: Theme.ThemeEngine.colors.primary
    property color backgroundColor: Theme.ThemeEngine.colors.surface1
    property int barRadius: 2

    // Internal
    property bool _isAnimating: indeterminate

    // Layout
    implicitWidth: 100
    implicitHeight: barHeight
    radius: barRadius
    color: backgroundColor

    // Clip children to rounded corners
    clip: true

    // Progress fill
    Rectangle {
        id: fill
        height: parent.height
        width: root.indeterminate ? parent.width * 0.3 : parent.width * Math.max(0, Math.min(1, root.value))
        radius: root.barRadius
        color: root.fillColor

        // Position for indeterminate mode
        x: root.indeterminate ? -width : 0

        Behavior on width {
            enabled: !root.indeterminate
            NumberAnimation {
                duration: Theme.ThemeEngine.animation.medium
                easing.type: Easing.OutQuad
            }
        }

        // Indeterminate animation
        SequentialAnimation on x {
            running: root.indeterminate
            loops: Animation.Infinite

            NumberAnimation {
                from: -fill.width
                to: root.width
                duration: 1500
                easing.type: Easing.InOutQuad
            }

            NumberAnimation {
                from: root.width
                to: -fill.width
                duration: 0
            }
        }

        // Gradient overlay for shine effect
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#20ffffff" }
                GradientStop { position: 0.5; color: "transparent" }
                GradientStop { position: 1.0; color: "#10ffffff" }
            }
        }
    }
}
