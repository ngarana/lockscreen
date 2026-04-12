// Spinner.qml - Loading Spinner Component
//
// Animated loading spinner with size variants and color customization.
//
// Usage:
//   Spinner {
//       size: 32
//       color: Theme.colors.primary
//   }
//
// Properties:
//   - size: int - Spinner size in pixels (default: 24)
//   - color: color - Spinner color
//   - thickness: int - Ring thickness (default: size/8)
//   - speed: int - Animation duration in ms (default: 1000)

import QtQuick
import "../theme" as Theme

Item {
    id: root

    // Public API
    property int size: 24
    property color color: Theme.Theme.colors.primary
    property int thickness: size / 8
    property int speed: 1000

    // Layout
    implicitWidth: size
    implicitHeight: size

    // Rotation animation container
    Item {
        anchors.fill: parent

        RotationAnimation on rotation {
            loops: Animation.Infinite
            from: 0
            to: 360
            duration: root.speed
        }

        // Spinner ring
        Canvas {
            id: spinnerCanvas
            anchors.fill: parent

            onPaint: {
                var ctx = getContext("2d")
                var centerX = width / 2
                var centerY = height / 2
                var radius = (Math.min(width, height) / 2) - root.thickness

                ctx.clearRect(0, 0, width, height)

                // Draw background track (optional)
                ctx.beginPath()
                ctx.arc(centerX, centerY, radius, 0, 2 * Math.PI)
                ctx.strokeStyle = Qt.rgba(root.color.r, root.color.g, root.color.b, 0.2)
                ctx.lineWidth = root.thickness
                ctx.stroke()

                // Draw animated arc
                ctx.beginPath()
                ctx.arc(centerX, centerY, radius, -Math.PI / 2, Math.PI)
                ctx.strokeStyle = root.color
                ctx.lineWidth = root.thickness
                ctx.lineCap = "round"
                ctx.stroke()
            }

            Component.onCompleted: requestPaint()
        }
    }

    // Alternative: simple rotating arc using Rectangle
    Rectangle {
        id: arcSegment
        anchors.fill: parent
        color: "transparent"
        visible: false // Set to true to use simpler version

        Rectangle {
            width: parent.width
            height: parent.height
            radius: width / 2
            color: "transparent"
            border.width: root.thickness
            border.color: root.color
        }

        // Mask to show only a portion
        Rectangle {
            anchors.fill: parent
            color: "transparent"

            // This creates a partial ring effect
            Rectangle {
                x: parent.width / 2
                y: 0
                width: parent.width / 2
                height: parent.height / 2
                color: "transparent"
            }
        }
    }
}
