// StatusBarPopups.qml - Popup windows for status bar interactions
//
// Hosts the control center, calendar/weather panel, notifications,
// and audio popovers used by the status bar.

import QtQuick
import QtQuick.Layouts
import Quickshell
import Quickshell.Wayland
import "../../atoms" as Atoms
import "../../components" as Components
import "../../services" as Services
import "../../modules/statusbar" as BarModule
import "../../theme" as Theme

Item {
    id: root

    required property var screen

    readonly property int topOffset: BarModule.BarController.barHeight + 8
    readonly property int rightMargin: 12

    PanelWindow {
        id: quickSettingsWindow
        screen: root.screen
        visible: BarModule.BarController.quickSettingsVisible

        anchors {
            top: true
            right: true
        }

        margins {
            top: root.topOffset
            right: root.rightMargin
        }

        implicitWidth: 340
        implicitHeight: quickSettingsContent.implicitHeight

        Components.QuickSettings {
            id: quickSettingsContent
            anchors.fill: parent
            wifiEnabled: (Services.NetworkService && Services.NetworkService.wifiEnabled) || false
            bluetoothEnabled: (Services.BluetoothService && Services.BluetoothService.isPowered) || false
            dndEnabled: (Services.NotificationService && Services.NotificationService.doNotDisturb) || false
            volume: (Services.AudioService && Services.AudioService.volume) || 0
            brightness: (Services.BrightnessService && Services.BrightnessService.primaryBrightness) || 0
            onVolumeChanged: function(value) {
                if (Services.AudioService) {
                    Services.AudioService.setVolume(value)
                }
            }
            onBrightnessChanged: function(value) {
                if (Services.BrightnessService) {
                    Services.BrightnessService.setAllBrightness(value)
                }
            }
            onWifiToggled: function(enabled) {
                if (Services.NetworkService) {
                    if (enabled) {
                        Services.NetworkService.scanNetworks()
                    } else {
                        Services.NetworkService.disconnect()
                    }
                }
            }
            onBluetoothToggled: function(enabled) {
                if (Services.BluetoothService) {
                    if (enabled) {
                        Services.BluetoothService.powerOn()
                    } else {
                        Services.BluetoothService.powerOff()
                    }
                }
            }
            onDndToggled: function(enabled) {
                if (Services.NotificationService) {
                    Services.NotificationService.setDoNotDisturb(enabled)
                }
            }
        }
    }

    PanelWindow {
        id: weatherWindow
        screen: root.screen
        visible: BarModule.BarController.weatherVisible

        anchors {
            top: true
            right: true
        }

        margins {
            top: root.topOffset
            right: root.rightMargin
        }

        implicitWidth: 360
        implicitHeight: weatherContent.implicitHeight

        Item {
            id: weatherContent
            implicitWidth: 360
            implicitHeight: weatherColumn.implicitHeight

            ColumnLayout {
                id: weatherColumn
                width: parent.width
                spacing: 10

                Components.Calendar {
                    Layout.fillWidth: true
                    implicitWidth: 360
                    implicitHeight: 336
                    showYearNavigation: false
                }

                Components.WeatherWidget {
                    Layout.fillWidth: true
                    implicitWidth: 360
                    location: "Weather"
                    temperature: 18
                    condition: "Forecast unavailable"
                    icon: "⛅"
                    humidity: 0
                    windSpeed: 0
                    showForecast: false
                }
            }
        }
    }

    PanelWindow {
        id: notificationsWindow
        screen: root.screen
        visible: BarModule.BarController.notificationsVisible

        anchors {
            top: true
            right: true
        }

        margins {
            top: root.topOffset
            right: root.rightMargin
        }

        implicitWidth: 360
        implicitHeight: notificationsPanel.implicitHeight

        Components.GlassPanel {
            id: notificationsPanel
            implicitWidth: 360
            implicitHeight: Math.min(520, notificationsColumn.implicitHeight + 24)
            radius: Theme.ThemeEngine.radius.large
            backgroundColor: Theme.ThemeEngine.colors.glass
            borderColor: Theme.ThemeEngine.colors.glassBorder
            elevation: 2

            ColumnLayout {
                id: notificationsColumn
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true

                    Atoms.Label {
                        text: "Notifications"
                        fontSize: Theme.ThemeEngine.typography.sizeMd
                        fontWeight: Theme.ThemeEngine.typography.weightBold
                        color: Theme.ThemeEngine.colors.textPrimary
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Atoms.TextButton {
                        text: "Clear"
                        variant: "secondary"
                        size: "small"
                        enabled: Services.NotificationService
                                 && Services.NotificationService.notificationHistory.length > 0
                        onClicked: {
                            if (Services.NotificationService) {
                                Services.NotificationService.clearAll()
                            }
                        }
                    }
                }

                Atoms.Label {
                    visible: Services.NotificationService
                             && Services.NotificationService.notificationHistory.length === 0
                    text: "You're all caught up."
                    fontSize: Theme.ThemeEngine.typography.sizeSm
                    color: Theme.ThemeEngine.colors.textSecondary
                }

                Flickable {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Math.min(contentHeight, 430)
                    contentWidth: width
                    contentHeight: notificationsList.implicitHeight
                    clip: true
                    visible: Services.NotificationService
                             && Services.NotificationService.notificationHistory.length > 0

                    Column {
                        id: notificationsList
                        width: parent.width
                        spacing: 8

                        Repeater {
                            model: Services.NotificationService
                                   ? Services.NotificationService.notificationHistory.slice().reverse()
                                   : []

                            Rectangle {
                                width: notificationsList.width
                                implicitHeight: notificationColumn.implicitHeight + 16
                                radius: Theme.ThemeEngine.radius.medium
                                color: Qt.alpha(Theme.ThemeEngine.colors.surface0, 0.68)
                                border.width: 1
                                border.color: Qt.alpha(Theme.ThemeEngine.colors.glassBorder, 0.9)

                                ColumnLayout {
                                    id: notificationColumn
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 6

                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 8

                                        Atoms.Icon {
                                            icon: modelData && modelData.icon ? modelData.icon : Theme.ThemeEngine.icons.notifications
                                            size: 16
                                            color: Theme.ThemeEngine.colors.textSecondary
                                        }

                                        Atoms.Label {
                                            Layout.fillWidth: true
                                            text: modelData && modelData.appName ? modelData.appName : "Notification"
                                            fontSize: Theme.ThemeEngine.typography.sizeXs
                                            color: Theme.ThemeEngine.colors.textSecondary
                                            truncate: true
                                        }
                                    }

                                    Atoms.Label {
                                        Layout.fillWidth: true
                                        text: modelData && modelData.title ? modelData.title : ""
                                        visible: text !== ""
                                        fontSize: Theme.ThemeEngine.typography.sizeSm
                                        fontWeight: Theme.ThemeEngine.typography.weightSemiBold
                                        color: Theme.ThemeEngine.colors.textPrimary
                                        wrap: true
                                        maxLines: 2
                                    }

                                    Atoms.Label {
                                        Layout.fillWidth: true
                                        text: modelData && modelData.body ? modelData.body : ""
                                        visible: text !== ""
                                        fontSize: Theme.ThemeEngine.typography.sizeSm
                                        color: Theme.ThemeEngine.colors.textSecondary
                                        wrap: true
                                        maxLines: 4
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    PanelWindow {
        id: audioWindow
        screen: root.screen
        visible: BarModule.BarController.audioControllerVisible

        anchors {
            top: true
            right: true
        }

        margins {
            top: root.topOffset
            right: root.rightMargin
        }

        implicitWidth: 300
        implicitHeight: audioPanel.implicitHeight

        Components.GlassPanel {
            id: audioPanel
            implicitWidth: 300
            implicitHeight: audioColumn.implicitHeight + 24
            radius: Theme.ThemeEngine.radius.large
            backgroundColor: Theme.ThemeEngine.colors.glass
            borderColor: Theme.ThemeEngine.colors.glassBorder
            elevation: 2

            ColumnLayout {
                id: audioColumn
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                Atoms.Label {
                    text: "Sound"
                    fontSize: Theme.ThemeEngine.typography.sizeMd
                    fontWeight: Theme.ThemeEngine.typography.weightBold
                    color: Theme.ThemeEngine.colors.textPrimary
                }

                Atoms.Label {
                    text: Services.AudioService && Services.AudioService.hasMetadata
                          ? Services.AudioService.title : "System volume"
                    fontSize: Theme.ThemeEngine.typography.sizeSm
                    fontWeight: Theme.ThemeEngine.typography.weightMedium
                    color: Theme.ThemeEngine.colors.textPrimary
                    wrap: true
                    maxLines: 2
                    visible: text !== ""
                }

                Atoms.Label {
                    text: Services.AudioService ? Services.AudioService.artist : ""
                    fontSize: Theme.ThemeEngine.typography.sizeSm
                    color: Theme.ThemeEngine.colors.textSecondary
                    visible: text !== ""
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Atoms.Icon {
                        icon: Services.AudioService && Services.AudioService.volume <= 0.05
                              ? Theme.ThemeEngine.icons.volumeMuted
                              : Services.AudioService && Services.AudioService.volume < 0.66
                                ? Theme.ThemeEngine.icons.volumeMedium
                                : Theme.ThemeEngine.icons.volumeHigh
                        size: 18
                        color: Theme.ThemeEngine.colors.textPrimary
                    }

                    Atoms.Slider {
                        Layout.fillWidth: true
                        from: 0
                        to: 1
                        stepSize: 0.01
                        value: (Services.AudioService && Services.AudioService.volume) || 0
                        onMoved: {
                            if (Services.AudioService) {
                                Services.AudioService.setVolume(value)
                            }
                        }
                    }

                    Atoms.Label {
                        text: String(Math.round(((Services.AudioService && Services.AudioService.volume) || 0) * 100)) + "%"
                        fontSize: Theme.ThemeEngine.typography.sizeSm
                        color: Theme.ThemeEngine.colors.textSecondary
                    }
                }
            }
        }
    }

    PanelWindow {
        id: monitorWindow
        screen: root.screen
        visible: BarModule.BarController.systemMonitorVisible

        anchors {
            top: true
            right: true
        }

        margins {
            top: root.topOffset
            right: root.rightMargin
        }

        implicitWidth: 300
        implicitHeight: monitorContent.implicitHeight

        Components.SystemMonitor {
            id: monitorContent
            anchors.fill: parent
        }
    }
}
