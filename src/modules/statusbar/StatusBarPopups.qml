// StatusBarPopups.qml - Popup Windows for Status Bar Components
//
// Contains FloatingWindow instances for Quick Settings, Weather,
// and System Monitor, triggered from the status bar.

import QtQuick
import Quickshell
import Quickshell.Wayland
import "../../components" as Components
import "../../services" as Services
import "../../modules/statusbar" as BarModule
import "../../theme" as Theme

Item {
    id: root

    required property var screen

    // ========================================================================
    // Quick Settings Popup (Control Center)
    // ========================================================================

    PanelWindow {
        id: quickSettingsWindow
        screen: root.screen
        visible: BarModule.BarController.quickSettingsVisible
        
        anchors {
            top: true
            right: true
        }

        implicitWidth: 320
        implicitHeight: quickSettingsContent.implicitHeight

        Components.QuickSettings {
            id: quickSettingsContent
            anchors.fill: parent
            
            // Wire up to actual services
            wifiEnabled: (Services.NetworkService && Services.NetworkService.wifiEnabled) || false
            bluetoothEnabled: (Services.BluetoothService && Services.BluetoothService.isPowered) || false
            volume: (Services.AudioService && Services.AudioService.volume) || 0
            brightness: (Services.BrightnessService && Services.BrightnessService.brightness) || 0
            
            onVolumeChanged: (val) => Services.AudioService.setVolume(val)
            onBrightnessChanged: (val) => Services.BrightnessService.setBrightness(val)
            onWifiToggled: (enabled) => Services.NetworkService.setEnabled(enabled)
            onBluetoothToggled: (enabled) => Services.BluetoothService.setEnabled(enabled)
        }
    }

    // ========================================================================
    // Weather Popup
    // ========================================================================

    PanelWindow {
        id: weatherWindow
        screen: root.screen
        visible: BarModule.BarController.weatherVisible
        
        anchors {
            top: true
            right: true
        }

        implicitWidth: 280
        implicitHeight: weatherContent.implicitHeight

        Components.WeatherWidget {
            id: weatherContent
            anchors.fill: parent
            location: "San Francisco" // Placeholder
            temperature: 18
            condition: "Partly Cloudy"
            icon: "⛅"
            showForecast: true
            forecastData: [
                {day: "Tue", icon: "☀️", high: 22, low: 14},
                {day: "Wed", icon: "☁️", high: 19, low: 13},
                {day: "Thu", icon: "🌧️", high: 16, low: 11}
            ]
        }
    }

    // ========================================================================
    // System Monitor Popup
    // ========================================================================

    PanelWindow {
        id: monitorWindow
        screen: root.screen
        visible: BarModule.BarController.systemMonitorVisible
        
        anchors {
            top: true
            right: true
        }

        implicitWidth: 300
        implicitHeight: monitorContent.implicitHeight

        Components.SystemMonitor {
            id: monitorContent
            anchors.fill: parent
        }
    }
}
