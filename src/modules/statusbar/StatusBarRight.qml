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
        visible: Services.BarController.showWorkspaceIndicator
        Layout.alignment: Qt.AlignVCenter
    }

    // ========================================================================
    // System Tray
    // ========================================================================

    RowLayout {
        id: trayContainer
        visible: Services.BarController.showSystemTray && Services.SystemTrayService.trayItemCount > 0
        spacing: 12

        Repeater {
            id: trayRepeater
            model: Services.SystemTrayService.trayItems

            StatusBarModule.TrayItem {
                itemId: modelData.id
                itemIcon: Services.SystemTrayService.getItemIcon(modelData.id)
                itemTooltip: Services.SystemTrayService.getItemTooltip(modelData.id)
                needsAttention: Services.SystemTrayService.needsAttention(modelData.id)

                Layout.preferredWidth: 18
                Layout.preferredHeight: 18
            }
        }

        Connections {
            target: Services.SystemTrayService
            function onTrayItemsChanged() {
                trayRepeater.model = Services.SystemTrayService.trayItems
            }
        }
    }

    // ========================================================================
    // Supplementary macOS Icons
    // ========================================================================

    RowLayout {
        spacing: 12
        Layout.alignment: Qt.AlignVCenter

        // Screen Mirroring Icon
        Text {
            text: Theme.ThemeEngine.icons.screenMirroring
            font.pixelSize: 14
            font.family: Theme.ThemeEngine.fonts.iconFontFamily
            color: Theme.ThemeEngine.colors.primary
        }

        // Display Icon
        Text {
            text: Theme.ThemeEngine.icons.display
            font.pixelSize: 14
            font.family: Theme.ThemeEngine.fonts.iconFontFamily
            color: Theme.ThemeEngine.colors.textPrimary
        }

        // Focus Mode Icon
        Text {
            text: Theme.ThemeEngine.icons.focus
            font.pixelSize: 14
            font.family: Theme.ThemeEngine.fonts.iconFontFamily
            color: Theme.ThemeEngine.colors.textPrimary
        }

        // Grid View Icon
        Text {
            text: Theme.ThemeEngine.icons.grid
            font.pixelSize: 14
            font.family: Theme.ThemeEngine.fonts.iconFontFamily
            color: Theme.ThemeEngine.colors.textPrimary
        }
    }

    // ========================================================================
    // Brightness Indicator
    // ========================================================================

    Text {
        visible: Services.BarController.showBrightnessIndicator
        text: Theme.ThemeEngine.icons.brightness
        font.pixelSize: 14
        font.family: Theme.ThemeEngine.fonts.iconFontFamily
        color: Theme.ThemeEngine.colors.textPrimary
        Layout.alignment: Qt.AlignVCenter

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onWheel: function(wheel) {
                const delta = wheel.angleDelta.y > 0 ? 0.05 : -0.05
                Services.BrightnessService.setBrightness(Math.max(0, Math.min(1, Services.BrightnessService.brightness + delta)))
            }
        }
    }

    // ========================================================================
    // Volume Indicator
    // ========================================================================

    Text {
        visible: Services.BarController.showVolumeIndicator
        text: root._volumeIcon
        font.pixelSize: 14
        font.family: Theme.ThemeEngine.fonts.iconFontFamily
        color: Theme.ThemeEngine.colors.textPrimary
        Layout.alignment: Qt.AlignVCenter

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onWheel: function(wheel) {
                const delta = wheel.angleDelta.y > 0 ? 0.05 : -0.05
                Services.AudioService.setVolume(Math.max(0, Math.min(1, Services.AudioService.volume + delta)))
            }
        }
    }

    // ========================================================================
    // Network Indicator
    // ========================================================================

    Text {
        text: root._networkIcon
        visible: Services.BarController.showNetworkIndicator
        font.pixelSize: 14
        font.family: Theme.ThemeEngine.fonts.iconFontFamily
        color: Theme.ThemeEngine.colors.textPrimary
        Layout.alignment: Qt.AlignVCenter
    }

    // ========================================================================
    // Battery Indicator (with percentage text)
    // ========================================================================

    RowLayout {
        spacing: 4
        visible: Services.BarController.showBatteryIndicator && Services.BatteryService.percentage > 0
        Layout.alignment: Qt.AlignVCenter

        Atoms.Label {
            text: Services.BatteryService.percentage + "%"
            fontSize: Theme.ThemeEngine.typography.sizeSm
            color: Theme.ThemeEngine.colors.textPrimary
            fontWeight: Theme.ThemeEngine.typography.weightMedium
            Layout.alignment: Qt.AlignVCenter
        }
        
        Text {
            text: root._batteryIcon
            font.pixelSize: 14
            font.family: Theme.ThemeEngine.fonts.iconFontFamily
            color: Theme.ThemeEngine.colors.textPrimary
            Layout.alignment: Qt.AlignVCenter
        }
    }

    // ========================================================================
    // Notification Center Toggle
    // ========================================================================

    Item {
        Layout.preferredWidth: 20
        Layout.preferredHeight: 20
        Layout.alignment: Qt.AlignVCenter

        Text {
            anchors.centerIn: parent
            text: Theme.ThemeEngine.icons.notifications
            font.pixelSize: 14
            font.family: Theme.ThemeEngine.fonts.iconFontFamily
            color: Theme.ThemeEngine.colors.textPrimary
        }

        // Badge for unread notifications
        Rectangle {
            visible: Services.NotificationService.unreadCount > 0
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: -2
            anchors.rightMargin: -2
            width: 10
            height: 10
            radius: 5
            color: Theme.ThemeEngine.colors.error

            Text {
                anchors.centerIn: parent
                text: Services.NotificationService.unreadCount > 9 ? "9+" : Services.NotificationService.unreadCount
                font.pixelSize: 6
                font.weight: Font.Bold
                color: Theme.ThemeEngine.colors.background
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                // Toggle notification center
            }
        }
    }

    // ========================================================================
    // Control Center Toggle
    // ========================================================================

    Item {
        Layout.preferredWidth: 20
        Layout.preferredHeight: 20
        Layout.alignment: Qt.AlignVCenter
        
        Text {
            anchors.centerIn: parent
            text: Theme.ThemeEngine.icons.settings
            font.pixelSize: 14
            font.family: Theme.ThemeEngine.fonts.iconFontFamily
            color: Theme.ThemeEngine.colors.textPrimary
        }
        
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                // Toggle control center
            }
        }
    }

    // ========================================================================
    // Clock Display
    // ========================================================================

    Atoms.Label {
        text: root._timeText
        visible: Services.BarController.showClock
        fontSize: Theme.ThemeEngine.typography.sizeSm
        fontWeight: Theme.ThemeEngine.typography.weightMedium
        color: Theme.ThemeEngine.colors.textPrimary
        Layout.alignment: Qt.AlignVCenter
    }

    // ========================================================================
    // Internal Properties
    // ========================================================================

    property string _timeText: ""

    readonly property string _networkIcon: {
        if (Services.NetworkService.isConnected) {
            return Services.NetworkService.connectionType === "wifi"
                ? Theme.ThemeEngine.icons.wifi
                : Theme.ThemeEngine.icons.ethernet
        }
        return Theme.ThemeEngine.icons.wifiOff
    }

    readonly property string _batteryIcon: {
        if (Services.BatteryService.isCharging) {
            return Theme.ThemeEngine.icons.batteryCharging
        }
        return Theme.ThemeEngine.icons.battery
    }

    readonly property string _volumeIcon: {
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
        function onVolumeChanged() {} // Force update
    }

    Connections {
        target: Services.BrightnessService
        function onBrightnessChanged() {} // Force update
    }

    Connections {
        target: Services.NetworkService
        function onNetworkStatusChanged() {} // Force update
    }

    Connections {
        target: Services.BatteryService
        function onBatteryLevelChanged() {} // Force update
        function onChargingStatusChanged() {} // Force update
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        _updateClock()
    }
}
