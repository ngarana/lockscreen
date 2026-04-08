// StatusMessage.qml - Authentication Status Display
//
// Displays status messages with text shadow for video readability.
// Supports fade in/out animation.

import QtQuick
import "../services"

Item {
    id: root

    implicitWidth: statusText.implicitWidth + Theme.spacing.medium * 2
    implicitHeight: statusText.implicitHeight + Theme.spacing.small

    property string message: ""
    property bool isError: false

    opacity: root.message !== "" ? 1.0 : 0.0

    Behavior on opacity {
        NumberAnimation {
            duration: Theme.animation.medium
            easing.type: Easing.InOutQuad
        }
    }

    Text {
        id: statusText
        anchors.centerIn: parent

        text: root.message
        font.pixelSize: Theme.fonts.textSize
        font.family: Theme.fonts.fontFamily
        font.weight: Font.Medium

        color: root.isError ? Theme.colors.error : Theme.colors.textSubtle

        Behavior on color {
            ColorAnimation { duration: Theme.animation.fast }
        }

        // Text shadow for video readability
        Text {
            z: -1
            anchors.fill: parent
            anchors.topMargin: 1
            anchors.leftMargin: 1
            text: parent.text
            font: parent.font
            color: Qt.rgba(0, 0, 0, Theme.effects.shadowOpacity)
        }
    }
}
