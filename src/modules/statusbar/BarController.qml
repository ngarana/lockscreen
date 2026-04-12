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

    // Bar visibility
    property bool visible: true

    // Bar height in pixels
    property int barHeight: 36

    // Auto-hide behavior
    property bool autoHide: false

    // Auto-hide delay in milliseconds
    property int autoHideDelay: 1000

    // Module visibility toggles
    property bool showWorkspaceIndicator: true
    property bool showClock: true
    property bool showSystemTray: true
    property bool showNetworkIndicator: true
    property bool showBatteryIndicator: true
    property bool showVolumeIndicator: true
    property bool showBrightnessIndicator: false

    // Layout mode: "macos" (centered clock, spread sections) or "windows" (right-aligned)
    property string layoutMode: "macos"

    // Multi-monitor mode: "all" (show on all), "primary" (primary only), "active" (active only)
    property string monitorMode: "all"

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
    signal visibilityChanged(bool visible)

    // Emitted when height changes
    signal heightChanged(int height)

    // Emitted when layout mode changes
    signal layoutModeChanged(string mode)

    // Emitted when a module toggle changes
    signal moduleVisibilityChanged(string module, bool visible)

    // ========================================================================
    // Public Methods
    // ========================================================================

    // Toggle bar visibility
    function toggleVisibility() {
        root.visible = !root.visible
        visibilityChanged(root.visible)
        Core.Logger.debug("Bar visibility: " + (root.visible ? "shown" : "hidden"), "BarController")
    }

    // Set bar height
    function setHeight(height) {
        if (height >= 24 && height <= 64) {
            root.barHeight = height
            heightChanged(height)
        } else {
            Core.Logger.warning("Invalid bar height: " + height, "BarController")
        }
    }

    // Toggle auto-hide
    function toggleAutoHide() {
        root.autoHide = !root.autoHide
        Core.Logger.debug("Auto-hide: " + (root.autoHide ? "enabled" : "disabled"), "BarController")
    }

    // Set layout mode
    function setLayoutMode(mode) {
        if (mode === "macos" || mode === "windows") {
            root.layoutMode = mode
            layoutModeChanged(mode)
            Core.Logger.debug("Layout mode: " + mode, "BarController")
        } else {
            Core.Logger.warning("Invalid layout mode: " + mode, "BarController")
        }
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
            default:
                Core.Logger.warning("Unknown module: " + module, "BarController")
        }
    }

    // Show the bar (override auto-hide)
    function show() {
        if (!root.visible) {
            root.visible = true
            visibilityChanged(true)
        }
    }

    // Hide the bar (override auto-hide)
    function hide() {
        if (root.visible) {
            root.visible = false
            visibilityChanged(false)
        }
    }

    // Initialize with defaults
    function initialize() {
        Core.Logger.info("BarController initialized", "BarController")
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        initialize()
    }
}
