// ActionButton.qml - Circular Glassmorphic Power Button
//
// Modern icon-only circular button with glass effect.
// Scale animation on hover, translucent background.

import QtQuick
import "../services"

Rectangle {
    id: root

    implicitWidth: 52
    implicitHeight: 52

    property string icon: ""
    property string label: ""  // Used for tooltip only
    property bool hovered: mouseArea.containsMouse

    signal clicked()

    // Circular shape with glassmorphism
    color: hovered ? Theme.colors.glassHover : Theme.colors.glass
    radius: width / 2
    border.width: 1
    border.color: hovered ? Theme.colors.primary : Theme.colors.glassBorder

    // Scale animation on hover
    scale: hovered ? 1.1 : 1.0

    Behavior on color {
        ColorAnimation { duration: Theme.animation.fast }
    }

    Behavior on border.color {
        ColorAnimation { duration: Theme.animation.fast }
    }

    Behavior on scale {
        NumberAnimation {
            duration: Theme.animation.medium
            easing.type: Easing.OutBack
        }
    }

    // Icon text
    Text {
        anchors.centerIn: parent
        text: root.icon
        font.pixelSize: 20
        font.family: Theme.fonts.iconFontFamily
        color: root.hovered ? Theme.colors.primary : Theme.colors.text
    }

    // Tooltip on hover
    Rectangle {
        id: tooltip
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.top
        anchors.bottomMargin: 8
        width: tooltipText.implicitWidth + Theme.spacing.medium
        height: tooltipText.implicitHeight + Theme.spacing.small
        color: Theme.colors.glass
        radius: Theme.radius.medium
        border.width: 1
        border.color: Theme.colors.glassBorder
        visible: root.hovered && root.label !== ""

        opacity: root.hovered ? 1.0 : 0.0
        Behavior on opacity {
            NumberAnimation { duration: Theme.animation.fast }
        }

        Text {
            id: tooltipText
            anchors.centerIn: parent
            text: root.label
            font.pixelSize: 12
            font.family: Theme.fonts.fontFamily
            color: Theme.colors.textSubtle
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
