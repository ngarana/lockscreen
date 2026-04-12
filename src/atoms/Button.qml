// Button.qml - Base Button Component
//
// Foundation button component with hover, pressed, and disabled states.
// Supports multiple sizes and variants with theme integration.
//
// Usage:
//   Button {
//       text: "Click Me"
//       onClicked: console.log("Clicked")
//   }
//
// Properties:
//   - text: string - Button label text
//   - icon: string - Optional icon source (URL or icon name)
//   - size: string - Button size: "small", "medium", "large" (default: "medium")
//   - variant: string - Button style: "filled", "outlined", "ghost" (default: "filled")
//   - enabled: bool - Whether button is interactive
//   - loading: bool - Show loading spinner instead of content
//
// Signals:
//   - clicked(): Emitted when button is clicked
//   - pressed(): Emitted when button is pressed
//   - released(): Emitted when button is released

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import "../theme" as Theme

Rectangle {
    id: root

    // Public API
    property string text: ""
    property string icon: ""
    property string size: "medium" // small, medium, large
    property string variant: "filled" // filled, outlined, ghost
    property bool loading: false

    signal clicked()
    signal pressed()
    signal released()

    // Internal state
    property bool _isHovered: false
    property bool _isPressed: false

    // Size configuration
    readonly property var _sizes: ({
        small: { height: 28, padding: 12, iconSize: 14, fontSize: Theme.Theme.typography.sizeSm },
        medium: { height: 36, padding: 16, iconSize: 16, fontSize: Theme.Theme.typography.sizeMd },
        large: { height: 44, padding: 24, iconSize: 20, fontSize: Theme.Theme.typography.sizeLg }
    })

    readonly property var _currentSize: _sizes[size] || _sizes.medium

    // Colors based on variant and state
    readonly property color _bgColor: {
        if (variant === "filled") {
            if (!enabled) return Theme.Theme.colors.surface0
            if (_isPressed) return Theme.Theme.colors.blue.darker(1.2)
            if (_isHovered) return Theme.Theme.colors.blue.lighter(1.1)
            return Theme.Theme.colors.blue
        }
        return "transparent"
    }

    readonly property color _borderColor: {
        if (variant === "outlined") {
            if (!enabled) return Theme.Theme.colors.surface1
            if (_isPressed) return Theme.Theme.colors.blue.darker(1.2)
            if (_isHovered) return Theme.Theme.colors.blue
            return Theme.Theme.colors.surface2
        }
        if (variant === "ghost") {
            if (_isHovered && enabled) return Theme.Theme.colors.glassBorder
            return "transparent"
        }
        return "transparent"
    }

    readonly property color _textColor: {
        if (variant === "filled") {
            if (!enabled) return Theme.Theme.colors.textMuted
            return Theme.Theme.colors.crust
        }
        if (!enabled) return Theme.Theme.colors.textMuted
        if (_isPressed) return Theme.Theme.colors.blue.darker(1.2)
        if (_isHovered) return Theme.Theme.colors.blue
        return Theme.Theme.colors.text
    }

    // Layout
    implicitWidth: Math.max(_currentSize.height, contentRow.implicitWidth + _currentSize.padding * 2)
    implicitHeight: _currentSize.height
    radius: Theme.Theme.radius.medium

    // Visual state
    color: _bgColor
    border.width: variant === "outlined" || (variant === "ghost" && _isHovered) ? 1 : 0
    border.color: _borderColor

    Behavior on color {
        ColorAnimation { duration: Theme.Theme.animation.fast }
    }

    Behavior on border.color {
        ColorAnimation { duration: Theme.Theme.animation.fast }
    }

    // Shadow for filled variant
    Rectangle {
        id: shadow
        anchors.fill: parent
        radius: parent.radius
        color: variant === "filled" && enabled && !_isPressed ? Theme.Theme.colors.primaryGlow : "transparent"
        z: -1
        anchors.margins: -2

        Behavior on color {
            ColorAnimation { duration: Theme.Theme.animation.fast }
        }
    }

    // Content row
    RowLayout {
        id: contentRow
        anchors.centerIn: parent
        spacing: 8

        // Loading spinner
        Spinner {
            visible: root.loading
            size: root._currentSize.iconSize
            color: root._textColor
        }

        // Icon
        Icon {
            visible: !root.loading && root.icon !== ""
            source: root.icon
            size: root._currentSize.iconSize
            color: root._textColor
        }

        // Text label
        Label {
            visible: root.text !== ""
            text: root.text
            fontSize: root._currentSize.fontSize
            fontWeight: Theme.Theme.typography.weightMedium
            color: root._textColor
        }
    }

    // Mouse area for interaction
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: root.enabled && !root.loading
        cursorShape: Qt.PointingHandCursor

        onPressed: {
            root._isPressed = true
            root.pressed()
        }

        onReleased: {
            root._isPressed = false
            root.released()
            if (containsMouse) {
                root.clicked()
            }
        }

        onEntered: root._isHovered = true
        onExited: {
            root._isHovered = false
            root._isPressed = false
        }
    }

    // Press animation
    scale: _isPressed ? 0.98 : 1.0

    Behavior on scale {
        NumberAnimation {
            duration: Theme.Theme.animation.fast
            easing.type: Easing.OutQuad
        }
    }
}
