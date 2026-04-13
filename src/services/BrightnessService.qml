// BrightnessService.qml - Display Brightness Control Service
//
// Singleton for controlling and monitoring display brightness across
// multiple monitors. Supports backlight control via brightnessctl or
// xrandr, with automatic detection of the best available method.
//
// Responsibilities:
// - Monitor display brightness levels
// - Control brightness for each output
// - Persistence of brightness preferences
// - Signal-based updates for UI components
//
// Usage:
// import "../services"
//
// Connections {
//     target: BrightnessService
//     function onBrightnessChanged(screen, value) { updateSlider(value) }
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

    // Step size for brightness adjustments (percentage points)
    property int stepSize: 5

    // Minimum brightness level (to prevent black screen)
    property int minBrightness: 5

    // ========================================================================
    // State Properties (Read-Only)
    // ========================================================================

    // Whether brightness control is available
    property bool isAvailable: false

    // Current brightness method: "brightnessctl", "xrandr", "ddcutil", "none"
    property string controlMethod: "none"

// Primary screen brightness (0-100)
property int primaryBrightness: 100

    // Whether currently adjusting brightness
    property bool isAdjusting: false

    // List of controllable displays
    property var displays: []

    // Number of displays
    property int displayCount: 0

    // ========================================================================
    // Signals
    // ========================================================================

// Emitted when brightness changes for any display
signal brightnessChanged(string display, int value)

// Emitted when primary brightness changes (auto-generated from property)
// signal primaryBrightnessChanged(int value) - implicit

