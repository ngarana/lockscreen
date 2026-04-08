import QtQuick
import "../services"

Item {
    id: root

    implicitWidth: statusText.implicitWidth + Theme.spacing.medium * 2
    implicitHeight: statusText.implicitHeight + Theme.spacing.small

    property string message: ""
    property bool isError: false

    Text {
        id: statusText
        anchors.centerIn: parent
        text: root.message
        font.pixelSize: Theme.fonts.textSize
        color: root.isError ? Theme.colors.error : Theme.colors.textSubtle
        visible: root.message !== ""

        Behavior on color {
            ColorAnimation {
                duration: Theme.animation.fast
            }
        }
    }
}
