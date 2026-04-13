// BatteryService.qml - Battery and Power Monitoring Service
//
// Singleton for monitoring battery status using upower for maximum reliability.
//
// Usage:
// import "../services"
//
// Connections {
//     target: BatteryService
//     function onBatteryLevelChanged() { updateBatteryIcon() }
// }

pragma Singleton
import Quickshell
import Quickshell.Io
import QtQuick
import "../core" as Core

QtObject {
    id: root

    // ========================================================================
    // Configuration
    // ========================================================================

    property int updateInterval: 30000

    // ========================================================================
    // State Properties (Read-Only)
    // ========================================================================

    property bool hasBattery: false
    property int percentage: 0
    property bool isCharging: false
    property bool isFullyCharged: false
    property string status: "unknown"
    property string timeRemainingFormatted: ""
    property string levelCategory: "unknown"
    property bool acConnected: false

    // ========================================================================
    // Signals
    // ========================================================================

    signal batteryLevelChanged(int percentage)
    signal chargingStatusChanged(bool isCharging)
    signal batteryChanged()

    // ========================================================================
    // Private Properties
    // ========================================================================

    property var _updateTimer: Timer {
        interval: root.updateInterval
        running: true
        repeat: true
        onTriggered: root.refresh()
    }

    // ========================================================================
    // Public Methods
    // ========================================================================

    function refresh() {
        _updateBattery()
    }

    // ========================================================================
    // Private Methods
    // ========================================================================

    function _updateBattery() {
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["upower", "-i", "/org/freedesktop/UPower/devices/DisplayDevice"]; running: true; onExited: function(c, s) { if (c === 0) root._parseUPower(stdout) } }',
            root
        )
    }

    function _parseUPower(output) {
        if (!output) {
            _fallbackToSysfs()
            return
        }

        const lines = output.split("\n")
        let foundBattery = false
        let newPercentage = 0
        let newStatus = "unknown"
        let newIsCharging = false
        let newIsFull = false

        for (let i = 0; i < lines.length; i++) {
            const line = lines[i].trim()
            if (line.startsWith("percentage:")) {
                newPercentage = parseInt(line.split(":")[1].replace("%", "").trim())
                foundBattery = true
            } else if (line.startsWith("state:")) {
                newStatus = line.split(":")[1].trim()
                newIsCharging = (newStatus === "charging" || newStatus === "pending-charge")
                newIsFull = (newStatus === "fully-charged" || newStatus === "full")
            } else if (line.startsWith("time to empty:")) {
                root.timeRemainingFormatted = line.split(":")[1].trim()
            } else if (line.startsWith("time to full:")) {
                root.timeRemainingFormatted = line.split(":")[1].trim()
            } else if (line.startsWith("present:")) {
                foundBattery = (line.split(":")[1].trim() === "yes")
            }
        }

        if (!foundBattery) {
            _fallbackToSysfs()
            return
        }

        const oldPercentage = root.percentage
        const oldCharging = root.isCharging

        root.hasBattery = true
        root.percentage = newPercentage
        root.status = newStatus
        root.isCharging = newIsCharging
        root.isFullyCharged = newIsFull

        // Determine level category
        if (root.percentage >= 95) root.levelCategory = "full"
        else if (root.percentage >= 50) root.levelCategory = "high"
        else if (root.percentage >= 30) root.levelCategory = "medium"
        else if (root.percentage >= 15) root.levelCategory = "low"
        else root.levelCategory = "critical"

        if (oldPercentage !== root.percentage) {
            batteryLevelChanged(root.percentage)
            batteryChanged()
        }
        if (oldCharging !== root.isCharging) {
            chargingStatusChanged(root.isCharging)
            batteryChanged()
        }

        _checkAcPower()
    }

    function _checkAcPower() {
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["sh", "-c", "cat /sys/class/power_supply/AC*/online || cat /sys/class/power_supply/ADP*/online"]; running: true; onExited: function(c, s) { if (c === 0 && stdout) root.acConnected = (stdout.trim() === "1") } }',
            root
        )
    }

    function _fallbackToSysfs() {
        // Implementation for systems without upower or DisplayDevice
        // Simplified for now, just checking BAT0
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["cat", "/sys/class/power_supply/BAT0/capacity"]; running: true; onExited: function(c, s) { if (c === 0 && stdout) root.percentage = parseInt(stdout.trim()) } }',
            root
        )
    }

    Component.onCompleted: {
        refresh()
    }
}
