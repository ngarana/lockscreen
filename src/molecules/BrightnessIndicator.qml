// BrightnessIndicator.qml - Brightness Icon + Percentage Molecule
//
// Displays brightness level with icon and percentage.
// Supports wheel scrolling for brightness adjustment.
//
// Usage:
//   BrightnessIndicator {
//       onClicked: Services.BarController.toggleModule("quicksettings")
//   }
//
// Properties:
//   - showPercentage: bool - Show percentage text (default: true)
//   - showIcon: bool - Show brightness icon (default: true)
//
// Signals:
//   - clicked(): Emitted when indicator is clicked

import QtQuick
import QtQuick.Layouts
import Quickshell.Io
import "../atoms" as Atoms
import "../theme" as Theme

Item {
    id: root

    // Public API
    property bool showPercentage: true
    property bool showIcon: true

    signal clicked()

    // Internal state
    property int _percentage: 100
    readonly property bool _isAvailable: true

    // Layout
    implicitWidth: rowLayout.implicitWidth
    implicitHeight: rowLayout.implicitHeight

    RowLayout {
        id: rowLayout
        anchors.fill: parent
        spacing: 4
        Layout.alignment: Qt.AlignVCenter

        // Brightness icon
        Text {
            id: brightnessIcon
            visible: root.showIcon
            text: root._getBrightnessIcon()
            font.pixelSize: 16
            color: root._isAvailable ? Theme.ThemeEngine.colors.textPrimary : Theme.ThemeEngine.colors.textMuted
            Layout.alignment: Qt.AlignVCenter
        }

        // Percentage label
        Atoms.Label {
            id: percentageLabel
            visible: root.showPercentage
            text: root._percentage + "%"
            fontSize: Theme.ThemeEngine.typography.sizeSm
            color: root._isAvailable ? Theme.ThemeEngine.colors.textPrimary : Theme.ThemeEngine.colors.textMuted
            Layout.alignment: Qt.AlignVCenter
            Layout.topMargin: -1 // Subtle adjustment for vertical center
            verticalAlignment: Text.AlignVCenter
        }
    }

    // Click and wheel area
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
        onWheel: function(wheel) {
            if (wheel.angleDelta.y > 0) {
                const newValue = Math.min(100, root._percentage + 5)
                root._setBrightness(newValue)
            } else if (wheel.angleDelta.y < 0) {
                const newValue = Math.max(5, root._percentage - 5)
                root._setBrightness(newValue)
            }
            wheel.accepted = true
        }
    }

    // Brightness control process
    Process {
        id: brightnessProcess
        stdout: StdioCollector {}

        onExited: function(code, status) {
            if (code === 0) {
                // Successfully set brightness
                root._updateBrightnessFromSystem()
            }
        }
    }

    // Process to get current brightness info
    Process {
        id: getBrightnessProcess
        command: ["brightnessctl", "-d", "intel_backlight", "-m"]
        stdout: StdioCollector {}

        onExited: function(code, status) {
            if (code === 0 && this.stdout.text.trim()) {
                // Format: intel_backlight,backlight,15360,50%,30720
                const parts = this.stdout.text.trim().split(',')
                if (parts.length >= 5) {
                    const current = parseInt(parts[2])
                    const max = parseInt(parts[4])
                    if (max > 0) {
                        root._percentage = Math.round((current / max) * 100)
                    }
                }
            }
        }
    }

    // Initialize brightness on component completion
    Component.onCompleted: {
        root._updateBrightnessFromSystem()
    }

    // Update brightness from system
    function _updateBrightnessFromSystem() {
        getBrightnessProcess.running = false
        getBrightnessProcess.running = true
    }

    // Set brightness
    function _setBrightness(percentage) {
        brightnessProcess.running = false
        brightnessProcess.command = ["brightnessctl", "-d", "intel_backlight", "set", percentage + "%"]
        brightnessProcess.running = true
        root._percentage = percentage
    }

    // Get appropriate brightness icon
    function _getBrightnessIcon() {
        return "☀" // Simple sun symbol
    }
}