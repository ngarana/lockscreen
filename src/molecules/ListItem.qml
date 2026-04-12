// ListItem.qml - List Item with Icon, Text, Actions Molecule
//
// Standard list item with icon, primary/secondary text,
// and optional action buttons. Supports hover effects and selection.
//
// Usage:
//   ListItem {
//       icon: "folder"
//       title: "Documents"
//       subtitle: "12 items"
//       onClicked: openFolder()
//   }
//
// Properties:
//   - icon: string - Icon source
//   - title: string - Primary text
//   - subtitle: string - Secondary text
//   - selected: bool - Selection state
//   - showArrow: bool - Show right arrow
//
// Signals:
//   - clicked(): Emitted when item is clicked
//   - actionClicked(): Emitted when action button is clicked

import QtQuick
import QtQuick.Layouts
import "../atoms" as Atoms
import "../theme" as Theme

Rectangle {
    id: root

    // Public API
    property string icon: ""
    property string title: ""
    property string subtitle: ""
    property bool selected: false
    property bool showArrow: false

    signal clicked()
    signal actionClicked()

    // Internal state
    property bool _isHovered: false

    // Layout
    implicitWidth: 200
    implicitHeight: contentRow.implicitHeight + 16
    radius: Theme.Theme.radius.medium

    // Visual state
    color: root.selected ? Theme.Theme.colors.glassActive :
           root._isHovered ? Theme.Theme.colors.glassHover : "transparent"

    Behavior on color {
        ColorAnimation { duration: Theme.Theme.animation.fast }
    }

    // Content row
    RowLayout {
        id: contentRow
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 12

        // Icon
        Atoms.Icon {
            visible: root.icon !== ""
            source: root.icon
            size: 20
            color: root.selected ? Theme.Theme.colors.primary : Theme.Theme.colors.textSecondary
        }

        // Text content
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Atoms.Label {
                Layout.fillWidth: true
                text: root.title
                fontSize: Theme.Theme.typography.sizeMd
                color: root.selected ? Theme.Theme.colors.primary : Theme.Theme.colors.textPrimary
                truncate: true
            }

            Atoms.Label {
                Layout.fillWidth: true
                visible: root.subtitle !== ""
                text: root.subtitle
                fontSize: Theme.Theme.typography.sizeSm
                color: Theme.Theme.colors.textSecondary
                truncate: true
            }
        }

        // Arrow
        Atoms.Icon {
            visible: root.showArrow
            source: "chevron-right"
            size: 16
            color: Theme.Theme.colors.textMuted
        }
    }

    // Click area
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true

        onClicked: root.clicked()
        onEntered: root._isHovered = true
        onExited: root._isHovered = false
    }
}
