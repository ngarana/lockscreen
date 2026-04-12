// WindowPreview.qml - Window Thumbnail Preview Molecule
//
// Displays a window thumbnail with hover effects.
// Click to focus window. Integrates with HyprlandService.
//
// Usage:
//   WindowPreview {
//       window: windowObject
//       onFocusRequested: focusWindow(window.address)
//   }
//
// Properties:
//   - window: var - Window object from HyprlandService
//   - showTitle: bool - Show window title (default: true)
//   - width: int - Preview width
//   - height: int - Preview height
//
// Signals:
//   - clicked(window): Emitted when preview is clicked
//   - focusRequested(window): Emitted to request window focus

import QtQuick
import "../atoms" as Atoms
import "../services" as Services
import "../theme" as Theme

Atoms.Card {
    id: root

    // Public API
    property var window: null
    property bool showTitle: true

    signal clicked(var window)
    signal focusRequested(var window)

    // Internal state
    readonly property string _title: window ? window.title : ""
    readonly property string _class: window ? window.class : ""
    readonly property int _workspaceId: window ? window.workspace.id : 0
    readonly property bool _isFocused: window ? window.focused : false

    // Layout
    width: 160
    height: showTitle ? 120 : 100
    padding: 4
    hoverable: true
    elevated: _isHovered

    // Visual state
    border.width: _isFocused ? 2 : 1
    border.color: _isFocused ? Theme.Theme.colors.primary : Theme.Theme.colors.glassBorder

    // Thumbnail placeholder
    Rectangle {
        id: thumbnail
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 4
        }
        height: root.showTitle ? 72 : 88
        radius: Theme.Theme.radius.small
        color: Theme.Theme.colors.surface0

        // Window class icon or initial
        Atoms.Label {
            anchors.centerIn: parent
            text: root._class.charAt(0).toUpperCase()
            fontSize: Theme.Theme.typography.size3xl
            color: Theme.Theme.colors.textMuted
            visible: root._class !== ""
        }

        // Focus indicator
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.width: 2
            border.color: Theme.Theme.colors.primary
            visible: root._isFocused
        }
    }

    // Title bar
    Rectangle {
        id: titleBar
        visible: root.showTitle
        anchors {
            top: thumbnail.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: 4
        }
        height: 24
        color: "transparent"

        Atoms.Label {
            anchors.fill: parent
            text: root._title || root._class
            fontSize: Theme.Theme.typography.sizeXs
            color: Theme.Theme.colors.textSecondary
            truncate: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    // Click to focus
    onClicked: {
        if (window) {
            Services.HyprlandService.focusWindow(window.address)
            root.focusRequested(window)
        }
    }
}
