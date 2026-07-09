// BarController.qml - Status Bar Controller Singleton
//
// Singleton controller for managing status bar state, visibility,
// and module configuration across all instances.
//
// Usage:
//   import "../../services" as Services
//
//   Connections {
//       target: Services.BarController
//       function onVisibilityChanged(visible) { handleVisibilityChange(visible) }
//   }

pragma Singleton
import QtQuick
import "../../core" as Core

QtObject {
    id: root

    // ========================================================================
    // Configuration
    // ========================================================================

    property bool isVisible: true

    property bool autoHide: false
    property int autoHideDelay: 1000

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

    property bool isHovered: false
    property int currentScreen: 0
    property int screenCount: 1

    // ========================================================================
    // Signals
    // ========================================================================

    signal barVisibilityChanged(bool visible)
    signal heightUpdated(int height)
    signal moduleVisibilityChanged(string module, bool visible)

    // ========================================================================
    // Public Methods
    // ========================================================================

    function toggleVisibility() {
        root.isVisible = !root.isVisible
        barVisibilityChanged(root.isVisible)
        Core.Logger.debug("Bar visibility: " + (root.isVisible ? "shown" : "hidden"), "BarController")
    }

    function toggleAutoHide() {
        root.autoHide = !root.autoHide
        Core.Logger.debug("Auto-hide: " + (root.autoHide ? "enabled" : "disabled"), "BarController")
    }

    function toggleModule(module) {
        switch (module) {
            case "quicksettings":
            case "weather":
            case "notifications":
            case "monitor":
            case "audio":
                _togglePopupVisibility(module)
                break
            default:
                Core.Logger.warning("Unknown module: " + module, "BarController")
        }
    }

    function closePopups() {
        _setPopupState("quicksettings", false)
        _setPopupState("weather", false)
        _setPopupState("notifications", false)
        _setPopupState("monitor", false)
        _setPopupState("audio", false)
    }

    function show() {
        if (!root.isVisible) {
            root.isVisible = true
            barVisibilityChanged(true)
        }
    }

    function hide() {
        if (root.isVisible) {
            root.isVisible = false
            barVisibilityChanged(false)
        }
    }

    function initialize() {
        Core.Logger.info("BarController initialized", "BarController")
    }

    function _popupState(module) {
        switch (module) {
            case "quicksettings": return root.quickSettingsVisible
            case "weather": return root.weatherVisible
            case "notifications": return root.notificationsVisible
            case "monitor": return root.systemMonitorVisible
            case "audio": return root.audioControllerVisible
            default: return false
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

    Component.onCompleted: {
        initialize()
    }
}
