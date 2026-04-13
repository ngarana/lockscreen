// IconButton.qml - Icon-Only Button Component
//
// Button component for icon-only actions. Includes hover effects,
// tooltip support, and circular variant option.
//
// Usage:
//   IconButton {
//       icon: "icons/close.svg"
//       tooltip: "Close"
//       onClicked: closeWindow()
//   }
//
// Properties:
//   - icon: string - Icon source URL or icon name
//   - size: int - Button size in pixels (default: 36)
//   - iconSize: int - Icon size in pixels (default: 20)
//   - circular: bool - Use circular shape instead of rounded
//   - tooltip: string - Tooltip text
//   - tooltipPosition: string - "top", "bottom", "left", "right"
//
// Signals:
//   - clicked(): Emitted when button is clicked

import QtQuick
import "../theme" as Theme

Rectangle {
    id: root

    // Public API
    property string icon: ""
    property int size: 36
    property int iconSize: 20
    property bool circular: false
    property string tooltip: ""
    property string tooltipPosition: "top"

    signal clicked()
    signal wheel(var wheel)

    // Internal state
    property bool _isHovered: false
    property bool _isPressed: false

    // Layout
    implicitWidth: size
    implicitHeight: size
    radius: circular ? size / 2 : Theme.ThemeEngine.radius.small

    // Visual state
    color: {
        if (!enabled) return "transparent"
        if (_isPressed) return Theme.ThemeEngine.colors.glassActive
        if (_isHovered) return Theme.ThemeEngine.colors.glassHover
        return "transparent"
    }



    border.width: 0

    Behavior on color {
        ColorAnimation { duration: Theme.ThemeEngine.animation.fast }
    }

    // Icon
    Icon {
        id: iconItem
        anchors.centerIn: parent
        source: root.icon
        size: root.iconSize
        color: {
            if (!enabled) return Theme.ThemeEngine.colors.textMuted
            if (_isPressed) return Theme.ThemeEngine.colors.primary
            if (_isHovered) return Theme.ThemeEngine.colors.textPrimary
            return Theme.ThemeEngine.colors.textSecondary
        }
    }

    // Tooltip
    Tooltip {
        visible: root.tooltip !== "" && root._isHovered && !root._isPressed
        text: root.tooltip
        position: root.tooltipPosition
        target: root
    }

    // Mouse area
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: root.enabled
        cursorShape: Qt.PointingHandCursor

        onPressed: root._isPressed = true
        onReleased: {
            root._isPressed = false
            if (containsMouse) {
                root.clicked()
            }
        }

        onEntered: root._isHovered = true
        onExited: {
            root._isHovered = false
            root._isPressed = false
        }

        onWheel: root.wheel(wheel)
    }

    // Press animation
    scale: _isPressed ? 0.92 : 1.0

    Behavior on scale {
        NumberAnimation {
            duration: Theme.ThemeEngine.animation.fast
            easing.type: Easing.OutQuad
        }
    }
}
