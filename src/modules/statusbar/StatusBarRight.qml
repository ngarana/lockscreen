// StatusBarRight.qml - Right section of the macOS-style status bar
//
// Provides compact menu-bar status items, grouped controls, and
// click targets for the status bar popups.

import QtQuick
import QtQuick.Layouts
import Quickshell
import "." as StatusBarModule
import "../../atoms" as Atoms
import "../../molecules" as Molecules
import "../../services" as Services
import "../../theme" as Theme

RowLayout {
    id: root

    readonly property string networkIcon: {
        if (!Services.NetworkService || !Services.NetworkService.isConnected) {
            return Theme.ThemeEngine.icons.wifiOff
        }
        if (Services.NetworkService.connectionType === "ethernet") {
            return Theme.ThemeEngine.icons.ethernet
        }
        return Theme.ThemeEngine.icons.wifi
    }

    readonly property string bluetoothIcon: {
        if (!Services.BluetoothService || !Services.BluetoothService.isPowered) {
            return Theme.ThemeEngine.icons.bluetoothOff
        }
        return Theme.ThemeEngine.icons.bluetooth
    }

    readonly property string batteryIcon: {
        if (Services.BatteryService && Services.BatteryService.isCharging) {
            return Theme.ThemeEngine.icons.batteryCharging
        }
        return Theme.ThemeEngine.icons.battery
    }

    readonly property color batteryColor: {
        if (!Services.BatteryService) {
            return Theme.ThemeEngine.colors.textPrimary
        }
        if (Services.BatteryService.isCharging) {
            return Theme.ThemeEngine.colors.success
        }
        if (Services.BatteryService.percentage <= 15) {
            return Theme.ThemeEngine.colors.error
        }
        if (Services.BatteryService.percentage <= 30) {
            return Theme.ThemeEngine.colors.warning
        }
        return Theme.ThemeEngine.colors.textPrimary
    }

    readonly property string batteryLabel: Services.BatteryService ? String(Services.BatteryService.percentage) + "%" : ""

    readonly property string brightnessLabel: {
        if (!Services.BrightnessService) {
            return ""
        }
        return String(Services.BrightnessService.primaryBrightness) + "%"
    }

    readonly property string volumeIcon: {
        const currentVolume = Services.AudioService ? Services.AudioService.volume : 0
        if (currentVolume <= 0.05) {
            return Theme.ThemeEngine.icons.volumeMuted
        }
        if (currentVolume < 0.33) {
            return Theme.ThemeEngine.icons.volumeLow
        }
        if (currentVolume < 0.66) {
            return Theme.ThemeEngine.icons.volumeMedium
        }
        return Theme.ThemeEngine.icons.volumeHigh
    }

    readonly property string volumeLabel: {
        const currentVolume = Services.AudioService ? Services.AudioService.volume : 0
        return String(Math.round(currentVolume * 100)) + "%"
    }

    readonly property string notificationIcon: {
        if (Services.NotificationService && Services.NotificationService.doNotDisturb) {
            return Theme.ThemeEngine.icons.notificationsOff
        }
        return Theme.ThemeEngine.icons.notifications
    }

    spacing: 1
    layoutDirection: Qt.LeftToRight

    Molecules.WorkspaceIndicator {
        visible: Services.BarController && Services.BarController.showWorkspaceIndicator
        Layout.alignment: Qt.AlignVCenter
    }

    RowLayout {
        id: trayContainer
        visible: Services.BarController
                 && Services.BarController.showSystemTray
                 && Services.SystemTrayService
                 && Services.SystemTrayService.trayItemCount > 0
        spacing: 0

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
    }

    StatusBarModule.MenuBarButton {
        leadingIcon: Theme.ThemeEngine.icons.screenMirroring
        tooltip: "Displays"
        iconSize: 11
        itemHeight: 20
        onClicked: Services.BarController.toggleModule("monitor")
    }

    StatusBarModule.MenuBarButton {
        leadingIcon: Services.NotificationService && Services.NotificationService.doNotDisturb
                     ? Theme.ThemeEngine.icons.nightLight : Theme.ThemeEngine.icons.focus
        tooltip: Services.NotificationService && Services.NotificationService.doNotDisturb
                 ? "Disable Do Not Disturb" : "Enable Do Not Disturb"
        active: Services.NotificationService && Services.NotificationService.doNotDisturb
        iconSize: 11
        itemHeight: 20
        onClicked: {
            if (Services.NotificationService) {
                Services.NotificationService.toggleDoNotDisturb()
            }
        }
    }

    StatusBarModule.MenuBarButton {
        visible: Services.BluetoothService
                 && (Services.BluetoothService.isPowered || Services.BluetoothService.hasConnectedDevice)
        leadingIcon: root.bluetoothIcon
        tooltip: Services.BluetoothService && Services.BluetoothService.hasConnectedDevice
                 ? "Bluetooth connected" : "Bluetooth"
        iconSize: 11
        itemHeight: 20
        active: Services.BluetoothService && Services.BluetoothService.hasConnectedDevice
        onClicked: Services.BarController.toggleModule("quicksettings")
    }

    StatusBarModule.MenuBarButton {
        visible: Services.BarController && Services.BarController.showNetworkIndicator
        leadingIcon: root.networkIcon
        tooltip: Services.NetworkService && Services.NetworkService.isConnected
                 ? Services.NetworkService.ssid || "Connected" : "Network"
        iconSize: 11
        itemHeight: 20
        active: Services.NetworkService && Services.NetworkService.isConnected
        onClicked: Services.BarController.toggleModule("quicksettings")
    }

    StatusBarModule.MenuBarButton {
        visible: Services.BarController && Services.BarController.showBrightnessIndicator
        leadingIcon: Theme.ThemeEngine.icons.brightness
        tooltip: "Brightness " + root.brightnessLabel
        iconSize: 11
        itemHeight: 20
        onClicked: Services.BarController.toggleModule("quicksettings")
        onWheel: function(wheel) {
            if (!Services.BrightnessService) {
                return
            }
            const delta = wheel.angleDelta.y > 0 ? 5 : -5
            const newValue = Math.max(5, Math.min(100, Services.BrightnessService.primaryBrightness + delta))
            Services.BrightnessService.setAllBrightness(newValue)
            wheel.accepted = true
        }
    }

    StatusBarModule.MenuBarButton {
        visible: Services.BarController && Services.BarController.showVolumeIndicator
        leadingIcon: root.volumeIcon
        tooltip: "Volume " + root.volumeLabel
        iconSize: 11
        itemHeight: 20
        onClicked: Services.BarController.toggleModule("audio")
        onWheel: function(wheel) {
            if (!Services.AudioService || !Services.AudioService.canSetVolume) {
                return
            }
            const step = wheel.angleDelta.y > 0 ? 0.05 : -0.05
            const newValue = Math.max(0.0, Math.min(1.0, Services.AudioService.volume + step))
            Services.AudioService.setVolume(newValue)
            wheel.accepted = true
        }
    }

    StatusBarModule.MenuBarButton {
        visible: Services.BarController && Services.BarController.showBatteryIndicator
        leadingIcon: root.batteryIcon
        label: root.batteryLabel
        foregroundColor: root.batteryColor
        textSize: 11
        tooltip: Services.BatteryService && Services.BatteryService.isCharging
                 ? "Battery charging" : "Battery"
        iconSize: 11
        itemHeight: 20
        onClicked: Services.BarController.toggleModule("monitor")
    }

    Item {
        implicitWidth: notificationButton.implicitWidth
        implicitHeight: notificationButton.implicitHeight

        StatusBarModule.MenuBarButton {
            id: notificationButton
            anchors.fill: parent
            leadingIcon: root.notificationIcon
            tooltip: Services.NotificationService && Services.NotificationService.unreadCount > 0
                     ? String(Services.NotificationService.unreadCount) + " unread notifications"
                     : "Notifications"
            active: Services.BarController && Services.BarController.notificationsVisible
            iconSize: 11
            itemHeight: 20
            onClicked: Services.BarController.toggleModule("notifications")
        }

        Atoms.Badge {
            visible: Services.NotificationService && Services.NotificationService.unreadCount > 0
            text: Services.NotificationService.unreadCount > 9 ? "9+" : String(Services.NotificationService.unreadCount)
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: 1
            anchors.rightMargin: 0
            size: "small"
        }
    }

    StatusBarModule.MenuBarButton {
        leadingIcon: Theme.ThemeEngine.icons.settings
        tooltip: "Control Center"
        active: Services.BarController && Services.BarController.quickSettingsVisible
        iconSize: 11
        itemHeight: 20
        onClicked: Services.BarController.toggleModule("quicksettings")
    }

    StatusBarModule.MenuBarButton {
        visible: Services.BarController && Services.BarController.showClock
        label: root.clockText
        tooltip: "Open calendar and weather"
        textWeight: Theme.ThemeEngine.typography.weightSemiBold
        textSize: 11
        horizontalPadding: 8
        itemHeight: 20
        onClicked: Services.BarController.toggleModule("weather")
    }

    readonly property string clockText: {
        const now = clock.date
        const days = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"]
        const months = ["Jan", "Feb", "Mar", "Apr", "May", "Jun",
                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"]
        return days[now.getDay()] + " "
             + months[now.getMonth()] + " "
             + now.getDate() + " "
             + (now.getHours() < 10 ? "0" : "") + now.getHours()
             + ":" + (now.getMinutes() < 10 ? "0" : "") + now.getMinutes()
    }

    SystemClock {
        id: clock
        precision: SystemClock.Minutes
    }

    Component.onCompleted: {
        if (Services.BatteryService) {
            Services.BatteryService.refresh()
        }
        if (Services.NetworkService) {
            Services.NetworkService.refresh()
        }
        if (Services.BrightnessService) {
            Services.BrightnessService.refresh()
        }
        if (Services.BluetoothService) {
            Services.BluetoothService.refresh()
        }
    }
}
