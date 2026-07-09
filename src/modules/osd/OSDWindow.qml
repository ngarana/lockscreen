// OSDWindow.qml - OSD Floating Window
//
// Displays OSD popups for volume, brightness, and microphone.
// Auto-hides after timeout, positioned at top-right of screen.

import QtQuick
import Quickshell
import Quickshell.Wayland
import "." as OSDModule
import "../../services" as Services
import "../../theme" as Theme

FloatingWindow {
    id: root

    required property var screen

    visible: OSDModule.OSDController.visible
    screen: root.screen

    width: content.implicitWidth
    height: content.implicitHeight

    // Position at top-right with margin
    x: screen.width - width - 20
    y: 80

    color: "transparent"

    // Slide in/out animation
    property real offsetX: visible ? 0 : width + 30

    Behavior on offsetX {
        NumberAnimation {
            duration: Theme.ThemeEngine.animation.medium
            easing.type: Easing.OutCubic
        }
    }

    transform: Translate {
        x: root.offsetX
    }

    OSDModule.OSDContent {
        id: content
        anchors.centerIn: parent

        osdType: OSDModule.OSDController.currentType
        value: {
            if (osdType === "volume") return OSDModule.OSDController.volume
            if (osdType === "brightness") return OSDModule.OSDController.brightness / 100
            if (osdType === "microphone") return OSDModule.OSDController.microphone
            return 0
        }
        muted: {
            if (osdType === "volume") return OSDModule.OSDController.volumeMuted
            if (osdType === "microphone") return OSDModule.OSDController.microphoneMuted
            return false
        }
        maxValue: osdType === "brightness" ? 1 : 1

        // Allow scroll wheel to adjust values
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            onWheel: function(wheel) {
                if (content.osdType === "volume") {
                    if (wheel.angleDelta.y > 0) {
                        Services.AudioService.setVolume(Math.min(1.0, Services.AudioService.volume + 0.05))
                    } else {
                        Services.AudioService.setVolume(Math.max(0.0, Services.AudioService.volume - 0.05))
                    }
                } else if (content.osdType === "brightness") {
                    if (wheel.angleDelta.y > 0) {
                        Services.BrightnessService.increase()
                    } else {
                        Services.BrightnessService.decrease()
                    }
                }
                wheel.accepted = true
            }
        }
    }

    // Connect to services
    Connections {
        target: Services.AudioService
        function onVolumeChanged() {
            OSDModule.OSDController.volume = Services.AudioService.volume
            OSDModule.OSDController.volumeMuted = false
            OSDModule.OSDController.show("volume")
        }
    }

    Connections {
        target: Services.BrightnessService
        function onBrightnessChanged(display, value) {
            OSDModule.OSDController.brightness = value
            OSDModule.OSDController.show("brightness")
        }
    }
}
