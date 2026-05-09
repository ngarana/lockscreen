// MenuBarButton.qml - Reusable status bar button/chip
//
// Provides a compact macOS-style button for the status bar with
// optional icon, label, tooltip, and wheel handling.

import QtQuick
import QtQuick.Layouts
import "../../atoms" as Atoms
import "../../theme" as Theme

Rectangle {
    id: root

    property string leadingIcon: ""
    property string label: ""
    property string trailingLabel: ""
    property string tooltip: ""
    property bool active: false
    property bool showBackground: true
    property int itemHeight: 20
    property int horizontalPadding: 6
    property int contentSpacing: 4
    property int iconSize: 12
    property int textSize: 11
    property int textWeight: Theme.ThemeEngine.typography.weightMedium
    property color foregroundColor: Theme.ThemeEngine.colors.textPrimary

    signal clicked()
    signal rightClicked()
    signal wheel(var wheel)

    readonly property bool _hovered: mouseArea.containsMouse
    readonly property bool _pressed: mouseArea.pressed
    readonly property color _backgroundColor: {
        if (!root.showBackground) {
            return "transparent"
        }
        if (root._pressed) {
            return Qt.alpha(Theme.ThemeEngine.colors.rosewater, 0.12)
        }
        if (root.active) {
            return Qt.alpha(Theme.ThemeEngine.colors.primary, 0.16)
        }
        if (root._hovered) {
            return Qt.alpha(Theme.ThemeEngine.colors.rosewater, 0.08)
        }
        return "transparent"
    }

    implicitWidth: contentRow.implicitWidth + (root.horizontalPadding * 2)
    implicitHeight: root.itemHeight
    radius: 6
    color: root._backgroundColor
    border.width: root.active ? 1 : 0
    border.color: Qt.alpha(Theme.ThemeEngine.colors.rosewater, 0.12)

    Behavior on color {
        ColorAnimation {
            duration: Theme.ThemeEngine.animation.fast
        }
    }

    Behavior on scale {
        NumberAnimation {
            duration: Theme.ThemeEngine.animation.fast
            easing.type: Easing.OutQuad
        }
    }

    scale: root._pressed ? 0.97 : 1.0

    RowLayout {
        id: contentRow
        anchors.centerIn: parent
        spacing: root.contentSpacing

        Atoms.Icon {
            visible: root.leadingIcon !== ""
            icon: root.leadingIcon
            size: root.iconSize
            color: root.foregroundColor
        }

        Atoms.Label {
            visible: root.label !== ""
            text: root.label
            fontSize: root.textSize
            fontWeight: root.textWeight
            color: root.foregroundColor
        }

        Atoms.Label {
            visible: root.trailingLabel !== ""
            text: root.trailingLabel
            fontSize: root.textSize
            fontWeight: Theme.ThemeEngine.typography.weightRegular
            color: Qt.alpha(root.foregroundColor, 0.82)
        }
    }

    Atoms.Tooltip {
        target: root
        visible: root.tooltip !== "" && root._hovered && !root._pressed
        text: root.tooltip
        position: "bottom"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        cursorShape: Qt.PointingHandCursor

        onClicked: function(mouse) {
            if (mouse.button === Qt.RightButton) {
                root.rightClicked()
            } else {
                root.clicked()
            }
        }

        onWheel: root.wheel(wheel)
    }
}
