// AudioService.qml
// System audio service with MPRIS media player integration.
// Provides system volume control and media player management.

pragma Singleton

import Quickshell
import Quickshell.Io
import QtQuick

Singleton {
    id: root

    // System volume properties
    property real systemVolume: 1.0
    property bool systemMuted: false
    property bool volumeControlAvailable: false

    // Media player properties (stub for now)
    readonly property var players: []
    readonly property var activePlayer: null
    readonly property bool hasPlayer: false
    readonly property bool isActive: false
    readonly property bool isPlaying: false
    readonly property string title: ""
    readonly property string artist: ""
    readonly property string album: ""
    readonly property string source: ""
    readonly property string sourceLabel: ""
    readonly property bool hasMetadata: false
    readonly property real durationSeconds: 0
    readonly property real positionSeconds: 0

    // Combined volume property - prefers media player volume if available, otherwise system volume
    readonly property real volume: hasPlayer && activePlayer ? activePlayer.volume : systemVolume

    readonly property bool canControl: hasPlayer && activePlayer ? activePlayer.canControl : volumeControlAvailable
    readonly property bool canTogglePlaying: hasPlayer && activePlayer ? activePlayer.canTogglePlaying : false
    readonly property bool canGoNext: hasPlayer && activePlayer ? activePlayer.canGoNext : false
    readonly property bool canGoPrevious: hasPlayer && activePlayer ? activePlayer.canGoPrevious : false
    readonly property bool canSeek: hasPlayer && activePlayer ? activePlayer.canSeek : false
    readonly property bool canSetVolume: volumeControlAvailable || (hasPlayer && activePlayer && activePlayer.volumeSupported)

    signal playbackStateChanged(bool playing)
    signal metadataChanged()
    signal positionChanged(real positionSeconds)
    signal playerChanged(string source)

    function play() {}
    function pause() {}
    function togglePlaying() {}
    function next() {}
    function previous() {}
    function stop() {}
    function setVolume(level) {
        var clamped = Math.max(0.0, Math.min(1.0, level))

        // If we have an active media player, set its volume
        if (hasPlayer && activePlayer && activePlayer.volumeSupported) {
            activePlayer.volume = clamped
        } else {
            // Otherwise set system volume
            _setSystemVolume(clamped)
        }
    }
    function seekTo(seconds) {}

    // System volume control using pactl
    Process {
        id: getVolumeProcess
        command: ["pactl", "get-sink-volume", "@DEFAULT_SINK@"]
        stdout: StdioCollector {}

        onExited: function(code, status) {
            if (code === 0 && this.stdout.text) {
                const match = this.stdout.text.match(/(\d+)%/)
                if (match) {
                    root.systemVolume = parseInt(match[1]) / 100.0
                }
            }
        }
    }

    Process {
        id: setVolumeProcess
        stdout: StdioCollector {}

        onExited: function(code, status) {
            if (code === 0) {
                // Refresh volume after setting
                getVolumeProcess.running = false
                getVolumeProcess.running = true
            }
        }
    }

    Process {
        id: checkPactlProcess
        command: ["which", "pactl"]
        stdout: StdioCollector {}

        onExited: function(code, status) {
            root.volumeControlAvailable = (code === 0)
            if (root.volumeControlAvailable) {
                // Get initial volume
                getVolumeProcess.running = false
                getVolumeProcess.running = true
            }
        }
    }

    function _setSystemVolume(level) {
        if (!volumeControlAvailable) return

        const percentage = Math.round(level * 100)
        setVolumeProcess.running = false
        setVolumeProcess.command = ["pactl", "set-sink-volume", "@DEFAULT_SINK@", percentage + "%"]
        setVolumeProcess.running = true
    }

    // Open audio control GUI
    function openAudioControl() {
        launchProcess.running = false
        launchProcess.command = ["pavucontrol"]
        launchProcess.running = true
    }

    Process {
        id: launchProcess
        command: []
        onExited: function(code, status) {
            if (code !== 0) {
                console.warn("Failed to launch audio control")
            }
        }
    }

    Component.onCompleted: {
        // Check if pactl is available
        checkPactlProcess.running = false
        checkPactlProcess.running = true
    }
}
