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



    property var _enumerateBacklightProcess: Process {
        id: enumerateBacklightProcess
        command: ["brightnessctl", "-l", "-m"]
        stdout: StdioCollector {}

        onExited: function(code, status) {
            Core.Logger.info("enumerate brightnessctl exited with code: " + code, "BrightnessService")
            if (code === 0 && this.stdout.text) {
                Core.Logger.info("brightnessctl list output: " + this.stdout.text.trim(), "BrightnessService")
                root._parseBrightnessctlList(this.stdout.text)
            } else {
                Core.Logger.error("Failed to enumerate brightness devices, code: " + code, "BrightnessService")
            }
        }
    }



    property var _setBrightnessctlProcess: Process {
        id: setBrightnessctlProcess
        stdout: StdioCollector {}

        onExited: function(code, status) {
            root.isAdjusting = false
            Core.Logger.info("brightnessctl command exited with code: " + code, "BrightnessService")
            if (code === 0) {
                // Extract device and percentage from command for callback
                const cmd = this.command
                if (cmd.length >= 6) {
                    const device = cmd[2]
                    const percentageStr = cmd[4].replace('%', '')
                    const percentage = parseInt(percentageStr)
                    Core.Logger.info("Brightness set successfully: " + device + " = " + percentage + "%", "BrightnessService")
                    root._updateBrightnessAfterSet(device, percentage)
                }
            } else {
                Core.Logger.error("Failed to set brightness, exit code: " + code, "BrightnessService")
            }
        }
    }



    // ========================================================================
    // Public Methods
    // ========================================================================

    // Initialize and detect available control methods
    function initialize() {
        Core.Logger.info("Initializing BrightnessService", "BrightnessService")

        // Directly use brightnessctl (known to be available)
        controlMethod = "brightnessctl"
        isAvailable = true
        availabilityChanged(true)
        _enumerateBacklightDevices()
        Core.Logger.info("Using brightnessctl for brightness control", "BrightnessService")
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

            // Parse: name, class, current absolute, percentage, max brightness
            const parts = line.split(",")
            if (parts.length >= 5) {
                const name = parts[0]
                const deviceClass = parts[1]
                const current = parseInt(parts[2]) || 0
                const max = parseInt(parts[4]) || 1
                
                // Percentage can be extracted from parts[3] (e.g. "83%") or calculated
                const percentStr = parts[3].replace('%', '')
                const percentage = parseInt(percentStr) || Math.round((current / max) * 100)

                if (deviceClass === "backlight" || deviceClass === "leds") {
                    const display = {
                        name: name,
                        type: deviceClass,
                        current: current,
                        max: max,
                        percentage: Math.max(0, Math.min(100, percentage))
                    }

                    newDisplays.push(display)
                    root._displays[name] = display
                    Core.Logger.info("Found brightness device: " + name + " (" + deviceClass + ") at " + percentage + "%", "BrightnessService")
                }
            }
        }

        displays = newDisplays
        displayCount = newDisplays.length

// Update primary brightness (use first backlight device)
if (newDisplays.length > 0) {
    for (let i = 0; i < newDisplays.length; i++) {
        if (newDisplays[i].type === "backlight") {
            primaryBrightness = newDisplays[i].percentage
            // Emit signal for OSD
            brightnessChanged(newDisplays[i].name, newDisplays[i].percentage)
            break
        }
    }
}
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

        _setBrightnessctl(displayName, percentage)
    }

    function _setBrightnessctl(device, percentage) {
        isAdjusting = true
        _setBrightnessctlProcess.running = false
        _setBrightnessctlProcess.command = ["brightnessctl", "-d", device, "set", percentage + "%"]
        Core.Logger.info("Setting brightness: " + device + " to " + percentage + "%", "BrightnessService")
        Core.Logger.info("Command: " + _setBrightnessctlProcess.command.join(" "), "BrightnessService")
        _setBrightnessctlProcess.running = true
    }



    function _updateBrightnessAfterSet(displayName, percentage) {
        if (root._displays[displayName]) {
            root._displays[displayName].percentage = percentage
            displayBrightnessChanged(displayName, percentage)
            brightnessChanged(displayName, percentage)

// Update primary brightness if this is a backlight device
if (displays.length > 0) {
    for (let i = 0; i < displays.length; i++) {
        if (displays[i].name === displayName && displays[i].type === "backlight") {
            primaryBrightness = percentage
            break
        }
    }
}
        }
    }

    // Set brightness for all displays
    function setAllBrightness(percentage) {
        if (!isAvailable || displays.length === 0) return
        
        for (let i = 0; i < displays.length; i++) {
            // Only set brightness for backlight devices, not LEDs
            if (displays[i].type === "backlight") {
                setBrightness(displays[i].name, percentage)
            }
        }
    }

// Adjust brightness by delta
function adjustBrightness(delta) {
    const nextVal = Math.max(minBrightness, Math.min(100, primaryBrightness + delta))
    if (nextVal !== primaryBrightness) {
        primaryBrightness = nextVal
        setAllBrightness(primaryBrightness)
    }
}

    // Increase brightness by step
    function increase() {
        if (!isAvailable || displays.length === 0) return
        _adjustBrightnessProcess.running = false
        _adjustBrightnessProcess.command = ["brightnessctl", "set", stepSize + "%+"]
        _adjustBrightnessProcess.running = true
    }

    // Decrease brightness by step
    function decrease() {
        if (!isAvailable || displays.length === 0) return
        _adjustBrightnessProcess.running = false
        _adjustBrightnessProcess.command = ["brightnessctl", "set", stepSize + "%-"]
        _adjustBrightnessProcess.running = true
    }

    property var _adjustBrightnessProcess: Process {
        id: adjustBrightnessProcess
        command: []
        onExited: function(code, status) {
            if (code === 0) {
                refresh()
                // Emit signal for OSD
                const firstBacklight = displays.find(d => d.type === "backlight")
                if (firstBacklight) {
                    brightnessChanged(firstBacklight.name, firstBacklight.percentage)
                }
            }
        }
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
        _enumerateBacklightDevices()
    }

    // Open display settings (xrandr GUI or system settings)
    function openDisplaySettings() {
        Core.Logger.info("Opening display settings", "BrightnessService")
        _launchProcess.running = false
        // Try multiple tools in order of preference
        _launchProcess.command = ["sh", "-c", "xfce4-display-settings || arandr || gnome-control-center display || systemsettings5 kcm_kscreen"]
        _launchProcess.running = true
    }

    property var _launchProcess: Process {
        id: launchProcess
        command: []
        onExited: function(code, status) {
            if (code !== 0) {
                Core.Logger.warning("Failed to launch display settings", "BrightnessService")
            }
        }
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        initialize()
    }
}
