// DrawerLayout.qml - Slide-out Drawer Layout
//
// A slide-out drawer panel with backdrop and animations.
// Supports left, right, top, and bottom drawer positions.
//
// Usage:
// DrawerLayout {
//     visible: true
//     position: "left"
//     width: 300
//     Item { ... }
// }
//
// Properties:
// - visible: bool - Whether drawer is shown
// - position: string - "left", "right", "top", "bottom" (default: "left")
// - drawerWidth: int - Drawer width for horizontal (default: 300)
// - drawerHeight: int - Drawer height for vertical (default: 400)
// - backdropEnabled: bool - Show backdrop overlay (default: true)
// - backdropOpacity: real - Backdrop opacity (default: 0.5)
// - animationDuration: int - Animation duration in ms (default: 300)
//
// Signals:
// - opened(): Emitted when drawer opens
// - closed(): Emitted when drawer closes

import QtQuick
import "../theme" as Theme

Item {
    id: root

    property bool visible: false
    property string position: "left"
    property int drawerWidth: 300
    property int drawerHeight: 400
    property bool backdropEnabled: true
    property real backdropOpacity: 0.5
    property int animationDuration: Theme.ThemeEngine.animation.medium

    signal opened()
    signal closed()

    implicitWidth: parent ? parent.width : 0
    implicitHeight: parent ? parent.height : 0

    // Backdrop overlay
    Rectangle {
        id: backdrop
        anchors.fill: parent
        color: Theme.ThemeEngine.colors.crust
        opacity: root.visible && root.backdropEnabled ? root.backdropOpacity : 0
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation {
                duration: root.animationDuration
                easing.type: Easing.OutQuad
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.visible = false
                root.closed()
            }
        }
    }

    // Drawer panel
    Rectangle {
        id: drawerPanel
        color: Theme.ThemeEngine.colors.surface0

        readonly property bool isHorizontal: root.position === "left" || root.position === "right"
        readonly property bool isFromStart: root.position === "left" || root.position === "top"

        width: isHorizontal ? root.drawerWidth : parent.width
        height: isHorizontal ? parent.height : root.drawerHeight

        x: {
            if (isHorizontal) {
                return isFromStart ?
                    (root.visible ? 0 : -width) :
                    (root.visible ? parent.width - width : parent.width)
            }
            return 0
        }

        y: {
            if (!isHorizontal) {
                return isFromStart ?
                    (root.visible ? 0 : -height) :
                    (root.visible ? parent.height - height : parent.height)
            }
            return 0
        }

        Behavior on x {
            enabled: isHorizontal
            NumberAnimation {
                duration: root.animationDuration
                easing.type: Easing.OutQuad
            }
        }

        Behavior on y {
            enabled: !isHorizontal
            NumberAnimation {
                duration: root.animationDuration
                easing.type: Easing.OutQuad
            }
        }

        // Border on exposed edge
        Rectangle {
            anchors {
                left: root.position === "right" ? parent.left : undefined
                right: root.position === "left" ? parent.right : undefined
                top: root.position === "bottom" ? parent.top : undefined
                bottom: root.position === "top" ? parent.bottom : undefined
            }
            width: root.position === "left" || root.position === "right" ? 1 : parent.width
            height: root.position === "top" || root.position === "bottom" ? 1 : parent.height
            color: Theme.ThemeEngine.colors.surface1
        }

        // Content container
        Item {
            anchors.fill: parent
            anchors.margins: Theme.ThemeEngine.spacing.md
            clip: true

            default property alias children: drawerPanel.contentItem.children
            property Item contentItem: Item {
                anchors.fill: parent
            }
        }
    }

    onVisibleChanged: {
        if (visible) {
            root.opened()
        }
    }
}
