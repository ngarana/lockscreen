// Card.qml - Card/Container Component
//
// Container component with glassmorphic styling, hover effects,
// and customizable border radius.
//
// Usage:
//   Card {
//       width: 300
//       height: 200
//       glassmorphic: true
//
//       // Card content here
//   }
//
// Properties:
//   - glassmorphic: bool - Enable glassmorphism effect
//   - hoverable: bool - Enable hover state changes
//   - elevated: bool - Add shadow elevation
//   - padding: int - Internal padding
//   - borderRadius: int - Corner radius
//   - borderColor: color - Border color
//   - backgroundColor: color - Background color
//
// Signals:
//   - clicked(): Emitted when card is clicked (if hoverable)

import QtQuick
import "../theme" as Theme

Rectangle {
    id: root

    // Public API
    property bool glassmorphic: true
    property bool hoverable: false
    property bool elevated: false
    property int padding: 16
    property int borderRadius: Theme.Theme.radius.large
    property color borderColor: Theme.Theme.colors.glassBorder
    property color backgroundColor: Theme.Theme.colors.glass

    signal clicked()

    // Internal state
    property bool _isHovered: false

    // Layout
    radius: borderRadius
    color: glassmorphic ? (hoverable && _isHovered ? Theme.Theme.colors.glassHover : backgroundColor) : backgroundColor
    border.width: glassmorphic ? 1 : 0
    border.color: borderColor

    Behavior on color {
        ColorAnimation { duration: Theme.Theme.animation.fast }
    }

    // Content container with padding
    default property alias content: contentContainer.children
    Item {
        id: contentContainer
        anchors.fill: parent
        anchors.margins: root.padding
    }

    // Shadow for elevated cards
    Rectangle {
        id: shadow
        anchors.fill: parent
        anchors.margins: -4
        radius: parent.radius + 4
        color: "transparent"
        border.width: 0
        z: -1
        visible: root.elevated || (root.hoverable && root._isHovered)

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: Theme.Theme.colors.shadowMd.color
            opacity: root.elevated ? Theme.Theme.colors.shadowMd.opacity : (root._isHovered ? 0.2 : 0)

            Behavior on opacity {
                NumberAnimation { duration: Theme.Theme.animation.fast }
            }
        }
    }

    // Mouse area for hover/click
    MouseArea {
        anchors.fill: parent
        enabled: root.hoverable
        cursorShape: root.hoverable ? Qt.PointingHandCursor : Qt.ArrowCursor
        hoverEnabled: root.hoverable

        onClicked: {
            if (root.hoverable) {
                root.clicked()
            }
        }

        onEntered: root._isHovered = true
        onExited: root._isHovered = false
    }
}
