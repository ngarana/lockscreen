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

    property var _batteryProcess: Process {
        id: batteryProcess
        command: ["upower", "-i", "/org/freedesktop/UPower/devices/DisplayDevice"]
        stdout: StdioCollector {}

        onExited: function(code, status) {
            if (code === 0 && this.stdout.text) {
                root._parseUPower(this.stdout.text)
            } else {
                root._fallbackToSysfs()
            }
        }
    }

    property var _acPowerProcess: Process {
        id: acPowerProcess
        command: ["sh", "-c", "cat /sys/class/power_supply/AC*/online || cat /sys/class/power_supply/ADP*/online"]
        stdout: StdioCollector {}

        onExited: function(code, status) {
            if (code === 0 && this.stdout.text) {
                root.acConnected = (this.stdout.text.trim() === "1")
            }
        }
    }

    property var _fallbackProcess: Process {
        id: fallbackProcess
        command: ["cat", "/sys/class/power_supply/BAT0/capacity"]
        stdout: StdioCollector {}

        onExited: function(code, status) {
            if (code === 0 && this.stdout.text) {
                const capacity = parseInt(this.stdout.text.trim())
                if (!isNaN(capacity)) {
                    root.percentage = capacity
                    root.hasBattery = true
                    root._updateLevelCategory()
                    root.batteryLevelChanged(root.percentage)
                    root.batteryChanged()
                }
            }
        }
    }

    // ========================================================================
    // Public Methods
    // ========================================================================

    function refresh() {
        _batteryProcess.running = false
        _batteryProcess.running = true
    }

    // ========================================================================
    // Private Methods
    // ========================================================================

    function _updateBattery() {
        _batteryProcess.running = false
        _batteryProcess.running = true
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
            if (line.includes("percentage:")) {
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

        root._updateLevelCategory()

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
        _acPowerProcess.running = false
        _acPowerProcess.running = true
    }

    function _fallbackToSysfs() {
        _fallbackProcess.running = false
        _fallbackProcess.running = true
    }

    function _updateLevelCategory() {
        if (root.percentage >= 95) root.levelCategory = "full"
        else if (root.percentage >= 50) root.levelCategory = "high"
        else if (root.percentage >= 30) root.levelCategory = "medium"
        else if (root.percentage >= 15) root.levelCategory = "low"
        else root.levelCategory = "critical"
    }

    Component.onCompleted: {
        refresh()
    }
}
