// StatusBarRight.qml - Right Section of Status Bar
//
// Right section mimicking macOS layout exactly:
// Features standard macOS tray icons (Screen sharing, display, bluetooth, etc.),
// weather, network, battery, control center, and uniform clock.
//
// Compact icon-based design, minimal spacing.

import QtQuick
import QtQuick.Layouts
import "." as StatusBarModule
import "../../atoms" as Atoms
import "../../molecules" as Molecules
import "../../services" as Services
import "../../theme" as Theme

RowLayout {
    id: root

    spacing: 12
    layoutDirection: Qt.LeftToRight

    // ========================================================================
    // Hyprland Workspace Indicator
    // ========================================================================
    
    Molecules.WorkspaceIndicator {
        visible: Services.BarController && Services.BarController.showWorkspaceIndicator
        Layout.alignment: Qt.AlignVCenter
    }

    // ========================================================================
    // System Tray
    // ========================================================================

    RowLayout {
        id: trayContainer
        visible: Services.BarController && Services.BarController.showSystemTray
                 && Services.SystemTrayService && Services.SystemTrayService.trayItemCount > 0
        spacing: 12

        Repeater {
            id: trayRepeater
            model: Services.SystemTrayService ? Services.SystemTrayService.trayItems : []

            StatusBarModule.TrayItem {
                itemId: modelData ? String(modelData.id) : ""
                itemIcon: (Services.SystemTrayService && modelData)
                          ? String(Services.SystemTrayService.getItemIcon(modelData.id)) : ""
                itemTooltip: (Services.SystemTrayService && modelData)
                             ? String(Services.SystemTrayService.getItemTooltip(modelData.id)) : ""
                needsAttention: (Services.SystemTrayService && modelData)
                                ? Services.SystemTrayService.needsAttention(modelData.id) : false

                Layout.preferredWidth: 20
                Layout.preferredHeight: 20
            }
        }

        Connections {
            target: Services.SystemTrayService
            enabled: target !== null
            function onTrayItemsChanged() {
                trayRepeater.model = Services.SystemTrayService.trayItems
            }
        }
    }

    // ========================================================================
    // Supplementary macOS Icons (Mirroring, Display, Focus, Grid)
    // ========================================================================

    RowLayout {
        spacing: 4
        Layout.alignment: Qt.AlignVCenter

        Atoms.IconButton {
            icon: Theme.ThemeEngine.icons.screenMirroring
            size: 26
            iconSize: 14
            tooltip: "AirPlay"
        }

        Atoms.IconButton {
            icon: Theme.ThemeEngine.icons.display
            size: 26
            iconSize: 14
            tooltip: "Display"
            onClicked: Services.BarController.toggleModule("monitor")
        }

        Atoms.IconButton {
            icon: Theme.ThemeEngine.icons.focus
            size: 26
            iconSize: 14
            tooltip: "Focus"
        }

        // Volume Control Toggle
        Atoms.IconButton {
            icon: root._volumeIcon
            size: 26
            iconSize: 14
            tooltip: "Volume: " + Math.round((Services.AudioService ? Services.AudioService.volume : 0) * 100) + "%"
            onClicked: Services.BarController.toggleModule("quicksettings")
        }

        // Brightness Control Toggle
        Atoms.IconButton {
            icon: root._brightnessIcon
            size: 26
            iconSize: 14
            tooltip: "Brightness: " + (Services.BrightnessService ? Services.BrightnessService.primaryBrightness : 100) + "%"
            onClicked: Services.BarController.toggleModule("quicksettings")
            onWheel: function(wheel) {
                if (Services.BrightnessService && Services.BrightnessService.isAvailable) {
                    if (wheel.angleDelta.y > 0) {
                        Services.BrightnessService.increase()
                    } else {
                        Services.BrightnessService.decrease()
                    }
                }
            }
        }

        Atoms.IconButton {
            icon: Theme.ThemeEngine.icons.grid
            size: 26
            iconSize: 14
            tooltip: "Windows"
        }
    }

    // ========================================================================
    // Status Indicators (Molecules)
    // ========================================================================

    Molecules.NetworkIndicator {
        visible: Services.BarController && Services.BarController.showNetworkIndicator
        Layout.alignment: Qt.AlignVCenter
        onClicked: Services.BarController.toggleModule("quicksettings")
    }

    Molecules.BatteryIndicator {
        visible: Services.BarController && Services.BarController.showBatteryIndicator
        Layout.alignment: Qt.AlignVCenter
        showPercentage: true
    }

    // ========================================================================
    // Notification & Control Center Popups
    // ========================================================================

    RowLayout {
        spacing: 4
        Layout.alignment: Qt.AlignVCenter

        // Notification Center Toggle
        Atoms.IconButton {
            id: notificationToggle
            icon: Theme.ThemeEngine.icons.notifications
            size: 26
            iconSize: 14
            tooltip: "Notifications"
            onClicked: Services.BarController.toggleModule("notifications")

            // Badge - overlaying the icon button
            Atoms.Badge {
                visible: Services.NotificationService && Services.NotificationService.unreadCount > 0
                text: Services.NotificationService.unreadCount > 9 ? "9+" : String(Services.NotificationService.unreadCount)
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.topMargin: 2
                anchors.rightMargin: 2
                size: "small"
            }
        }

        // Control Center (Quick Settings) Toggle
        Atoms.IconButton {
            icon: Theme.ThemeEngine.icons.settings
            size: 26
            iconSize: 14
            tooltip: "Control Center"
            onClicked: Services.BarController.toggleModule("quicksettings")
        }
    }

    // ========================================================================
    // Clock Display
    // ========================================================================

    Atoms.Label {
        text: root._timeText
        visible: Services.BarController && Services.BarController.showClock
        fontSize: Theme.ThemeEngine.typography.sizeSm
        fontWeight: Theme.ThemeEngine.typography.weightMedium
        color: Theme.ThemeEngine.colors.textPrimary
        Layout.alignment: Qt.AlignVCenter
        
        MouseArea {
            anchors.fill: parent
            onClicked: Services.BarController.toggleModule("weather")
        }
    }

    // ========================================================================
    // Internal Properties
    // ========================================================================

    property string _timeText: ""

    readonly property string _networkIcon: {
        if (Services.NetworkService && Services.NetworkService.isConnected) {
            return Services.NetworkService.connectionType === "wifi"
                ? Theme.ThemeEngine.icons.wifi
                : Theme.ThemeEngine.icons.ethernet
        }
        return Theme.ThemeEngine.icons.wifiOff
    }

    readonly property string _batteryIcon: {
        if (Services.BatteryService && Services.BatteryService.isCharging) {
            return Theme.ThemeEngine.icons.batteryCharging
        }
        return Theme.ThemeEngine.icons.battery
    }

    readonly property string _volumeIcon: {
        const volume = (Services.AudioService && Services.AudioService.volume) || 0
        if (volume <= 0.05) { // Handle near-zero/muted
            return Theme.ThemeEngine.icons.volumeMuted
        } else if (volume < 0.33) {
            return Theme.ThemeEngine.icons.volumeLow
        } else if (volume < 0.66) {
            return Theme.ThemeEngine.icons.volumeMedium
        } else {
            return Theme.ThemeEngine.icons.volumeHigh
        }
    }

    readonly property string _brightnessIcon: {
        const brightness = (Services.BrightnessService && Services.BrightnessService.primaryBrightness) || 100
        if (brightness < 50) {
            return Theme.ThemeEngine.icons.brightnessLow
        } else {
            return Theme.ThemeEngine.icons.brightness
        }
    }

    // ========================================================================
    // Update Timer for Clock
    // ========================================================================

    Timer {
        id: clockTimer
        interval: 1000
        running: true
        repeat: true
        triggeredOnStart: true
        onTriggered: root._updateClock()
    }

    function _updateClock() {
        const now = new Date()
        
        const days = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat']
        const months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec']
        
        const dayName = days[now.getDay()]
        const monthName = months[now.getMonth()]
        const date = now.getDate()
        const hours = root._pad(now.getHours())
        const minutes = root._pad(now.getMinutes())
        const seconds = root._pad(now.getSeconds())
        
        root._timeText = dayName + " " + monthName + " " + date + " " + hours + ":" + minutes + ":" + seconds
    }

    function _pad(num) {
        return num < 10 ? "0" + num : num
    }

    // ========================================================================
    // Service Connections
    // ========================================================================

    Connections {
        target: Services.AudioService
        enabled: target !== null
        function onVolumeChanged() {} // Force update
    }

    Connections {
        target: Services.BrightnessService
        enabled: target !== null
        function onBrightnessChanged() {} // Force update
    }

    Connections {
        target: Services.NetworkService
        enabled: target !== null
        function onNetworkStatusChanged() {} // Force update
    }

    Connections {
        target: Services.BatteryService
        enabled: target !== null
        function onBatteryLevelChanged() {} // Force update
        function onChargingStatusChanged() {} // Force update
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        _updateClock()
        // Force service instantiation
        if (Services.BatteryService) {
            Services.BatteryService.refresh()
        }
        if (Services.NetworkService) {
            Services.NetworkService.refresh()
        }
        if (Services.BrightnessService) {
            Services.BrightnessService.refresh()
        }
    }
}