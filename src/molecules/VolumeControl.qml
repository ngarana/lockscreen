// VolumeControl.qml - Volume Slider + Icon Molecule
//
// Combines volume icon and slider with mute button.
// Integrates with AudioService for system volume control.
//
// Usage:
//   VolumeControl {
//       width: 200
//       showIcon: true
//   }
//
// Properties:
//   - value: real - Current volume 0.0-1.0
//   - showIcon: bool - Show volume icon (default: true)
//   - showMuteButton: bool - Show mute button (default: true)
//   - orientation: int - Qt.Horizontal or Qt.Vertical
//
// Signals:
//   - valueChanged(value): Emitted when volume changes
//   - muteToggled(): Emitted when mute is toggled

import QtQuick
import QtQuick.Layouts
import "../atoms" as Atoms
import "../services" as Services
import "../theme" as Theme

RowLayout {
    id: root

    // Public API
    property real value: Services.AudioService.volume
    property bool showIcon: true
    property bool showMuteButton: true
    property int orientation: Qt.Horizontal

    signal valueChanged(real value)
    signal muteToggled()

    // Internal
    property bool _isMuted: value <= 0

    // Layout
    spacing: 8
    layoutDirection: root.orientation === Qt.Horizontal ? Qt.LeftToRight : Qt.TopToBottom

    // Mute button
    Atoms.IconButton {
        id: muteButton
        visible: root.showMuteButton
        size: 32
        iconSize: 18
        icon: root._getVolumeIcon()
        tooltip: root._isMuted ? "Unmute" : "Mute"
        circular: true

        onClicked: {
            root._isMuted = !root._isMuted
            Services.AudioService.setVolume(root._isMuted ? 0 : 0.5)
            root.muteToggled()
        }
    }

    // Volume icon (when no mute button)
    Atoms.Icon {
        id: volumeIcon
        visible: root.showIcon && !root.showMuteButton
        source: root._getVolumeIcon()
        size: 20
        color: Theme.ThemeEngine.colors.textSecondary
    }

    // Volume slider
    Atoms.Slider {
        id: slider
        Layout.fillWidth: root.orientation === Qt.Horizontal
        Layout.fillHeight: root.orientation === Qt.Vertical
        implicitWidth: root.orientation === Qt.Horizontal ? 120 : 24
        implicitHeight: root.orientation === Qt.Vertical ? 120 : 24

        from: 0
        to: 1
        value: root.value
        stepSize: 0.05

        onValueChanged: {
            if (Math.abs(value - root.value) > 0.001) {
                root.value = value
                Services.AudioService.setVolume(value)
                root.valueChanged(value)
            }
        }

        onMoved: {
            root.value = value
            Services.AudioService.setVolume(value)
            root.valueChanged(value)
        }
    }

    // Value label (optional)
    Atoms.Label {
        id: valueLabel
        visible: false // Set to true to show percentage
        text: Math.round(root.value * 100) + "%"
        fontSize: Theme.ThemeEngine.typography.sizeSm
        color: Theme.ThemeEngine.colors.textSecondary
    }

    // Update when service changes
    Connections {
        target: Services.AudioService
        function onVolumeChanged() {
            root.value = Services.AudioService.volume
        }
    }

    // Get appropriate volume icon
    function _getVolumeIcon() {
        if (root._isMuted || root.value <= 0) return "volume-off"
        if (root.value < 0.3) return "volume-low"
        if (root.value < 0.7) return "volume-medium"
        return "volume-high"
    }
}
