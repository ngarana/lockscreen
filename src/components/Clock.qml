// Clock.qml - Modern Time and Date Display
//
// Large clock with text shadow for readability over video backgrounds.

import QtQuick
import Quickshell
import "../services"

Item {
    id: root

    implicitWidth: column.implicitWidth
    implicitHeight: column.implicitHeight

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
        spacing: Theme.spacing.small

        // Time display - Large, ultra-light
        Text {
            id: clockText
            anchors.horizontalCenter: parent.horizontalCenter
            text: Qt.formatDateTime(systemClock.date, root.timeFormat)
            font.pixelSize: Theme.fonts.textSizeClock
            font.family: Theme.fonts.fontFamily
            font.weight: Font.Thin
            color: Theme.colors.text

            // Drop shadow for video readability
            layer.enabled: true
            layer.effect: Item {
                property var source
                ShaderEffect {
                    anchors.fill: parent
                    property var source: parent.source
                }
            }

            // Manual text shadow using duplicate
            Text {
                z: -1
                anchors.fill: parent
                anchors.topMargin: Theme.effects.shadowOffset
                anchors.leftMargin: Theme.effects.shadowOffset
                text: parent.text
                font: parent.font
                color: Qt.rgba(0, 0, 0, Theme.effects.shadowOpacity)
            }
        }

        // Date display
        Text {
            id: dateText
            anchors.horizontalCenter: parent.horizontalCenter
            text: Qt.formatDateTime(systemClock.date, root.dateFormat)
            font.pixelSize: Theme.fonts.textSizeDate
            font.family: Theme.fonts.fontFamily
            font.weight: Font.Light
            color: Theme.colors.textSubtle

            Text {
                z: -1
                anchors.fill: parent
                anchors.topMargin: Theme.effects.shadowOffset
                anchors.leftMargin: Theme.effects.shadowOffset
                text: parent.text
                font: parent.font
                color: Qt.rgba(0, 0, 0, Theme.effects.shadowOpacity)
            }
        }
    }
}
