// BarClock.qml - Clock widget for status bar
//
// Compact clock display with time.
// Designed for horizontal status bar layout.

import QtQuick
import "../../../theme" as Theme

Item {
    id: root

    property bool showSeconds: false

    implicitWidth: timeLabel.implicitWidth
    implicitHeight: timeLabel.implicitHeight

    Text {
        id: timeLabel
        anchors.centerIn: parent
        font.pixelSize: 12
        font.family: Theme.ThemeEngine.typography.fontFamily
        color: Theme.ThemeEngine.colors.textPrimary
    }

    Timer {
        interval: root.showSeconds ? 1000 : 10000
        running: true
        repeat: true
        triggeredOnStart: true
        onTriggered: {
            const now = new Date()
            const h = now.getHours()
            const m = now.getMinutes().toString().padStart(2, "0")
            const s = now.getSeconds().toString().padStart(2, "0")
            timeLabel.text = (h < 10 ? "0" : "") + h + ":" + m + (root.showSeconds ? ":" + s : "")
        }
    }
}