// Clock.qml - Time/Date Display Molecule
//
// Displays current time and date with live updates.
// Supports 12/24 hour format and customizable styling.
//
// Usage:
//   Clock {
//       showDate: true
//       format24Hour: true
//   }
//
// Properties:
//   - showTime: bool - Show time display (default: true)
//   - showDate: bool - Show date display (default: true)
//   - format24Hour: bool - Use 24-hour format (default: true)
//   - showSeconds: bool - Show seconds in time (default: false)
//   - fontSize: int - Base font size
//
// Signals:
//   - clicked(): Emitted when clock is clicked

import QtQuick
import "../atoms" as Atoms
import "../theme" as Theme

Item {
    id: root

    // Public API
    property bool showTime: true
    property bool showDate: true
    property bool format24Hour: true
    property bool showSeconds: false
    property int fontSize: Theme.Theme.typography.size2xl

    signal clicked()

    // Internal state
    property string _timeText: ""
    property string _dateText: ""

    // Layout
    implicitWidth: Math.max(timeLabel.implicitWidth, dateLabel.implicitWidth)
    implicitHeight: showTime && showDate ? timeLabel.height + dateLabel.height + 4 :
                    showTime ? timeLabel.height : dateLabel.height

    // Update timer
    Timer {
        interval: 1000
        running: true
        repeat: true
        triggeredOnStart: true
        onTriggered: root._updateTime()
    }

    // Time display
    Atoms.Label {
        id: timeLabel
        visible: root.showTime
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
        text: root._timeText
        fontSize: root.fontSize
        fontWeight: Theme.Theme.typography.weightLight
        color: Theme.Theme.colors.textPrimary
    }

    // Date display
    Atoms.Label {
        id: dateLabel
        visible: root.showDate
        anchors {
            top: root.showTime ? timeLabel.bottom : parent.top
            topMargin: root.showTime ? 4 : 0
            horizontalCenter: parent.horizontalCenter
        }
        text: root._dateText
        fontSize: Theme.Theme.typography.sizeSm
        color: Theme.Theme.colors.textSecondary
    }

    // Click area
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }

    // Update function
    function _updateTime() {
        var now = new Date()

        // Format time
        var hours = now.getHours()
        var minutes = now.getMinutes()
        var seconds = now.getSeconds()
        var ampm = ""

        if (!root.format24Hour) {
            ampm = hours >= 12 ? " PM" : " AM"
            hours = hours % 12
            hours = hours ? hours : 12
        }

        var timeStr = pad(hours) + ":" + pad(minutes)
        if (root.showSeconds) {
            timeStr += ":" + pad(seconds)
        }
        timeStr += ampm

        root._timeText = timeStr

        // Format date
        var options = { weekday: 'long', year: 'numeric', month: 'long', day: 'numeric' }
        root._dateText = now.toLocaleDateString(Qt.locale(), options)
    }

    function pad(num) {
        return num < 10 ? "0" + num : num
    }

    // Initialize
    Component.onCompleted: _updateTime()
}
