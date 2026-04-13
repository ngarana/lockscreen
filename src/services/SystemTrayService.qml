// SystemTrayService.qml - System Tray Protocol Service
//
// Singleton for managing system tray icons and context menus.
// Implements the StatusNotifierItem (SNI) protocol via Quickshell.Services.SystemTray.
//
// Responsibilities:
// - System tray protocol integration
// - Tray icon management
// - Tray item activation and interaction
//
// Usage:
// import "../services"
//
// Repeater {
//     model: SystemTrayService.trayItems
//     // ...
// }

pragma Singleton
import Quickshell
import Quickshell.Services.SystemTray
import QtQuick
import "../core" as Core

QtObject {
    id: root

    // ========================================================================
    // State Properties (Read-Only)
    // ========================================================================

    // Direct access to the system tray items model
    // This is an ObjectModel containing SystemTrayItem objects
    property var trayItems: SystemTray.items

    // Reactive count of tray items
    property var trayItemCount: (SystemTray.items && SystemTray.items.count !== undefined) ? SystemTray.items.count : 0

    // Whether tray is available/ready
    property bool isAvailable: true

    // ========================================================================
    // Signals
    // ========================================================================

    // Emitted when the set of tray items changes


    // ========================================================================
    // Public Methods
    // ========================================================================

    // Refresh tray items (usually handled automatically by Quickshell)
    function refresh() {
        // Quickshell handles SNI updates automatically
    }

    // Activate a tray item (typically left-click)
    function activateItem(itemId) {
        const item = _getItemById(itemId);
        if (item) {
            Core.Logger.debug("Activating tray item: " + itemId, "SystemTrayService");
            item.activate();
            return true;
        }
        return false;
    }

    // Secondary activate a tray item (typically right-click)
    function secondaryActivateItem(itemId) {
        const item = _getItemById(itemId);
        if (item) {
            Core.Logger.debug("Secondary activating tray item: " + itemId, "SystemTrayService");
            item.secondaryActivate();
            return true;
        }
        return false;
    }

    // Scroll on a tray item
    function scrollOnItem(itemId, delta, orientation) {
        const item = _getItemById(itemId);
        if (item) {
            item.scroll(delta, orientation === "horizontal");
            return true;
        }
        return false;
    }

    // Helper functions for easy access from components
    
    function getItem(itemId) {
        return _getItemById(itemId);
    }

    function getItemIcon(itemId) {
        const item = _getItemById(itemId);
        if (item) {
            // Return either the icon name or the icon theme path if it exists
            return item.icon || "";
        }
        return "";
    }

    function getItemTooltip(itemId) {
        const item = _getItemById(itemId);
        if (item) {
            return item.tooltipTitle || item.title || "";
        }
        return "";
    }

    function needsAttention(itemId) {
        const item = _getItemById(itemId);
        if (item) {
            // attentionIcon is set when the item is in high-importance state
            return item.attentionIcon !== "";
        }
        return false;
    }

    // ========================================================================
    // Private Helpers
    // ========================================================================

    // Internal helper to find an item in the model by its ID
    function _getItemById(itemId) {
        // Since SystemTray.items is a model, we iterate through it
        for (let i = 0; i < SystemTray.items.count; i++) {
            const item = SystemTray.items.get(i);
            if (item && item.id === itemId) return item;
        }
        return null;
    }

    // Quickshell SystemTray.items model handles its own change notifications. 
    // Consumers like Repeaters will automatically update when the model changes.


    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        Core.Logger.info("SystemTrayService initialized with Quickshell SNI backend", "SystemTrayService");
    }
}
