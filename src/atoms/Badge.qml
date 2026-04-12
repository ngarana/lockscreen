// Badge.qml - Notification Badge Component
//
// Small notification badge for displaying counts or status indicators.
// Supports color variants and size options.
//
// Usage:
//   Badge {
//       count: 5
//       variant: "error"
//   }
//
// Properties:
//   - count: int - Number to display (0 hides badge)
//   - text: string - Custom text (overrides count)
//   - variant: string - "info", "success", "warning", "error" (default: "info")
//   - size: string - "small", "medium" (default: "medium")
//   - maxCount: int - Maximum number before showing "99+"
//   - dot: bool - Show as dot only (no text)

import QtQuick
import "../theme" as Theme

Rectangle {
    id: root

    // Public API
    property int count: 0
    property string text: ""
    property string variant: "info" // info, success, warning, error
    property string size: "medium" // small, medium
    property int maxCount: 99
    property bool dot: false

    // Internal
    readonly property var _variants: ({
        info: Theme.Theme.colors.info,
        success: Theme.Theme.colors.success,
        warning: Theme.Theme.colors.warning,
        error: Theme.Theme.colors.error
    })

    readonly property color _bgColor: _variants[variant] || _variants.info

    readonly property var _sizes: ({
        small: { height: 16, fontSize: Theme.Theme.typography.sizeXs, padding: 4 },
        medium: { height: 20, fontSize: Theme.Theme.typography.sizeSm, padding: 6 }
    })

    readonly property var _currentSize: _sizes[size] || _sizes.medium

    readonly property string _displayText: {
        if (root.dot) return ""
        if (root.text !== "") return root.text
        if (root.count <= 0) return ""
        if (root.count > root.maxCount) return root.maxCount + "+"
        return String(root.count)
    }

    // Visibility
    visible: (root.count > 0 || root.text !== "") && !root.dot || root.dot

    // Layout
    implicitWidth: root.dot ? _currentSize.height : Math.max(_currentSize.height, textLabel.implicitWidth + _currentSize.padding * 2)
    implicitHeight: _currentSize.height
    radius: height / 2

    // Visual
    color: _bgColor

    // Text label
    Label {
        id: textLabel
        anchors.centerIn: parent
        text: root._displayText
        fontSize: root._currentSize.fontSize
        fontWeight: Theme.Theme.typography.weightBold
        color: Theme.Theme.colors.crust
        visible: !root.dot
    }

    // Pulse animation for attention
    Rectangle {
        id: pulseRing
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        radius: parent.radius
        color: root._bgColor
        opacity: 0.6
        visible: root.variant === "error" && root.count > 0

        SequentialAnimation on scale {
            running: pulseRing.visible
            loops: Animation.Infinite

            NumberAnimation {
                from: 1.0
                to: 1.4
                duration: 1000
                easing.type: Easing.OutQuad
            }

            NumberAnimation {
                from: 1.4
                to: 1.0
                duration: 0
            }
        }

        SequentialAnimation on opacity {
            running: pulseRing.visible
            loops: Animation.Infinite

            NumberAnimation {
                from: 0.6
                to: 0
                duration: 1000
                easing.type: Easing.OutQuad
            }

            NumberAnimation {
                from: 0
                to: 0.6
                duration: 0
            }
        }
    }
}
