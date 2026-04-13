// QuickSettings.qml - Quick Settings Toggle Grid Component
//
// A glassmorphic panel with quick toggle buttons for common
// system settings like WiFi, Bluetooth, DND, and sliders
// for volume and brightness.
//
// Properties:
//   - wifiEnabled: bool - WiFi toggle state
//   - bluetoothEnabled: bool - Bluetooth toggle state
//   - dndEnabled: bool - Do Not Disturb toggle state
//   - airplaneMode: bool - Airplane mode state
//   - volume: real - Current volume (0-1)
//   - brightness: real - Current brightness (0-1)
//   - showSliders: bool - Show volume/brightness sliders
//
// Signals:
//   - wifiToggled(enabled: bool)
//   - bluetoothToggled(enabled: bool)
//   - dndToggled(enabled: bool)
//   - airplaneModeToggled(enabled: bool)
//   - volumeChanged(value: real)
//   - brightnessChanged(value: real)

import QtQuick
import QtQuick.Layouts
import "../services" as Services
import "../atoms" as Atoms
import "../molecules" as Molecules
import "../theme" as Theme

Item {
    id: root

    implicitWidth: 320
    implicitHeight: showSliders ? 280 : 180

    // ====== Public Properties ======

    property bool wifiEnabled: false
    property bool bluetoothEnabled: false
    property bool dndEnabled: false
    property bool airplaneMode: false
    property bool nightLight: false

    property real volume: 0.5
    property real brightness: 0.8
    property bool showSliders: true

    // Service availability
    property bool wifiAvailable: true
    property bool bluetoothAvailable: true

    // Signals
    signal wifiToggled(bool enabled)
    signal bluetoothToggled(bool enabled)
    signal dndToggled(bool enabled)
    signal airplaneModeToggled(bool enabled)
    signal nightLightToggled(bool enabled)
    // signal volumeChanged(real value) - Implicit
    // signal brightnessChanged(real value) - Implicit

    // ====== Glassmorphic Panel ======

    Rectangle {
        id: panel
        anchors.fill: parent

        color: Services.Theme.colors.glass
        radius: Services.Theme.radius.large
        border.width: 1
        border.color: Services.Theme.colors.glassBorder

        ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            anchors.margins: Services.Theme.spacing.medium
            spacing: Services.Theme.spacing.medium

            // ====== Toggle Grid ======

            GridLayout {
                id: toggleGrid
                Layout.fillWidth: true
                columns: 4
                rowSpacing: Services.Theme.spacing.small
                columnSpacing: Services.Theme.spacing.small

                // WiFi Toggle
                QuickToggle {
                    id: wifiToggle
                    icon: root.wifiEnabled ? "📶" : "📵"
                    label: "WiFi"
                    active: root.wifiEnabled
                    enabled: root.wifiAvailable
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onToggled: root.wifiToggled(!root.wifiEnabled)
                }

                // Bluetooth Toggle
                QuickToggle {
                    id: bluetoothToggle
                    icon: root.bluetoothEnabled ? "🔵" : "⚫"
                    label: "Bluetooth"
                    active: root.bluetoothEnabled
                    enabled: root.bluetoothAvailable
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onToggled: root.bluetoothToggled(!root.bluetoothEnabled)
                }

                // DND Toggle
                QuickToggle {
                    id: dndToggle
                    icon: root.dndEnabled ? "🔇" : "🔔"
                    label: "DND"
                    active: root.dndEnabled
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onToggled: root.dndToggled(!root.dndEnabled)
                }

                // Airplane Mode Toggle
                QuickToggle {
                    id: airplaneToggle
                    icon: root.airplaneMode ? "✈️" : "✈️"
                    label: "Airplane"
                    active: root.airplaneMode
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onToggled: root.airplaneModeToggled(!root.airplaneMode)
                }

                // Night Light Toggle
                QuickToggle {
                    id: nightLightToggle
                    icon: root.nightLight ? "🌙" : "☀️"
                    label: "Night"
                    active: root.nightLight
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    onToggled: root.nightLightToggled(!root.nightLight)
                }

                // Placeholder slots for extensibility
                QuickToggle {
                    icon: "📍"
                    label: "Location"
                    active: false
                    enabled: false
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                QuickToggle {
                    icon: "⚡"
                    label: "Battery"
                    active: false
                    enabled: false
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                QuickToggle {
                    icon: "⚙️"
                    label: "Settings"
                    active: false
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }

            // ====== Sliders Section ======

            ColumnLayout {
                id: slidersSection
                visible: root.showSliders
                Layout.fillWidth: true
                spacing: Services.Theme.spacing.medium

                // Volume Slider
                RowLayout {
                    Layout.fillWidth: true
                    spacing: Services.Theme.spacing.medium

                    Text {
                        text: "🔊"
                        font.pixelSize: 16
                    }

                    Atoms.Slider {
                        id: volumeSlider
                        Layout.fillWidth: true
                        value: root.volume
                        onValueChanged: root.volume = value
                    }

                    Text {
                        text: Math.round(root.volume * 100) + "%"
                        font.pixelSize: 12
                        font.family: Services.Theme.fonts.fontFamily
                        color: Services.Theme.colors.textMuted
                        Layout.minimumWidth: 40
                        horizontalAlignment: Text.AlignRight
                    }
                }

                // Brightness Slider
                RowLayout {
                    Layout.fillWidth: true
                    spacing: Services.Theme.spacing.medium

                    Text {
                        text: "🔆"
                        font.pixelSize: 16
                    }

                    Atoms.Slider {
                        id: brightnessSlider
                        Layout.fillWidth: true
                        value: root.brightness
                        onValueChanged: root.brightness = value
                    }

                    Text {
                        text: Math.round(root.brightness * 100) + "%"
                        font.pixelSize: 12
                        font.family: Services.Theme.fonts.fontFamily
                        color: Services.Theme.colors.textMuted
                        Layout.minimumWidth: 40
                        horizontalAlignment: Text.AlignRight
                    }
                }
            }
        }
    }

    // ====== Quick Toggle Component ======

    component QuickToggle: Rectangle {
        id: toggle

        implicitWidth: 64
        implicitHeight: 64

        property string icon: ""
        property string label: ""
        property bool active: false
        property bool enabled: true

        signal toggled()

        property bool hovered: mouseArea.containsMouse

        radius: Theme.ThemeEngine.radius.medium
        color: {
            if (!enabled) return Qt.rgba(0, 0, 0, 0.2)
            if (active) return Theme.ThemeEngine.colors.glassActive
            if (hovered) return Theme.ThemeEngine.colors.glassHover
            return Theme.ThemeEngine.colors.glass
        }

        border.width: active ? 1 : 0
        border.color: Theme.ThemeEngine.colors.primary

        Behavior on color {
            ColorAnimation { duration: Theme.ThemeEngine.animation.fast }
        }

        Column {
            anchors.centerIn: parent
            spacing: 4

            Text {
                text: toggle.icon
                font.pixelSize: 24
                anchors.horizontalCenter: parent.horizontalCenter
                opacity: toggle.enabled ? 1.0 : 0.4
            }

            Text {
                text: toggle.label
                font.pixelSize: 10
                font.family: Services.Theme.fonts.fontFamily
                color: toggle.active ? Services.Theme.colors.primary : Services.Theme.colors.textMuted
                anchors.horizontalCenter: parent.horizontalCenter
                opacity: toggle.enabled ? 1.0 : 0.4
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: toggle.enabled ? Qt.PointingHandCursor : Qt.ForbiddenCursor
            onClicked: {
                if (toggle.enabled) toggle.toggled()
            }
        }
    }
}
