// BatteryService.qml - Battery and Power Monitoring Service
//
// Singleton for monitoring battery status, charge level, and power state.
// Provides signal-based updates for UI components like the battery indicator.
//
// Responsibilities:
// - Battery percentage monitoring
// - Charging status detection
// - Time remaining estimation
// - Multiple battery support
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

    // Update interval in milliseconds
    property int updateInterval: 30000

    // ========================================================================
    // State Properties (Read-Only)
    // ========================================================================

    // Whether a battery is present
    property bool hasBattery: false

    // Battery charge percentage (0-100)
    property int percentage: 0

    // Whether battery is currently charging
    property bool isCharging: false

    // Whether battery is fully charged
    property bool isFullyCharged: false

    // Battery status: "discharging", "charging", "full", "unknown"
    property string status: "unknown"

    // Estimated time remaining in minutes (-1 if unknown)
    property int timeRemaining: -1

    // Time remaining formatted string
    property string timeRemainingFormatted: ""

    // Battery health percentage (0-100, -1 if unknown)
    property int health: -1

    // Battery technology (Li-ion, Li-poly, etc.)
    property string technology: ""

    // Battery level category: "critical", "low", "medium", "high", "full"
    property string levelCategory: "unknown"

    // Multiple batteries support
    property var batteries: []

    // Combined battery percentage (for multiple batteries)
    property int combinedPercentage: 0

    // Whether AC power is connected
    property bool acConnected: false

    // ========================================================================
    // Signals
    // ========================================================================

    // Emitted when battery percentage changes
    signal batteryLevelChanged(int percentage)

    // Emitted when charging status changes
    signal chargingStatusChanged(bool isCharging)

    // Emitted when battery is low
    signal batteryLow(int percentage)

    // Emitted when battery is critical
    signal batteryCritical(int percentage)

    // Emitted when any battery property changes
    signal batteryChanged()

    // ========================================================================
    // Private Properties
    // ========================================================================

    property var _updateTimer: Timer {
        interval: root.updateInterval
        running: true
        repeat: true
        onTriggered: root._updateBattery()
    }

    property var _batteryPath: ""
    property var _batteryPaths: []

    // Component for creating Process objects dynamically
    property var processComponent: Component {
        Process {
            property string batteryPath: ""
            property int batteryIndex: -1
            onExited: function(code, status) {
                if (code === 0) root._parseBatteryInfo(stdout, batteryPath, batteryIndex)
            }
        }
    }

    // ========================================================================
    // Public Methods
    // ========================================================================

    // Get icon name based on current state
    function getIconName() {
        if (!hasBattery) {
            return "battery-missing"
        }

        if (isCharging) {
            if (percentage >= 90) return "battery-full-charging"
            if (percentage >= 70) return "battery-good-charging"
            if (percentage >= 50) return "battery-medium-charging"
            if (percentage >= 30) return "battery-low-charging"
            return "battery-caution-charging"
        }

        if (isFullyCharged) {
            return "battery-full"
        }

        if (percentage >= 90) return "battery-full"
        if (percentage >= 70) return "battery-good"
        if (percentage >= 50) return "battery-medium"
        if (percentage >= 30) return "battery-low"
        if (percentage >= 15) return "battery-caution"
        return "battery-empty"
    }

    // Get human-readable status string
    function getStatusText() {
        if (!hasBattery) {
            return "No battery"
        }

        if (isCharging) {
            return "Charging" + (timeRemainingFormatted ? " - " + timeRemainingFormatted + " until full" : "")
        }

        if (isFullyCharged) {
            return "Fully charged"
        }

        return percentage + "%" + (timeRemainingFormatted ? " - " + timeRemainingFormatted + " remaining" : "")
    }

    // Check if battery is low
    function isLow() {
        return percentage <= 20 && !isCharging
    }

    // Check if battery is critical
    function isCritical() {
        return percentage <= 10 && !isCharging
    }

    // Force refresh battery status
    function refresh() {
        _updateBattery()
    }

    // ========================================================================
    // Private Methods
    // ========================================================================

    function _updateBattery() {
        // Find battery paths first
        _findBatteryPaths()
    }

    function _findBatteryPaths() {
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["ls", "/sys/class/power_supply/"]; running: true; onExited: function(c, s) { if (c === 0) root._parseBatteryPaths(stdout) } }',
            root
        )
    }

    function _parseBatteryPaths(output) {
        if (!output) {
            root.hasBattery = false
            return
        }
        const paths = output.split("\n").filter(function(p) { return p.trim().length > 0 && p.startsWith("BAT") })
        _batteryPaths = paths.map(function(p) { return "/sys/class/power_supply/" + p })

        root.hasBattery = _batteryPaths.length > 0
        root.batteries = []

        if (!root.hasBattery) {
            root.percentage = 0
            root.status = "unknown"
            root.isCharging = false
            root.levelCategory = "unknown"
            return
        }

        // Read each battery
        let totalPercentage = 0
        let batteryCount = 0

        for (let i = 0; i < _batteryPaths.length; i++) {
            _readBatteryInfo(_batteryPaths[i], i)
        }
    }

    function _readBatteryInfo(path, index) {
        const proc = processComponent.createObject(root, {
            command: ["cat", path + "/uevent"],
            batteryPath: path,
            batteryIndex: index
        })
        proc.running = true
    }

    function _parseBatteryInfo(output, path, index) {
        const lines = output.split("\n")
        let battery = {
            path: path,
            name: path.split("/").pop(),
            percentage: 0,
            status: "unknown",
            isCharging: false,
            isFull: false,
            energyFull: -1,
            energyNow: -1,
            powerNow: -1,
            voltage: -1,
            technology: ""
        }

        for (let i = 0; i < lines.length; i++) {
            const line = lines[i].trim()
            if (line.length === 0) continue

            const parts = line.split("=")
            if (parts.length !== 2) continue

            const key = parts[0]
            const value = parts[1]

            switch (key) {
                case "POWER_SUPPLY_STATUS":
                    battery.status = value.toLowerCase()
                    battery.isCharging = battery.status === "charging"
                    battery.isFull = battery.status === "full"
                    break
                case "POWER_SUPPLY_CAPACITY":
                    battery.percentage = parseInt(value) || 0
                    break
                case "POWER_SUPPLY_ENERGY_FULL":
                    battery.energyFull = parseInt(value) || 0
                    break
                case "POWER_SUPPLY_ENERGY_NOW":
                    battery.energyNow = parseInt(value) || 0
                    break
                case "POWER_SUPPLY_POWER_NOW":
                    battery.powerNow = parseInt(value) || 0
                    break
                case "POWER_SUPPLY_VOLTAGE_NOW":
                    battery.voltage = parseInt(value) || 0
                    break
                case "POWER_SUPPLY_TECHNOLOGY":
                    battery.technology = value
                    break
            }
        }

        // Calculate percentage if not provided
        if (battery.percentage === 0 && battery.energyFull > 0 && battery.energyNow > 0) {
            battery.percentage = Math.round((battery.energyNow / battery.energyFull) * 100)
        }

        // Clamp percentage
        battery.percentage = Math.max(0, Math.min(100, battery.percentage))

        // Calculate time remaining
        if (battery.powerNow > 0 && battery.energyNow > 0) {
            if (battery.isCharging) {
                const remainingEnergy = battery.energyFull - battery.energyNow
                battery.timeRemaining = Math.round((remainingEnergy / battery.powerNow) * 60)
            } else {
                battery.timeRemaining = Math.round((battery.energyNow / battery.powerNow) * 60)
            }
        }

        // Add to batteries array
        let newBatteries = root.batteries.slice()
        newBatteries[index] = battery
        root.batteries = newBatteries

        // Update main battery (use first battery or combined)
        _updateMainBattery()
    }

    function _updateMainBattery() {
        if (root.batteries.length === 0) return

        let totalPercentage = 0
        let anyCharging = false
        let allFull = true
        let totalTimeRemaining = 0
        let hasTimeEstimate = false

        for (let i = 0; i < root.batteries.length; i++) {
            const battery = root.batteries[i]
            totalPercentage += battery.percentage
            if (battery.isCharging) anyCharging = true
            if (!battery.isFull) allFull = false
            if (battery.timeRemaining > 0) {
                totalTimeRemaining += battery.timeRemaining
                hasTimeEstimate = true
            }
        }

        const oldPercentage = root.percentage
        const oldCharging = root.isCharging

        root.combinedPercentage = Math.round(totalPercentage / root.batteries.length)
        root.percentage = root.combinedPercentage
        root.isCharging = anyCharging
        root.isFullyCharged = allFull
        root.status = anyCharging ? "charging" : (allFull ? "full" : "discharging")
        root.technology = root.batteries[0]?.technology || ""

        // Determine level category
        if (root.percentage >= 95) {
            root.levelCategory = "full"
        } else if (root.percentage >= 50) {
            root.levelCategory = "high"
        } else if (root.percentage >= 30) {
            root.levelCategory = "medium"
        } else if (root.percentage >= 15) {
            root.levelCategory = "low"
        } else {
            root.levelCategory = "critical"
        }

        // Calculate time remaining
        if (hasTimeEstimate) {
            root.timeRemaining = Math.round(totalTimeRemaining / root.batteries.length)
            root.timeRemainingFormatted = _formatTime(root.timeRemaining)
        } else {
            root.timeRemaining = -1
            root.timeRemainingFormatted = ""
        }

        // Emit signals if changed
        if (oldPercentage !== root.percentage) {
            batteryLevelChanged(root.percentage)
            batteryChanged()

            // Check for low/critical levels
            if (isCritical()) {
                batteryCritical(root.percentage)
            } else if (isLow()) {
                batteryLow(root.percentage)
            }
        }

        if (oldCharging !== root.isCharging) {
            chargingStatusChanged(root.isCharging)
            batteryChanged()
        }

        // Check AC power status
        _checkAcPower()
    }

    function _checkAcPower() {
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["cat", "/sys/class/power_supply/AC/online"]; running: true; onExited: function(c) { if (c === 0) root.acConnected = (stdout.trim() === "1") } }',
            root
        )
    }

    function _formatTime(minutes) {
        if (minutes < 0) return ""

        const hours = Math.floor(minutes / 60)
        const mins = minutes % 60

        if (hours > 0) {
            return hours + "h " + mins + "m"
        }
        return mins + "m"
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        _updateBattery()
        Core.Logger.info("BatteryService initialized", "BatteryService")
    }
}
