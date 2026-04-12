// PopupLayout.qml - Centered Popup/Dialog Layout
//
// A centered popup container with backdrop overlay and animations.
// Supports modal and non-modal popups with configurable styling.
//
// Usage:
// PopupLayout {
//     visible: true
//     modal: true
//     Popup { ... }
// }
//
// Properties:
// - visible: bool - Whether popup is shown
// - modal: bool - Show backdrop overlay (default: true)
// - backdropColor: color - Backdrop overlay color
// - backdropOpacity: real - Backdrop opacity (default: 0.5)
// - closeOnBackdrop: bool - Close when clicking backdrop (default: true)
// - animationDuration: int - Animation duration in ms (default: 300)
//
// Signals:
// - opened(): Emitted when popup opens
// - closed(): Emitted when popup closes
// - backdropClicked(): Emitted when backdrop is clicked

import QtQuick
import "../theme" as Theme

Item {
    id: root

    property bool visible: false
    property bool modal: true
    property color backdropColor: Theme.Theme.colors.crust
    property real backdropOpacity: 0.5
    property bool closeOnBackdrop: true
    property int animationDuration: Theme.Theme.animation.medium

    signal opened()
    signal closed()
    signal backdropClicked()

    implicitWidth: parent ? parent.width : 0
    implicitHeight: parent ? parent.height : 0
    visible: root.visible

    // Backdrop overlay
    Rectangle {
        id: backdrop
        anchors.fill: parent
        color: root.backdropColor
        opacity: root.visible && root.modal ? root.backdropOpacity : 0
        visible: root.visible && root.modal

        Behavior on opacity {
            NumberAnimation {
                duration: root.animationDuration
                easing.type: Easing.OutQuad
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.backdropClicked()
                if (root.closeOnBackdrop) {
                    root.visible = false
                    root.closed()
                }
            }
        }
    }

    // Popup container
    Item {
        id: popupContainer
        anchors.centerIn: parent
        visible: root.visible
        opacity: root.visible ? 1 : 0
        scale: root.visible ? 1 : 0.95

        Behavior on opacity {
            NumberAnimation {
                duration: root.animationDuration
                easing.type: Easing.OutQuad
            }
        }

        Behavior on scale {
            NumberAnimation {
                duration: root.animationDuration
                easing.type: Easing.OutBack
            }
        }

        // Default property - children go here
        default property alias children: popupContainer.children
    }

    // Keyboard handling
    Keys.onEscapePressed: {
        if (root.visible) {
            root.visible = false
            root.closed()
        }
    }

    onVisibleChanged: {
        if (visible) {
            root.opened()
        }
    }
}
