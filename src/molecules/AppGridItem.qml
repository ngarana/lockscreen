// AppGridItem.qml - Application Grid Tile Molecule
//
// Grid tile for displaying applications with icon and name.
// Supports launch on click and hover effects.
//
// Usage:
//   AppGridItem {
//       appName: "Firefox"
//       appIcon: "firefox"
//       onLaunch: launchApp(appId)
//   }
//
// Properties:
//   - appId: string - Application ID
//   - appName: string - Display name
//   - appIcon: string - Icon source
//   - appDescription: string - Description tooltip
//
// Signals:
//   - launch(appId): Emitted when app is launched
//   - clicked(): Emitted when tile is clicked

import QtQuick
import "../atoms" as Atoms
import "../theme" as Theme

Atoms.Card {
    id: root

    // Public API
    property string appId: ""
    property string appName: ""
    property string appIcon: ""
    property string appDescription: ""

    signal launch(string appId)
    signal clicked()

    // Internal state
    property bool _isPressed: false

    // Layout
    width: 88
    height: 100
    padding: 8
    hoverable: true
    elevated: false

    // Scale animation on press
    scale: _isPressed ? 0.95 : 1.0

    Behavior on scale {
        NumberAnimation {
            duration: Theme.ThemeEngine.animation.fast
            easing.type: Easing.OutQuad
        }
    }

    // Content column
    Column {
        anchors.fill: parent
        spacing: 8

        // App icon
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 48
            height: 48
            radius: Theme.ThemeEngine.radius.medium
            color: Theme.ThemeEngine.colors.glass

            Atoms.Icon {
                anchors.centerIn: parent
                source: root.appIcon || "application"
                size: 28
                color: Theme.ThemeEngine.colors.textPrimary
            }
        }

        // App name
        Atoms.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - 8
            text: root.appName
            fontSize: Theme.ThemeEngine.typography.sizeSm
            color: Theme.ThemeEngine.colors.textPrimary
            horizontalAlignment: Text.AlignHCenter
            truncate: true
        }
    }

    // Tooltip
    Atoms.Tooltip {
        visible: root._isHovered && root.appDescription !== ""
        text: root.appDescription
        position: "bottom"
        target: root
    }

    // Interaction
    onClicked: {
        root.launch(root.appId)
    }

    // Press handling
    MouseArea {
        anchors.fill: parent
        onPressed: root._isPressed = true
        onReleased: {
            root._isPressed = false
            if (containsMouse) {
                root.clicked()
            }
        }
        onExited: root._isPressed = false
    }
}
