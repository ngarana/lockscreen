// Clock.qml - Time and Date Display Component
//
// Displays a large clock with the current time and date below it.
// Uses Quickshell's SystemClock for efficient time updates.
//
// Visual Structure:
//   +------------------+
//   |      14:32       |  <- Large time display
//   | Wednesday, Apr 9 |  <- Smaller date display
//   +------------------+
//
// Features:
// - Auto-updating time display via SystemClock
// - Configurable precision (Seconds or Minutes)
// - Customizable time and date formats
// - Theme-aware styling
//
// Usage:
//   Clock {
//       precision: SystemClock.Seconds
//       timeFormat: "HH:mm"
//       dateFormat: "dddd, MMMM d"
//   }
//
// Properties:
//   precision - Update frequency (SystemClock.Seconds or SystemClock.Minutes)
//   timeFormat - Qt datetime format string for time
//   dateFormat - Qt datetime format string for date

import QtQuick
import Quickshell
import "../services"

Item {
    id: root

    // ========================================================================
    // Layout
    // ========================================================================

    // Calculate implicit size based on text content
    implicitWidth: clockText.implicitWidth
    implicitHeight: clockText.implicitHeight + dateText.implicitHeight + Theme.spacing.medium

    // ========================================================================
    // Public Properties
    // ========================================================================

    // Clock update precision
    // SystemClock.Seconds - Update every second (default, shows seconds)
    // SystemClock.Minutes - Update every minute (saves power)
    property int precision: SystemClock.Seconds

    // Time format string (Qt datetime format)
    // "HH:mm" - 24-hour format (default)
    // "h:mm AP" - 12-hour format with AM/PM
    // "HH:mm:ss" - Include seconds
    property string timeFormat: "HH:mm"

    // Date format string (Qt datetime format)
    // "dddd, MMMM d" - Full day name, full month (default)
    // "MM/dd/yyyy" - Numeric format
    // "ddd MMM d" - Short day name
    property string dateFormat: "dddd, MMMM d"

    // ========================================================================
    // Time Source
    // ========================================================================

    SystemClock {
        id: systemClock
        // Use the precision property to control update frequency
        // This affects both battery usage and displayed precision
        precision: root.precision
    }

    // ========================================================================
    // Visual Layout
    // ========================================================================

    Column {
        id: column
        anchors.centerIn: parent
        spacing: Theme.spacing.medium

        // Time display - Large, prominent
        Text {
            id: clockText
            anchors.horizontalCenter: parent.horizontalCenter
            text: Qt.formatDateTime(systemClock.date, root.timeFormat)
            font.pixelSize: Theme.fonts.textSizeClock  // 72px by default
            font.weight: Font.Light  // Thin weight for elegance
            color: Theme.colors.text  // High contrast
        }

        // Date display - Smaller, subtle
        Text {
            id: dateText
            anchors.horizontalCenter: parent.horizontalCenter
            text: Qt.formatDateTime(systemClock.date, root.dateFormat)
            font.pixelSize: Theme.fonts.textSizeDate  // 24px by default
            font.weight: Font.Light
            color: Theme.colors.textSubtle  // Lower contrast
        }
    }
}
