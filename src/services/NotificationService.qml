// NotificationService.qml - Notification Daemon Integration
//
// Singleton for integrating with the system notification daemon.
// Handles notification reception, management, and do-not-disturb mode.
//
// Responsibilities:
// - Notification creation/reception via D-Bus
// - Notification queue management
// - Do not disturb mode
// - Notification history
// - Dismissal handling
//
// Usage:
// import "../services"
//
// Connections {
//     target: NotificationService
//     function onNotificationReceived(notification) { showPopup(notification) }
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

    // Maximum number of notifications to keep in history
    property int maxHistorySize: 100

    // Default notification timeout in ms
    property int defaultTimeout: 5000

    // Maximum number of visible popups
    property int maxVisiblePopups: 5

    // ========================================================================
    // State Properties
    // ========================================================================

    // Do not disturb mode
    property bool doNotDisturb: false

    // List of active notifications (visible)
    property var activeNotifications: []

    // List of all notifications (history)
    property var notificationHistory: []

    // Number of unread notifications
    property int unreadCount: 0

    // Notification count (active + history)
    property int notificationCount: 0

    // Whether the notification center is visible
    property bool centerVisible: false

    // Next notification ID
    property int _nextId: 1

    // ========================================================================
    // Signals
    // ========================================================================

    // Emitted when a new notification is received
    signal notificationReceived(var notification)

    // Emitted when a notification is dismissed
    signal notificationDismissed(int id)

    // Emitted when all notifications are cleared
    signal notificationsCleared()

    // Emitted when do not disturb mode changes
    signal dndModeChanged(bool enabled)

    // Emitted when notification center visibility changes
    signal centerVisibilityChanged(bool visible)

    // ========================================================================
    // Public Methods
    // ========================================================================

    // Create a new notification
    function createNotification(appName, title, body, icon) {
        const notification = {
            id: _nextId++,
            appName: appName || "Qypr",
            title: title || "",
            body: body || "",
            icon: icon || "",
            timestamp: new Date(),
            actions: [],
            urgent: false,
            expired: false,
            dismissed: false
        }

        _addNotification(notification)
        return notification.id
    }

    // Show a notification (same as create but with explicit show intent)
    function show(appName, title, body, options) {
        const opts = options || {}
        const notification = {
            id: _nextId++,
            appName: appName || "Qypr",
            title: title || "",
            body: body || "",
            icon: opts.icon || "",
            timestamp: new Date(),
            actions: opts.actions || [],
            urgent: opts.urgent || false,
            expired: false,
            dismissed: false,
            timeout: opts.timeout || defaultTimeout
        }

        _addNotification(notification)

        // Auto-dismiss after timeout (unless urgent or persistent)
        if (!notification.urgent && notification.timeout > 0) {
            Qt.callLater(function() {
                dismiss(notification.id)
            }, notification.timeout)
        }

        return notification.id
    }

    // Dismiss a notification by ID
    function dismiss(id) {
        for (let i = 0; i < activeNotifications.length; i++) {
            if (activeNotifications[i].id === id) {
                const notification = activeNotifications[i]
                notification.dismissed = true

                // Remove from active list
                const newActive = activeNotifications.slice()
                newActive.splice(i, 1)
                activeNotifications = newActive

                notificationDismissed(id)
                Core.Logger.debug("Dismissed notification " + id, "NotificationService")
                return true
            }
        }
        return false
    }

    // Clear all notifications
    function clearAll() {
        activeNotifications = []
        notificationHistory = []
        unreadCount = 0
        notificationCount = 0
        notificationsCleared()
        Core.Logger.info("All notifications cleared", "NotificationService")
    }

    // Clear active (popup) notifications
    function clearActive() {
        for (let i = 0; i < activeNotifications.length; i++) {
            notificationDismissed(activeNotifications[i].id)
        }
        activeNotifications = []
    }

    // Mark all notifications as read
    function markAllRead() {
        unreadCount = 0
        for (let i = 0; i < notificationHistory.length; i++) {
            notificationHistory[i].read = true
        }
    }

    // Mark a specific notification as read
    function markRead(id) {
        for (let i = 0; i < notificationHistory.length; i++) {
            if (notificationHistory[i].id === id) {
                notificationHistory[i].read = true
                if (unreadCount > 0) {
                    unreadCount--
                }
                break
            }
        }
    }

    // Toggle do not disturb mode
    function toggleDoNotDisturb() {
        doNotDisturb = !doNotDisturb
        dndModeChanged(doNotDisturb)

        if (doNotDisturb) {
            Core.Logger.info("Do not disturb enabled", "NotificationService")
        } else {
            Core.Logger.info("Do not disturb disabled", "NotificationService")
        }
    }

    // Set do not disturb mode
    function setDoNotDisturb(enabled) {
        if (doNotDisturb !== enabled) {
            doNotDisturb = enabled
            dndModeChanged(enabled)
            Core.Logger.info("Do not disturb: " + enabled, "NotificationService")
        }
    }

    // Show/hide notification center
    function showCenter() {
        centerVisible = true
        centerVisibilityChanged(true)
    }

    function hideCenter() {
        centerVisible = false
        centerVisibilityChanged(false)
    }

    function toggleCenter() {
        if (centerVisible) {
            hideCenter()
        } else {
            showCenter()
        }
    }

    // Get notification by ID
    function getNotification(id) {
        for (let i = 0; i < notificationHistory.length; i++) {
            if (notificationHistory[i].id === id) {
                return notificationHistory[i]
            }
        }
        return null
    }

    // Get notifications grouped by date
    function getGroupedNotifications() {
        const groups = {}
        const today = new Date().toDateString()
        const yesterday = new Date(Date.now() - 86400000).toDateString()

        for (let i = 0; i < notificationHistory.length; i++) {
            const notification = notificationHistory[i]
            const date = notification.timestamp.toDateString()

            let groupName
            if (date === today) {
                groupName = "Today"
            } else if (date === yesterday) {
                groupName = "Yesterday"
            } else {
                groupName = date
            }

            if (!groups[groupName]) {
                groups[groupName] = []
            }
            groups[groupName].push(notification)
        }

        return groups
    }

    // Get icon name for notification service
    function getIconName() {
        if (doNotDisturb) {
            return "notifications-disabled"
        }
        if (unreadCount > 0) {
            return "notifications-unread"
        }
        return "notifications"
    }

    // Execute a notification action
    function executeAction(notificationId, actionId) {
        const notification = getNotification(notificationId)
        if (notification && notification.actions) {
            for (let i = 0; i < notification.actions.length; i++) {
                if (notification.actions[i].id === actionId) {
                    if (notification.actions[i].callback) {
                        notification.actions[i].callback()
                    }
                    return true
                }
            }
        }
        return false
    }

    // ========================================================================
    // Private Methods
    // ========================================================================

    function _addNotification(notification) {
        // Add to history first
        const newHistory = [notification].concat(notificationHistory)
        if (newHistory.length > maxHistorySize) {
            newHistory.pop()
        }
        notificationHistory = newHistory

        // Increment counts
        notificationCount = notificationHistory.length
        if (!notification.read) {
            unreadCount++
        }

        // Show popup if not in DND mode and not urgent
        if (!doNotDisturb || notification.urgent) {
            const newActive = activeNotifications.slice()
            newActive.unshift(notification)

            // Limit visible popups
            if (newActive.length > maxVisiblePopups) {
                const removed = newActive.pop()
                notificationDismissed(removed.id)
            }

            activeNotifications = newActive
            notificationReceived(notification)
        }

        Core.Logger.debug("Added notification: " + notification.title, "NotificationService")
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        Core.Logger.info("NotificationService initialized", "NotificationService")
    }
}
