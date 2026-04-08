import QtQuick
import "../services"

Rectangle {
    id: root

    implicitWidth: 80
    implicitHeight: 80

    property string icon: ""
    property string label: ""
    property bool hovered: mouseArea.containsMouse

    signal clicked()

    color: hovered ? Theme.colors.surfaceHover : Theme.colors.surface
    radius: Theme.radius.large
    border.width: 2
    border.color: hovered ? Theme.colors.primary : Theme.colors.surfaceHover

    Behavior on color {
        ColorAnimation {
            duration: Theme.animation.fast
        }
    }

    Behavior on border.color {
        ColorAnimation {
            duration: Theme.animation.fast
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: Theme.spacing.small

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.icon
            font.pixelSize: Theme.fonts.textSizeLarge
            font.family: "Noto Sans"
            color: Theme.colors.text
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.label
            font.pixelSize: Theme.fonts.textSize - 4
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
