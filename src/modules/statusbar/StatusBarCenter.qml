// StatusBarCenter.qml - Center Section of Status Bar
//
// Center section containing:
// - Clock display (time only, compact)
// - Date display (optional, shown on hover or always)
// - Calendar popup on click
//
// macOS-style: centered clock pill with minimal styling.

import QtQuick
import QtQuick.Layouts
import "../../atoms" as Atoms
import "../../molecules" as Molecules
import "../../services" as Services
import "../../theme" as Theme

Rectangle {
    id: root

    // ========================================================================
    // Public Properties
    // ========================================================================

    // Show date below time
    property bool showDate: false

    // Show seconds in time
    property bool showSeconds: false

    // Use 24-hour format
    property bool format24Hour: true

    // Font size
    property int fontSize: Theme.ThemeEngine.typography.sizeSm

    // Calendar popup visibility
    property bool calendarVisible: false

    // ========================================================================
    // Visual Configuration
    // ========================================================================

    color: "transparent"
    implicitWidth: timeLabel.implicitWidth + (root.showDate ? dateLabel.implicitWidth + 8 : 0) + 20
    implicitHeight: root.showDate ? timeLabel.height + dateLabel.height + 12 : timeLabel.height + 8

    // ========================================================================
    // Time Display
    // ========================================================================

    Atoms.Label {
        id: timeLabel
        anchors {
            top: parent.top
            topMargin: 4
            horizontalCenter: parent.horizontalCenter
        }
        text: root._timeText
        fontSize: root.fontSize
        fontWeight: Theme.ThemeEngine.typography.weightMedium
        color: Theme.ThemeEngine.colors.textPrimary
    }

    // ========================================================================
    // Date Display (Optional)
    // ========================================================================

    Atoms.Label {
        id: dateLabel
        visible: root.showDate
        anchors {
            top: timeLabel.bottom
            topMargin: 2
            horizontalCenter: parent.horizontalCenter
        }
        text: root._dateText
        fontSize: Theme.ThemeEngine.typography.sizeXs
        color: Theme.ThemeEngine.colors.textSecondary
    }

    // ========================================================================
    // Click Area for Calendar Popup
    // ========================================================================

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            // TODO: Toggle calendar popup when Calendar component is ready
            root.calendarVisible = !root.calendarVisible
        }
    }

    // Hover effect
    Rectangle {
        id: hoverEffect
        anchors.fill: parent
        radius: Theme.ThemeEngine.radius.small
        color: Theme.ThemeEngine.colors.surface1
        opacity: mouseArea.containsMouse ? 0.3 : 0

        Behavior on opacity {
            NumberAnimation {
                duration: Theme.ThemeEngine.animation.fast
            }
        }
    }

    // Ensure hover effect is behind text
    z: -1

    // ========================================================================
    // Internal State
    // ========================================================================

    property string _timeText: ""
    property string _dateText: ""

    // ========================================================================
    // Update Timer
    // ========================================================================

    Timer {
        id: updateTimer
        interval: 1000
        running: true
        repeat: true
        triggeredOnStart: true
        onTriggered: root._updateTime()
    }

    // ========================================================================
    // Update Function
    // ========================================================================

    function _updateTime() {
        const now = new Date()

        // Format time
        let hours = now.getHours()
        const minutes = now.getMinutes()
        const seconds = now.getSeconds()
        let ampm = ""

        if (!root.format24Hour) {
            ampm = hours >= 12 ? " PM" : " AM"
            hours = hours % 12
            hours = hours ? hours : 12
        }

        let timeStr = root._pad(hours) + ":" + root._pad(minutes)
        if (root.showSeconds) {
            timeStr += ":" + root._pad(seconds)
        }
        timeStr += ampm

        root._timeText = timeStr

        // Format date manually to avoid locale issues
        const months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec']
        root._dateText = months[now.getMonth()] + ' ' + now.getDate() + ', ' + now.getFullYear()
    }

    function _pad(num) {
        return num < 10 ? "0" + num : num
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        _updateTime()
    }
}
