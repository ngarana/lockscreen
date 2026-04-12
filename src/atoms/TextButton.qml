// TextButton.qml - Text Button Component
//
// Button with text label and optional icon. Supports primary,
// secondary, and tertiary variants for different hierarchy levels.
//
// Usage:
//   TextButton {
//       text: "Save Changes"
//       icon: "icons/save.svg"
//       variant: "primary"
//       onClicked: save()
//   }
//
// Properties:
//   - text: string - Button text
//   - icon: string - Optional icon source
//   - iconPosition: string - "left" or "right" (default: "left")
//   - variant: string - "primary", "secondary", "tertiary" (default: "primary")
//   - size: string - "small", "medium", "large" (default: "medium")
//   - destructive: bool - Use error color for destructive actions
//
// Signals:
//   - clicked(): Emitted when button is clicked

import QtQuick
import QtQuick.Layouts
import "../theme" as Theme

Rectangle {
    id: root

    // Public API
    property string text: ""
    property string icon: ""
    property string iconPosition: "left" // left, right
    property string variant: "primary" // primary, secondary, tertiary
    property string size: "medium" // small, medium, large
    property bool destructive: false

    signal clicked()

    // Internal state
    property bool _isHovered: false
    property bool _isPressed: false

    // Size configuration
    readonly property var _sizes: ({
        small: { height: 28, padding: 12, iconSize: 14, fontSize: Theme.Theme.typography.sizeSm },
        medium: { height: 36, padding: 16, iconSize: 16, fontSize: Theme.Theme.typography.sizeMd },
        large: { height: 44, padding: 24, iconSize: 18, fontSize: Theme.Theme.typography.sizeLg }
    })

    readonly property var _currentSize: _sizes[size] || _sizes.medium

    // Colors
    readonly property color _accentColor: destructive ? Theme.Theme.colors.error : Theme.Theme.colors.primary

    readonly property color _bgColor: {
        if (variant === "primary") {
            if (!enabled) return Theme.Theme.colors.surface0
            if (_isPressed) return Qt.darker(_accentColor, 1.2)
            if (_isHovered) return Qt.lighter(_accentColor, 1.1)
            return _accentColor
        }
        if (variant === "secondary") {
            if (!enabled) return Theme.Theme.colors.surface0
            if (_isPressed) return Qt.darker(_accentColor, 1.2)
            if (_isHovered) return Qt.lighter(_accentColor, 1.1)
            return _accentColor
        }
        // tertiary - transparent
        if (!enabled) return "transparent"
        if (_isPressed) return Theme.Theme.colors.glassActive
        if (_isHovered) return Theme.Theme.colors.glassHover
        return "transparent"
    }

    readonly property color _textColor: {
        if (variant === "primary" || variant === "secondary") {
            if (!enabled) return Theme.Theme.colors.textMuted
            return Theme.Theme.colors.crust
        }
        // tertiary
        if (!enabled) return Theme.Theme.colors.textMuted
        if (_isPressed) return _accentColor
        if (_isHovered) return Qt.lighter(_accentColor, 1.2)
        return Theme.Theme.colors.text
    }

    // Layout
    implicitWidth: Math.max(_currentSize.height, contentRow.implicitWidth + _currentSize.padding * 2)
    implicitHeight: _currentSize.height
    radius: variant === "tertiary" ? Theme.Theme.radius.small : Theme.Theme.radius.medium

    // Visual state
    color: _bgColor
    border.width: variant === "tertiary" && _isHovered ? 1 : 0
    border.color: variant === "tertiary" ? Theme.Theme.colors.glassBorder : "transparent"

    Behavior on color {
        ColorAnimation { duration: Theme.Theme.animation.fast }
    }

    // Secondary variant uses outline style
    Rectangle {
        visible: variant === "secondary"
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        border.width: 2
        border.color: {
            if (!enabled) return Theme.Theme.colors.surface1
            if (_isPressed) return Qt.darker(_accentColor, 1.2)
            if (_isHovered) return _accentColor
            return Theme.Theme.colors.surface2
        }

        Behavior on border.color {
            ColorAnimation { duration: Theme.Theme.animation.fast }
        }
    }

    // Content
    RowLayout {
        id: contentRow
        anchors.centerIn: parent
        spacing: 6
        layoutDirection: root.iconPosition === "right" ? Qt.RightToLeft : Qt.LeftToRight

        Icon {
            visible: root.icon !== ""
            source: root.icon
            size: root._currentSize.iconSize
            color: root._textColor
        }

        Label {
            text: root.text
            fontSize: root._currentSize.fontSize
            fontWeight: Theme.Theme.typography.weightMedium
            color: root._textColor
        }
    }

    // Mouse area
    MouseArea {
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
