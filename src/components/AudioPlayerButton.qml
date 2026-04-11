// AudioPlayerButton.qml - Glassmorphic Transport Button
//
// Circular button for play/pause/next/previous actions.
// Matches ActionButton styling with accessibility support.

import QtQuick
import "../services"

Rectangle {
    id: root

    implicitWidth: Theme.audio.buttonSize
    implicitHeight: Theme.audio.buttonSize

    // ====== Public Properties ======

    property string action: "playPause"  // "playPause", "next", "previous", "stop"
    property string icon: ""
    property string label: ""
    // Note: 'enabled' is inherited from Item; do NOT redeclare it.
    // External bindings (e.g., AudioService.canGoPrevious) set the inherited
    // property directly, which also controls Qt's interactivity mechanism.
    property alias hovered: mouseArea.containsMouse
    property alias pressed: mouseArea.pressed

    signal clicked()

    // ====== Visual Design ======

    color: root.enabled
        ? (root.hovered ? Theme.colors.glassHover : Theme.colors.glass)
        : Qt.rgba(0.3, 0.3, 0.3, 0.4)
    radius: width / 2
    border.width: 1
    border.color: root.enabled
        ? (root.hovered ? Theme.colors.primary : Theme.colors.glassBorder)
        : Theme.colors.textMuted

    // Scale animation on hover
    scale: root.hovered ? 1.1 : 1.0
    opacity: root.enabled ? 1.0 : 0.4

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

    // ====== Icon Glyph ======

    Text {
        id: iconText
        anchors.centerIn: parent
        text: root.icon
        font.pixelSize: Theme.audio.buttonIconSize
        font.family: "Noto Sans"
        // Use non-emoji color glyphs to avoid emoji font fallback issues
        color: root.enabled
            ? (root.hovered ? Theme.colors.primary : Theme.colors.text)
            : Theme.colors.textMuted

        Behavior on color {
            ColorAnimation { duration: Theme.animation.fast }
        }
    }

    // ====== Tooltip ======

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

    // ====== Input Handling ======

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        enabled: root.enabled
        onClicked: root.clicked()
    }

    // ====== Keyboard Activation ======

    Keys.onPressed: (event) => {
        if (!root.enabled) return
        if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return || event.key === Qt.Key_Space) {
            root.clicked()
            event.accepted = true
        }
    }

    // ====== Accessibility ======

    Accessible.role: Accessible.Button
    Accessible.name: root.label
    Accessible.description: "Audio " + root.action + " button"
    Accessible.onPressAction: {
        if (root.enabled) root.clicked()
    }
}
