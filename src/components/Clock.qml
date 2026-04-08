import QtQuick
import Quickshell
import "../services"

Item {
    id: root

    implicitWidth: clockText.implicitWidth
    implicitHeight: clockText.implicitHeight + dateText.implicitHeight + Theme.spacing.medium

    property int precision: SystemClock.Seconds
    property string timeFormat: "HH:mm"
    property string dateFormat: "dddd, MMMM d"

    SystemClock {
        id: systemClock
        precision: root.precision
    }

    Column {
        id: column
        anchors.centerIn: parent
        spacing: Theme.spacing.medium

        Text {
            id: clockText
            anchors.horizontalCenter: parent.horizontalCenter
            text: Qt.formatDateTime(systemClock.date, root.timeFormat)
            font.pixelSize: Theme.fonts.textSizeClock
            font.weight: Font.Light
            color: Theme.colors.text
        }

        Text {
            id: dateText
            anchors.horizontalCenter: parent.horizontalCenter
            text: Qt.formatDateTime(systemClock.date, root.dateFormat)
            font.pixelSize: Theme.fonts.textSizeDate
            font.weight: Font.Light
            color: Theme.colors.textSubtle
        }
    }
}
