// OSDController.qml - OSD State Management
//
// Singleton for managing OSD visibility and state.
// Handles auto-hide timers and tracks current OSD type.

pragma Singleton
import QtQuick

QtObject {
    id: root

    // Visibility state
    property bool visible: false

    // Current OSD type: "volume", "brightness", "microphone"
    property string currentType: "volume"

    // Auto-hide delay in milliseconds
    property int hideDelay: 2000

    // Current values
    property real volume: 0
    property bool volumeMuted: false
    property real brightness: 0
    property real microphone: 0
    property bool microphoneMuted: false

    // Show OSD with specific type
    function show(type) {
        currentType = type
        visible = true
        hideTimer.restart()
    }

    // Hide OSD
    function hide() {
        visible = false
    }

    // Internal timer
    property var hideTimer: Timer {
        id: hideTimer
        interval: root.hideDelay
        onTriggered: root.hide()
    }
}
