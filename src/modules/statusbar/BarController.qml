// BarController.qml - Status Bar Controller Singleton
//
// Singleton controller for managing status bar state, visibility,
// and module configuration across all instances.
//
// Usage:
// import "../services" as Services
//
// Connections {
//     target: Services.BarController
//     function onVisibilityChanged(visible) { handleVisibilityChange(visible) }
// }

pragma Singleton
import QtQuick
import "../../core" as Core

QtObject {
    id: root

    // ========================================================================
    // Configuration
    // ========================================================================

    // Bar visibility (renamed to avoid signal conflict with QtObject)
    property bool isVisible: true

    // Bar height in pixels (macOS-style compact)
    property int barHeight: 26

    // Auto-hide behavior (disabled for macOS-style always-visible bar)
    property bool autoHide: false

    // Auto-hide delay in milliseconds
    property int autoHideDelay: 1000

    // Module visibility toggles
    property bool showWorkspaceIndicator: false
    property bool showClock: true
    property bool showSystemTray: true
    property bool showNetworkIndicator: true
    property bool showBatteryIndicator: true
    property bool showVolumeIndicator: true
    property bool showBrightnessIndicator: true

    // Multi-monitor mode: "all" (show on all), "primary" (primary only), "active" (active only)
    property string monitorMode: "all"

    // Popup visibility states
    property bool quickSettingsVisible: false
    property bool weatherVisible: false
    property bool notificationsVisible: false
    property bool systemMonitorVisible: false
    property bool audioControllerVisible: false

    // ========================================================================
    // State Properties
    // ========================================================================

    // Whether the bar is currently hovered (for auto-hide)
    property bool isHovered: false

    // Current screen index
    property int currentScreen: 0

    // Number of screens
    property int screenCount: 1

    // ========================================================================
    // Signals
    // ========================================================================

    // Emitted when visibility changes
    signal barVisibilityChanged(bool visible)

    // Emitted when height changes
    signal heightUpdated(int height)

    // Emitted when a module toggle changes
    signal moduleVisibilityChanged(string module, bool visible)

    // ========================================================================
    // Public Methods
    // ========================================================================

    // Toggle bar visibility
    function toggleVisibility() {
        root.isVisible = !root.isVisible
        barVisibilityChanged(root.isVisible)
        Core.Logger.debug("Bar visibility: " + (root.isVisible ? "shown" : "hidden"), "BarController")
    }

    // Set bar height
    function setHeight(height) {
        if (height >= 24 && height <= 64) {
            root.barHeight = height
            heightUpdated(height)
        } else {
            Core.Logger.warning("Invalid bar height: " + height, "BarController")
        }
    }

    // Toggle auto-hide
    function toggleAutoHide() {
        root.autoHide = !root.autoHide
        Core.Logger.debug("Auto-hide: " + (root.autoHide ? "enabled" : "disabled"), "BarController")
    }

    // Toggle a specific module visibility
    function toggleModule(module) {
        switch (module) {
            case "workspace":
                root.showWorkspaceIndicator = !root.showWorkspaceIndicator
                moduleVisibilityChanged(module, root.showWorkspaceIndicator)
                break
            case "clock":
                root.showClock = !root.showClock
                moduleVisibilityChanged(module, root.showClock)
                break
            case "tray":
                root.showSystemTray = !root.showSystemTray
                moduleVisibilityChanged(module, root.showSystemTray)
                break
            case "network":
                root.showNetworkIndicator = !root.showNetworkIndicator
                moduleVisibilityChanged(module, root.showNetworkIndicator)
                break
            case "battery":
                root.showBatteryIndicator = !root.showBatteryIndicator
                moduleVisibilityChanged(module, root.showBatteryIndicator)
                break
            case "volume":
                root.showVolumeIndicator = !root.showVolumeIndicator
                moduleVisibilityChanged(module, root.showVolumeIndicator)
                break
            case "quicksettings":
                _togglePopupVisibility(module)
                break
            case "weather":
                _togglePopupVisibility(module)
                break
            case "notifications":
                _togglePopupVisibility(module)
                break
            case "monitor":
                _togglePopupVisibility(module)
                break
            case "audio":
                _togglePopupVisibility(module)
                break
            default:
                Core.Logger.warning("Unknown module: " + module, "BarController")
        }
    }

    // Close all popups
    function closePopups() {
        _setPopupState("quicksettings", false)
        _setPopupState("weather", false)
        _setPopupState("notifications", false)
        _setPopupState("monitor", false)
        _setPopupState("audio", false)
    }

    // Show the bar (override auto-hide)
    function show() {
        if (!root.isVisible) {
            root.isVisible = true
            barVisibilityChanged(true)
        }
    }

    // Hide the bar (override auto-hide)
    function hide() {
        if (root.isVisible) {
            root.isVisible = false
            barVisibilityChanged(false)
        }
    }

    // Initialize with defaults
    function initialize() {
        Core.Logger.info("BarController initialized", "BarController")
    }

    function _popupState(module) {
        switch (module) {
            case "quicksettings":
                return root.quickSettingsVisible
            case "weather":
                return root.weatherVisible
            case "notifications":
                return root.notificationsVisible
            case "monitor":
                return root.systemMonitorVisible
            case "audio":
                return root.audioControllerVisible
            default:
                return false
        }
    }

    function _setPopupState(module, visible) {
        switch (module) {
            case "quicksettings":
                if (root.quickSettingsVisible !== visible) {
                    root.quickSettingsVisible = visible
                    moduleVisibilityChanged(module, visible)
                }
                break
            case "weather":
                if (root.weatherVisible !== visible) {
                    root.weatherVisible = visible
                    moduleVisibilityChanged(module, visible)
                }
                break
            case "notifications":
                if (root.notificationsVisible !== visible) {
                    root.notificationsVisible = visible
                    moduleVisibilityChanged(module, visible)
                }
                break
            case "monitor":
                if (root.systemMonitorVisible !== visible) {
                    root.systemMonitorVisible = visible
                    moduleVisibilityChanged(module, visible)
                }
                break
            case "audio":
                if (root.audioControllerVisible !== visible) {
                    root.audioControllerVisible = visible
                    moduleVisibilityChanged(module, visible)
                }
                break
            default:
                Core.Logger.warning("Unknown popup module: " + module, "BarController")
        }
    }

    function _togglePopupVisibility(module) {
        const nextVisible = !root._popupState(module)
        root.closePopups()
        root._setPopupState(module, nextVisible)
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        initialize()
    }
}
