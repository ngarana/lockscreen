// StatusBarRight.qml - Right Section of Status Bar
//
// Right section containing (macOS-style, right-to-left):
// - Control center toggle (quick settings)
// - Notification center badge
// - Volume indicator
// - Network indicator
// - Bluetooth indicator
// - Battery indicator
// - System tray
//
// Compact icon-based design with hover effects.

import QtQuick
import QtQuick.Layouts
import "../../atoms" as Atoms
import "../../molecules" as Molecules
import "../../services" as Services
import "../../theme" as Theme

RowLayout {
    id: root

    spacing: 6
    layoutDirection: Qt.LeftToRight

    // ========================================================================
    // System Tray
    // ========================================================================

    RowLayout {
        id: trayContainer
        visible: Services.BarController.showSystemTray && Services.SystemTrayService.trayItemCount > 0
        spacing: 4

        Repeater {
            id: trayRepeater
            model: Services.SystemTrayService.trayItems

            TrayItem {
                itemId: modelData.id
                itemIcon: Services.SystemTrayService.getItemIcon(modelData.id)
                itemTooltip: Services.SystemTrayService.getItemTooltip(modelData.id)
                needsAttention: Services.SystemTrayService.needsAttention(modelData.id)

                Layout.preferredWidth: 20
                Layout.preferredHeight: 20
            }
        }

        Connections {
            target: Services.SystemTrayService
            function onTrayItemsChanged() {
                trayRepeater.model = Services.SystemTrayService.trayItems
            }
        }
    }

    Divider {
        visible: trayContainer.visible
        Layout.preferredWidth: 1
        Layout.preferredHeight: 18
        color: Theme.ThemeEngine.colors.glassBorder
    }

    // ========================================================================
    // Battery Indicator
    // ========================================================================

    Molecules.BatteryIndicator {
        id: batteryIndicator
        visible: Services.BarController.showBatteryIndicator && Services.BatteryService.percentage > 0
        showPercentage: false

        Layout.preferredWidth: 24
        Layout.preferredHeight: 20
    }

    // ========================================================================
    // Network Indicator
    // ========================================================================

    Molecules.NetworkIndicator {
        id: networkIndicator
        visible: Services.BarController.showNetworkIndicator
        showLabel: false

        Layout.preferredWidth: 20
        Layout.preferredHeight: 20
    }

    // ========================================================================
    // Volume Indicator
    // ========================================================================

    Rectangle {
        id: volumeContainer
        visible: Services.BarController.showVolumeIndicator
        color: "transparent"

        Layout.preferredWidth: 24
        Layout.preferredHeight: 20

        Text {
            id: volumeIcon
            anchors.centerIn: parent
            text: root._volumeIcon
            font.pixelSize: 16
            color: Theme.ThemeEngine.colors.textPrimary

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    // TODO: Toggle mute when AudioService supports it
                    // For now, just log
                }
                onWheel: function(wheel) {
                    // Adjust volume on scroll
                    const delta = wheel.angleDelta.y > 0 ? 0.05 : -0.05
                    Services.AudioService.setVolume(Math.max(0, Math.min(1, Services.AudioService.volume + delta)))
                }
            }
        }
    }

    Divider {
        Layout.preferredWidth: 1
        Layout.preferredHeight: 18
        color: Theme.ThemeEngine.colors.glassBorder
    }

    // ========================================================================
    // Control Center Toggle
    // ========================================================================

    Rectangle {
        id: controlCenterButton
        color: "transparent"

        Layout.preferredWidth: 24
        Layout.preferredHeight: 24

        property bool hovered: controlCenterMouse.containsMouse

        Text {
            anchors.centerIn: parent
            text: Theme.ThemeEngine.icons.settings
            font.pixelSize: 16
            color: controlCenterButton.hovered ? Theme.ThemeEngine.colors.textPrimary : Theme.ThemeEngine.colors.textSecondary
        }

        MouseArea {
            id: controlCenterMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                // TODO: Toggle control center when module is implemented
            }
        }
    }

    // ========================================================================
    // Notification Center Toggle
    // ========================================================================

    Rectangle {
        id: notificationCenterButton
        color: "transparent"

        Layout.preferredWidth: 24
        Layout.preferredHeight: 24

        property bool hovered: notificationCenterMouse.containsMouse

        Text {
            anchors.centerIn: parent
            text: Theme.ThemeEngine.icons.notifications
            font.pixelSize: 16
            color: notificationCenterButton.hovered ? Theme.ThemeEngine.colors.textPrimary : Theme.ThemeEngine.colors.textSecondary
        }

        // Badge for unread notifications
        Rectangle {
            id: badge
            visible: Services.NotificationService.unreadCount > 0
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: -2
            anchors.rightMargin: -2
            width: 12
            height: 12
            radius: 6
            color: Theme.ThemeEngine.colors.error

            Text {
                anchors.centerIn: parent
                text: Services.NotificationService.unreadCount > 99 ? "99+" : Services.NotificationService.unreadCount
                font.pixelSize: 7
                font.weight: Font.Bold
                color: Theme.ThemeEngine.colors.background
            }
        }

        MouseArea {
            id: notificationCenterMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                // TODO: Toggle notification center when module is implemented
            }
        }
    }

    // ========================================================================
    // Public Properties
    // ========================================================================

    // Volume icon based on current state
    readonly property string _volumeIcon: {
        // TODO: Update when AudioService has mute support
        if (Services.AudioService.volume === 0) {
            return Theme.ThemeEngine.icons.volumeMuted
        } else if (Services.AudioService.volume < 0.33) {
            return Theme.ThemeEngine.icons.volumeLow
        } else if (Services.AudioService.volume < 0.66) {
            return Theme.ThemeEngine.icons.volumeMedium
        } else {
            return Theme.ThemeEngine.icons.volumeHigh
        }
    }

    // ========================================================================
    // Divider Component
    // ========================================================================

    component Divider: Rectangle {
        color: Theme.ThemeEngine.colors.surface1
        radius: 1
    }

    // ========================================================================
    // Service Connections
    // ========================================================================

    Connections {
        target: Services.AudioService
        function onVolumeChanged() {
            // Force update of volume icon
        }
    }
}
