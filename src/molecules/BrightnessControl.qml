// BrightnessControl.qml - Brightness Slider + Icon Molecule
//
// Combines brightness icon and slider for display control.
// Integrates with BrightnessService.
//
// Usage:
//   BrightnessControl {
//       width: 200
//   }
//
// Properties:
//   - value: int - Current brightness 0-100
//   - showIcon: bool - Show brightness icon (default: true)
//   - orientation: int - Qt.Horizontal or Qt.Vertical
//
// Signals:
//   - valueChanged(value): Emitted when brightness changes

import QtQuick
import QtQuick.Layouts
import "../atoms" as Atoms
import "../services" as Services
import "../theme" as Theme

RowLayout {
    id: root

    // Public API
    property int value: Services.BrightnessService.primaryBrightness
    property bool showIcon: true
    property int orientation: Qt.Horizontal

    signal valueChanged(int value)

    // Layout
    spacing: 8
    layoutDirection: root.orientation === Qt.Horizontal ? Qt.LeftToRight : Qt.TopToBottom

    // Brightness icon
    Atoms.Icon {
        id: brightnessIcon
        visible: root.showIcon
        source: root._getBrightnessIcon()
        size: 20
        color: Theme.ThemeEngine.colors.textSecondary
    }

    // Brightness slider
    Atoms.Slider {
        id: slider
        Layout.fillWidth: root.orientation === Qt.Horizontal
        Layout.fillHeight: root.orientation === Qt.Vertical
        implicitWidth: root.orientation === Qt.Horizontal ? 120 : 24
        implicitHeight: root.orientation === Qt.Vertical ? 120 : 24

        from: 5 // Minimum brightness to prevent black screen
        to: 100
        value: root.value
        stepSize: 5

        onValueChanged: {
            if (value !== root.value) {
                root.value = value
                Services.BrightnessService.setAllBrightness(value)
                root.valueChanged(value)
            }
        }

        onMoved: {
            root.value = value
            Services.BrightnessService.setAllBrightness(value)
            root.valueChanged(value)
        }
    }

    // Update when service changes
    Connections {
        target: Services.BrightnessService
        function onBrightnessChanged() {
            root.value = Services.BrightnessService.primaryBrightness
        }
    }

    // Get appropriate brightness icon
    function _getBrightnessIcon() {
        if (root.value <= 20) return "display-brightness-low"
        if (root.value <= 60) return "display-brightness-medium"
        return "display-brightness-high"
    }
}
