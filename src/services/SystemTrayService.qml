// SystemTrayService.qml - System Tray Protocol Service
//
// Singleton for managing system tray icons and context menus.
// Implements the StatusNotifierItem (SNI) protocol for tray icon support.
//
// Responsibilities:
// - System tray protocol implementation (SNI)
// - Tray icon management
// - Context menu handling
// - Tray item activation
//
// Usage:
// import "../services"
//
// Connections {
//     target: SystemTrayService
//     function onTrayItemsChanged() { updateTrayIcons() }
// }

pragma Singleton
import Quickshell
import QtQuick
import "../core" as Core

QtObject {
    id: root

    // ========================================================================
    // Configuration
    // ========================================================================

    // Maximum number of tray items to display
    property int maxItems: 20

    // Update interval for polling (ms)
    property int updateInterval: 2000

    // ========================================================================
    // State Properties (Read-Only)
    // ========================================================================

    // List of tray items
    property var trayItems: []

    // Number of tray items
    property int trayItemCount: 0

    // Whether tray is available/enabled
    property bool isAvailable: true

    // Whether SNI watcher is available
    property bool sniAvailable: false

    // ========================================================================
    // Signals
    // ========================================================================

    // Emitted when a tray item is activated
    signal trayItemActivated(string itemId)

    // Emitted when a tray item menu is requested
    signal trayItemMenuRequested(string itemId)

    // ========================================================================
    // Private Properties
    // ========================================================================

    property var _updateTimer: Timer {
        interval: root.updateInterval
        running: true
        repeat: true
        onTriggered: root._updateTrayItems()
    }

    // ========================================================================
    // Public Methods
    // ========================================================================

    // Refresh tray items
    function refresh() {
        _updateTrayItems()
    }

    // Activate a tray item (left-click)
    function activateItem(itemId) {
        Core.Logger.debug("Activating tray item: " + itemId, "SystemTrayService")

        // Find the item
        for (let i = 0; i < trayItems.length; i++) {
            if (trayItems[i].id === itemId) {
                // Emit signal
                trayItemActivated(itemId)

                // Try to activate via dbus (placeholder)
                _activateViaDBus(itemId, "Activate")
                return true
            }
        }
        return false
    }

    // Secondary activate a tray item (right-click - open menu)
    function secondaryActivateItem(itemId) {
        Core.Logger.debug("Secondary activate tray item: " + itemId, "SystemTrayService")

        // Find the item
        for (let i = 0; i < trayItems.length; i++) {
            if (trayItems[i].id === itemId) {
                // Emit signal
                trayItemMenuRequested(itemId)

                // Try to show menu via dbus (placeholder)
                _activateViaDBus(itemId, "ContextMenu")
                return true
            }
        }
        return false
    }

    // Scroll on a tray item
    function scrollOnItem(itemId, delta, orientation) {
        // orientation: "horizontal" or "vertical"
        Core.Logger.debug("Scroll on tray item: " + itemId + " delta: " + delta, "SystemTrayService")
        // Implementation would send Scroll event via dbus
    }

    // Get item by ID
    function getItem(itemId) {
        for (let i = 0; i < trayItems.length; i++) {
            if (trayItems[i].id === itemId) {
                return trayItems[i]
            }
        }
        return null
    }

    // Get icon for tray item
    function getItemIcon(itemId) {
        const item = getItem(itemId)
        if (item && item.icon) {
            return item.icon
        }
        return "application-x-executable-symbolic"
    }

    // Get tooltip for tray item
    function getItemTooltip(itemId) {
        const item = getItem(itemId)
        if (item && item.tooltip) {
            return item.tooltip
        }
        return item ? item.title : ""
    }

    // Get attention icon (for notifications)
    function getAttentionIcon(itemId) {
        const item = getItem(itemId)
        if (item && item.attentionIcon) {
            return item.attentionIcon
        }
        return getItemIcon(itemId)
    }

    // Check if item needs attention
    function needsAttention(itemId) {
        const item = getItem(itemId)
        return item ? item.needsAttention : false
    }

    // ========================================================================
    // Private Methods
    // ========================================================================

    function _updateTrayItems() {
        // In a full implementation, this would:
        // 1. Check for StatusNotifierWatcher on D-Bus
        // 2. Get list of registered items from org.freedesktop.StatusNotifierWatcher
        // 3. For each item, get properties from StatusNotifierItem interface

        // For now, we simulate with common tray applications
        // This is a placeholder - real implementation would use D-Bus

        const items = []

        // Check for common tray applications (Network/Bluetooth are natively handled so mock triggers are removed to prevent image://icon warnings)
        // _checkForTrayApp(items, "nm-applet", "Network Manager", "network-wireless-symbolic", "nm-applet")
        // _checkForTrayApp(items, "blueman-applet", "Bluetooth", "bluetooth-symbolic", "blueman-manager")
        // _checkForTrayApp(items, "volumeicon", "Volume", "audio-volume-high-symbolic", "pavucontrol")
        _checkForTrayApp(items, "parcellite", "Clipboard", "edit-paste-symbolic", "parcellite")
        _checkForTrayApp(items, "kdeconnect-indicator", "KDE Connect", "phone-symbolic", "kdeconnect-app")
        _checkForTrayApp(items, "flameshot", "Flameshot", "camera-photo-symbolic", "flameshot")

        // Update items if changed
        if (JSON.stringify(items) !== JSON.stringify(trayItems)) {
            trayItems = items
            trayItemCount = items.length
            trayItemsChanged()
        }
    }

    function _checkForTrayApp(items, processName, title, icon, tooltip) {
        // This is a simplified check - real implementation would use D-Bus
        // Check if process is running
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["pgrep", "-x", "' + processName + '"]; running: true; onExited: function(c) { if (c === 0) root._addTrayItem("' + processName + '", "' + title + '", "' + icon + '", "' + tooltip + '") } }',
            root
        )
    }

    function _addTrayItem(id, title, icon, tooltip) {
        // This is called from the process callback
        // In practice, we'd maintain the items list properly
        const newItems = trayItems.slice()

        // Check if already exists
        let exists = false
        for (let i = 0; i < newItems.length; i++) {
            if (newItems[i].id === id) {
                exists = true
                break
            }
        }

        if (!exists) {
            newItems.push({
                id: id,
                title: title,
                icon: icon,
                tooltip: tooltip,
                attentionIcon: "",
                needsAttention: false,
                category: "ApplicationStatus",
                status: "Active"
            })

            trayItems = newItems
            trayItemCount = newItems.length
            trayItemsChanged()
        }
    }

    function _removeTrayItem(id) {
        const newItems = []
        for (let i = 0; i < trayItems.length; i++) {
            if (trayItems[i].id !== id) {
                newItems.push(trayItems[i])
            }
        }

        if (newItems.length !== trayItems.length) {
            trayItems = newItems
            trayItemCount = newItems.length
            trayItemsChanged()
        }
    }

    function _activateViaDBus(itemId, action) {
        // Placeholder for D-Bus activation
        // Real implementation would:
        // 1. Get the service name from the itemId
        // 2. Call the appropriate method on the StatusNotifierItem interface
        Core.Logger.debug("D-Bus " + action + " for " + itemId, "SystemTrayService")
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        // Check if StatusNotifierWatcher is available
        Core.Logger.info("SystemTrayService initialized", "SystemTrayService")
        _updateTrayItems()
    }
}
