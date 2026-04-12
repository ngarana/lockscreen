// NotificationItem.qml - Single Notification Display Molecule
//
// Displays a notification with app icon, title, body, and actions.
// Supports dismiss button and timestamp.
//
// Usage:
//   NotificationItem {
//       notification: notificationObject
//       onDismissed: removeNotification(notification.id)
//   }
//
// Properties:
//   - notification: var - Notification object from NotificationService
//   - compact: bool - Compact mode for popups (default: false)
//
// Signals:
//   - dismissed(): Emitted when dismiss button is clicked
//   - actionTriggered(actionId): Emitted when action is clicked
//   - clicked(): Emitted when notification is clicked

import QtQuick
import QtQuick.Layouts
import "../atoms" as Atoms
import "../theme" as Theme

Atoms.Card {
    id: root

    // Public API
    property var notification: null
    property bool compact: false

    signal dismissed()
    signal actionTriggered(string actionId)
    signal clicked()

    // Internal state
    readonly property string _appName: notification ? notification.appName : ""
    readonly property string _title: notification ? notification.title : ""
    readonly property string _body: notification ? notification.body : ""
    readonly property string _icon: notification ? notification.icon : ""
    readonly property var _timestamp: notification ? notification.timestamp : null
    readonly property bool _urgent: notification ? notification.urgent : false
    readonly property var _actions: notification ? notification.actions : []

    // Layout
    width: root.compact ? 320 : 360
    implicitHeight: contentColumn.implicitHeight + (root.padding * 2)
    padding: root.compact ? 12 : 16
    hoverable: true
    elevated: root.compact

    // Visual state
    border.width: _urgent ? 2 : 1
    border.color: _urgent ? Theme.ThemeEngine.colors.error : Theme.ThemeEngine.colors.glassBorder

    // Content column
    ColumnLayout {
        id: contentColumn
        anchors.fill: parent
        spacing: 8

        // Header row
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            // App icon
            Atoms.Icon {
                source: root._icon || "notification"
                size: root.compact ? 16 : 20
                color: Theme.ThemeEngine.colors.textSecondary
            }

            // App name
            Atoms.Label {
                Layout.fillWidth: true
                text: root._appName
                fontSize: Theme.ThemeEngine.typography.sizeXs
                color: Theme.ThemeEngine.colors.textMuted
            }

            // Timestamp
            Atoms.Label {
                text: root._formatTime(root._timestamp)
                fontSize: Theme.ThemeEngine.typography.sizeXs
                color: Theme.ThemeEngine.colors.textMuted
            }

            // Dismiss button
            Atoms.IconButton {
                visible: !root.compact
                size: 20
                iconSize: 12
                icon: "close"
                tooltip: "Dismiss"
                circular: true
                onClicked: root.dismissed()
            }
        }

        // Title
        Atoms.Label {
            Layout.fillWidth: true
            visible: root._title !== ""
            text: root._title
            fontSize: Theme.ThemeEngine.typography.sizeMd
            fontWeight: Theme.ThemeEngine.typography.weightSemiBold
            color: Theme.ThemeEngine.colors.textPrimary
            wrap: true
            maxLines: 2
        }

        // Body
        Atoms.Label {
            Layout.fillWidth: true
            visible: root._body !== ""
            text: root._body
            fontSize: Theme.ThemeEngine.typography.sizeSm
            color: Theme.ThemeEngine.colors.textSecondary
            wrap: true
            maxLines: root.compact ? 3 : 5
        }

        // Actions
        RowLayout {
            Layout.fillWidth: true
            visible: root._actions && root._actions.length > 0
            spacing: 8

            Repeater {
                model: root._actions

                Atoms.TextButton {
                    text: modelData.label || modelData.id
                    variant: "secondary"
                    size: "small"
                    onClicked: root.actionTriggered(modelData.id)
                }
            }
        }
    }

    // Click to expand/interact
    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }

    // Format timestamp
    function _formatTime(timestamp) {
        if (!timestamp) return ""

        var now = new Date()
        var diff = Math.floor((now - timestamp) / 1000) // seconds

        if (diff < 60) return "now"
        if (diff < 3600) return Math.floor(diff / 60) + "m"
        if (diff < 86400) return Math.floor(diff / 3600) + "h"
        return Math.floor(diff / 86400) + "d"
    }
}
