// Calendar.qml - Calendar Widget Component
//
// A glassmorphic calendar widget with month/year navigation.
// Displays current month with day grid and highlights today.
//
// Properties:
//   - selectedDate: Date - Currently selected date
//   - showYearNavigation: bool - Show year navigation controls
//   - eventDates: array - Array of dates with events for indicators
//   - firstDayOfWeek: int - 0 = Sunday, 1 = Monday
//
// Signals:
//   - dateSelected(date: Date): Emitted when a date is clicked
//   - monthChanged(year: int, month: int): Emitted when month changes

import QtQuick
import QtQuick.Layouts
import "../services" as Services
import "../atoms" as Atoms

Item {
    id: root

    implicitWidth: 320
    implicitHeight: 360

    // ====== Public Properties ======

    property var selectedDate: new Date()
    property bool showYearNavigation: true
    property var eventDates: []  // Array of Date objects with events
    property int firstDayOfWeek: 1  // 0 = Sunday, 1 = Monday (default)

    // Current view state
    property int currentYear: _today.getFullYear()
    property int currentMonth: _today.getMonth()

    // Internal
    readonly property var _today: new Date()
    readonly property var _monthNames: [
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    ]
    readonly property var _dayNames: firstDayOfWeek === 0 ?
        ["Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"] :
        ["Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"]

    // Signals
    signal dateSelected(var date)
    signal monthChanged(int year, int month)

    // ====== Glassmorphic Panel ======

    Rectangle {
        id: panel
        anchors.fill: parent

        color: Services.Theme.colors.glass
        radius: Services.Theme.radius.large
        border.width: 1
        border.color: Services.Theme.colors.glassBorder

        ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            anchors.margins: Services.Theme.spacing.medium
            spacing: Services.Theme.spacing.medium

            // ====== Header: Month/Year + Navigation ======

            RowLayout {
                id: headerLayout
                Layout.fillWidth: true
                spacing: Services.Theme.spacing.small

                // Previous month button
                Atoms.IconButton {
                    implicitWidth: 32
                    implicitHeight: 32
                    icon: "←"
                    iconSize: 16
                    onClicked: _previousMonth()
                }

                // Month/Year display
                Item {
                    Layout.fillWidth: true
                    implicitHeight: monthYearLabel.height

                    Row {
                        id: monthYearRow
                        anchors.centerIn: parent
                        spacing: Services.Theme.spacing.small

                        Text {
                            id: monthYearLabel
                            text: _monthNames[root.currentMonth] + " " + root.currentYear
                            font.pixelSize: Services.Theme.fonts.textSize
                            font.family: Services.Theme.fonts.fontFamily
                            font.weight: Font.Bold
                            color: Services.Theme.colors.text
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: _goToToday()
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true

                        Rectangle {
                            anchors.fill: parent
                            color: "transparent"
                            border.width: parent.containsMouse ? 1 : 0
                            border.color: Services.Theme.colors.glassBorder
                            radius: Services.Theme.radius.small
                        }
                    }
                }

                // Next month button
                Atoms.IconButton {
                    implicitWidth: 32
                    implicitHeight: 32
                    icon: "→"
                    iconSize: 16
                    onClicked: _nextMonth()
                }

                // Year navigation (optional)
                Row {
                    visible: root.showYearNavigation
                    spacing: 4

                    Atoms.IconButton {
                        implicitWidth: 24
                        implicitHeight: 24
                        icon: "←"
                        iconSize: 12
                        onClicked: _previousYear()
                    }

                    Atoms.IconButton {
                        implicitWidth: 24
                        implicitHeight: 24
                        icon: "→"
                        iconSize: 12
                        onClicked: _nextYear()
                    }
                }
            }

            // ====== Day Names Header ======

            RowLayout {
                id: dayNamesRow
                Layout.fillWidth: true
                spacing: 4

                Repeater {
                    model: 7

                    Item {
                        Layout.fillWidth: true
                        implicitHeight: dayNameText.height + 8

                        Text {
                            id: dayNameText
                            anchors.centerIn: parent
                            text: root._dayNames[index]
                            font.pixelSize: 12
                            font.family: Services.Theme.fonts.fontFamily
                            font.weight: Font.Medium
                            color: Services.Theme.colors.textMuted
                        }
                    }
                }
            }

            // ====== Day Grid ======

            GridLayout {
                id: dayGrid
                Layout.fillWidth: true
                Layout.fillHeight: true
                columns: 7
                rowSpacing: 4
                columnSpacing: 4

                Repeater {
                    model: _getDaysInMonthView()

                    delegate: Rectangle {
                        id: dayCell
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        readonly property var dayData: modelData
                        readonly property bool isCurrentMonth: dayData.month === root.currentMonth
                        readonly property bool isToday: _isToday(dayData)
                        readonly property bool isSelected: _isSelected(dayData)
                        readonly property bool hasEvent: _hasEvent(dayData)

                        property bool hovered: mouseArea.containsMouse

                        radius: Services.Theme.radius.small
                        color: {
                            if (isToday) return Services.Theme.colors.primary
                            if (isSelected) return Services.Theme.colors.glassActive
                            if (hovered && isCurrentMonth) return Services.Theme.colors.glassHover
                            return "transparent"
                        }

                        border.width: isSelected ? 1 : 0
                        border.color: Services.Theme.colors.primary

                        // Day number
                        Text {
                            anchors.centerIn: parent
                            text: dayData.day
                            font.pixelSize: 14
                            font.family: Services.Theme.fonts.fontFamily
                            font.weight: isToday ? Font.Bold : Font.Normal
                            color: {
                                if (isToday) return Services.Theme.colors.background
                                if (!isCurrentMonth) return Services.Theme.colors.textMuted
                                return Services.Theme.colors.text
                            }
                        }

                        // Event indicator dot
                        Rectangle {
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 4
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: 4
                            height: 4
                            radius: 2
                            visible: hasEvent
                            color: isToday ? Services.Theme.colors.background : Services.Theme.colors.primary
                        }

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: isCurrentMonth ? Qt.PointingHandCursor : Qt.ArrowCursor
                            onClicked: {
                                if (isCurrentMonth) {
                                    var date = new Date(dayData.year, dayData.month, dayData.day)
                                    root.selectedDate = date
                                    root.dateSelected(date)
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // ====== Helper Functions ======

    function _getDaysInMonthView() {
        var days = []

        // Get first day of current month
        var firstDay = new Date(root.currentYear, root.currentMonth, 1)
        var firstDayIndex = firstDay.getDay() - root.firstDayOfWeek
        if (firstDayIndex < 0) firstDayIndex += 7

        // Days in current month
        var daysInCurrentMonth = new Date(root.currentYear, root.currentMonth + 1, 0).getDate()

        // Days in previous month
        var daysInPrevMonth = new Date(root.currentYear, root.currentMonth, 0).getDate()

        // Previous month days
        for (var i = firstDayIndex - 1; i >= 0; i--) {
            var day = daysInPrevMonth - i
            var month = root.currentMonth - 1
            var year = root.currentYear
            if (month < 0) {
                month = 11
                year--
            }
            days.push({ day: day, month: month, year: year })
        }

        // Current month days
        for (var d = 1; d <= daysInCurrentMonth; d++) {
            days.push({ day: d, month: root.currentMonth, year: root.currentYear })
        }

        // Next month days (fill to 6 rows = 42 cells)
        var remaining = 42 - days.length
        for (var n = 1; n <= remaining; n++) {
            var nextMonth = root.currentMonth + 1
            var nextYear = root.currentYear
            if (nextMonth > 11) {
                nextMonth = 0
                nextYear++
            }
            days.push({ day: n, month: nextMonth, year: nextYear })
        }

        return days
    }

    function _isToday(dayData) {
        return dayData.year === root._today.getFullYear() &&
               dayData.month === root._today.getMonth() &&
               dayData.day === root._today.getDate()
    }

    function _isSelected(dayData) {
        return dayData.year === root.selectedDate.getFullYear() &&
               dayData.month === root.selectedDate.getMonth() &&
               dayData.day === root.selectedDate.getDate()
    }

    function _hasEvent(dayData) {
        for (var i = 0; i < root.eventDates.length; i++) {
            var eventDate = root.eventDates[i]
            if (eventDate.getFullYear() === dayData.year &&
                eventDate.getMonth() === dayData.month &&
                eventDate.getDate() === dayData.day) {
                return true
            }
        }
        return false
    }

    function _previousMonth() {
        if (root.currentMonth === 0) {
            root.currentMonth = 11
            root.currentYear--
        } else {
            root.currentMonth--
        }
        root.monthChanged(root.currentYear, root.currentMonth)
    }

    function _nextMonth() {
        if (root.currentMonth === 11) {
            root.currentMonth = 0
            root.currentYear++
        } else {
            root.currentMonth++
        }
        root.monthChanged(root.currentYear, root.currentMonth)
    }

    function _previousYear() {
        root.currentYear--
        root.monthChanged(root.currentYear, root.currentMonth)
    }

    function _nextYear() {
        root.currentYear++
        root.monthChanged(root.currentYear, root.currentMonth)
    }

    function _goToToday() {
        root.currentYear = root._today.getFullYear()
        root.currentMonth = root._today.getMonth()
        root.selectedDate = new Date(root._today)
        root.monthChanged(root.currentYear, root.currentMonth)
    }
}