// Emitted when a specific display's brightness changes
signal displayBrightnessChanged(string display, int value)

    // Emitted when brightness control becomes available/unavailable
    signal availabilityChanged(bool available)

    // ========================================================================
    // Private Properties
    // ========================================================================

    property var _displays: ({}) // Internal display state

    // Process objects for various operations
    property var _checkBrightnessctlProcess: Process {
        id: checkBrightnessctlProcess
        command: ["which", "brightnessctl"]
        stdout: StdioCollector {}

        onExited: function(code, status) {
            root._onBrightnessctlCheck(code)
        }
    }

    property var _checkXrandrProcess: Process {
        id: checkXrandrProcess
        command: ["which", "xrandr"]
        stdout: StdioCollector {}

        onExited: function(code, status) {
            root._onXrandrCheck(code)
        }
    }

    property var _enumerateBacklightProcess: Process {
        id: enumerateBacklightProcess
        command: ["brightnessctl", "-l", "-m"]
        stdout: StdioCollector {}

        onExited: function(code, status) {
            if (code === 0 && this.stdout.text) {
                root._parseBrightnessctlList(this.stdout.text)
            }
        }
    }

    property var _enumerateXrandrProcess: Process {
        id: enumerateXrandrProcess
        command: ["xrandr", "--listmonitors"]
        stdout: StdioCollector {}

        onExited: function(code, status) {
            if (code === 0 && this.stdout.text) {
                root._parseXrandrList(this.stdout.text)
            }
        }
    }

    property var _setBrightnessctlProcess: Process {
        id: setBrightnessctlProcess
        stdout: StdioCollector {}

        onExited: function(code, status) {
            root.isAdjusting = false
            if (code === 0) {
                // Extract device and percentage from command for callback
                const cmd = this.command
                if (cmd.length >= 6) {
                    const device = cmd[2]
                    const percentageStr = cmd[4].replace('%', '')
                    const percentage = parseInt(percentageStr)
                    root._updateBrightnessAfterSet(device, percentage)
                }
            }
        }
    }

    property var _setXrandrProcess: Process {
        id: setXrandrProcess
        stdout: StdioCollector {}

        onExited: function(code, status) {
            root.isAdjusting = false
            if (code === 0) {
                // Extract display and percentage from command for callback
                const cmd = this.command
                if (cmd.length >= 6) {
                    const display = cmd[2]
                    const brightnessValue = parseFloat(cmd[4])
                    const percentage = Math.round(brightnessValue * 100)
                    root._updateBrightnessAfterSet(display, percentage)
                }
            }
        }
    }

    // ========================================================================
    // Public Methods
    // ========================================================================

    // Initialize and detect available control methods
    function initialize() {
        Core.Logger.info("Initializing BrightnessService", "BrightnessService")

        // Try brightnessctl first (most reliable for backlight)
        _checkBrightnessctl()
    }

    // Check if brightnessctl is available
    function _checkBrightnessctl() {
        _checkBrightnessctlProcess.running = false
        _checkBrightnessctlProcess.running = true
    }

    function _onBrightnessctlCheck(exitCode) {
        if (exitCode === 0) {
            controlMethod = "brightnessctl"
            isAvailable = true
            availabilityChanged(true)
            _enumerateBacklightDevices()
            Core.Logger.info("Using brightnessctl for brightness control", "BrightnessService")
        } else {
            // Fall back to xrandr
            _checkXrandr()
        }
    }

    // Check if xrandr is available
    function _checkXrandr() {
        _checkXrandrProcess.running = false
        _checkXrandrProcess.running = true
    }

    function _onXrandrCheck(exitCode) {
        if (exitCode === 0) {
            controlMethod = "xrandr"
            isAvailable = true
            availabilityChanged(true)
            _enumerateXrandrDisplays()
            Core.Logger.info("Using xrandr for brightness control", "BrightnessService")
        } else {
            controlMethod = "none"
            isAvailable = false
            availabilityChanged(false)
            Core.Logger.warning("No brightness control method available", "BrightnessService")
        }
    }

    // Enumerate backlight devices using brightnessctl
    function _enumerateBacklightDevices() {
        _enumerateBacklightProcess.running = false
        _enumerateBacklightProcess.running = true
    }

    function _parseBrightnessctlList(output) {
        if (!output) return;
        const lines = output.split("\n")
        const newDisplays = []
        root._displays = {}

        for (let i = 0; i < lines.length; i++) {
            const line = lines[i].trim()
            if (line.length === 0) continue

            // Parse: class, name, encoded brightness, current brightness, max brightness
            const parts = line.split(",")
            if (parts.length >= 5) {
                const deviceClass = parts[0]
                const name = parts[1]
                const current = parseInt(parts[3]) || 0
                const max = parseInt(parts[4]) || 1

                if (deviceClass === "backlight" || deviceClass === "leds") {
                    const display = {
                        name: name,
                        type: deviceClass,
                        current: current,
                        max: max,
                        percentage: Math.round((current / max) * 100)
                    }

                    newDisplays.push(display)
                    root._displays[name] = display
                }
            }
        }

        displays = newDisplays
        displayCount = newDisplays.length

// Update primary brightness (use first display)
if (newDisplays.length > 0) {
    primaryBrightness = newDisplays[0].percentage
}
    }

    // Enumerate displays using xrandr
    function _enumerateXrandrDisplays() {
        _enumerateXrandrProcess.running = false
        _enumerateXrandrProcess.running = true
    }

    function _parseXrandrList(output) {
        if (!output) return;
        const lines = output.split("\n")
        const newDisplays = []
        root._displays = {}

        for (let i = 0; i < lines.length; i++) {
            const line = lines[i].trim()
            if (line.length === 0 || line.startsWith("0:")) continue

            // Parse monitor line: index: +*DisplayName resolution ...
            const match = line.match(/^\s*\d+:\s+[+\*]?\s*(\S+)/)
            if (match) {
                const name = match[1]
                const display = {
                    name: name,
                    type: "xrandr",
                    current: 100,
                    max: 100,
                    percentage: 100
                }

                newDisplays.push(display)
                root._displays[name] = display
            }
        }

displays = newDisplays
displayCount = newDisplays.length
primaryBrightness = 100
    }

    // Get brightness for a specific display
    function getBrightness(displayName) {
        if (root._displays[displayName]) {
            return root._displays[displayName].percentage
        }
        return 100
    }

    // Set brightness for a specific display
    function setBrightness(displayName, percentage) {
        if (!isAvailable) return

        percentage = Math.max(minBrightness, Math.min(100, percentage))

        if (controlMethod === "brightnessctl") {
            _setBrightnessctl(displayName, percentage)
        } else if (controlMethod === "xrandr") {
            _setXrandrBrightness(displayName, percentage)
        }
    }

    function _setBrightnessctl(device, percentage) {
        isAdjusting = true
        _setBrightnessctlProcess.running = false
        _setBrightnessctlProcess.command = ["brightnessctl", "-d", device, "set", percentage + "%"]
        _setBrightnessctlProcess.running = true
    }

    function _setXrandrBrightness(display, percentage) {
        isAdjusting = true
        const brightnessValue = percentage / 100
        _setXrandrProcess.running = false
        _setXrandrProcess.command = ["xrandr", "--output", display, "--brightness", brightnessValue.toString()]
        _setXrandrProcess.running = true
    }

    function _updateBrightnessAfterSet(displayName, percentage) {
        if (root._displays[displayName]) {
            root._displays[displayName].percentage = percentage
            displayBrightnessChanged(displayName, percentage)
            brightnessChanged(displayName, percentage)

// Update primary brightness if this is the first display
if (displays.length > 0 && displays[0].name === displayName) {
    primaryBrightness = percentage
}
        }
    }

    // Set brightness for all displays
    function setAllBrightness(percentage) {
        for (let i = 0; i < displays.length; i++) {
            setBrightness(displays[i].name, percentage)
        }
    }

// Adjust brightness by delta
function adjustBrightness(delta) {
    const newBrightness = primaryBrightness + delta
    setAllBrightness(newBrightness)
}

    // Increase brightness by step
    function increase() {
        adjustBrightness(stepSize)
    }

    // Decrease brightness by step
    function decrease() {
        adjustBrightness(-stepSize)
    }

// Get icon name based on current brightness
function getIconName() {
    if (!isAvailable) {
        return "display-brightness-off"
    }

    if (primaryBrightness >= 80) return "display-brightness-high"
    if (primaryBrightness >= 50) return "display-brightness-medium"
    if (primaryBrightness >= 20) return "display-brightness-low"
    return "display-brightness-off"
}

    // Refresh brightness values
    function refresh() {
        if (controlMethod === "brightnessctl") {
            _enumerateBacklightDevices()
        } else if (controlMethod === "xrandr") {
            // xrandr doesn't provide a way to read current brightness
            // so we just update the displays list
            _enumerateXrandrDisplays()
        }
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        initialize()
    }
}
