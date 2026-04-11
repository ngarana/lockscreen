// AudioService.qml
// Compatibility shim kept out of the shared Services module so optional audio
// integrations cannot interfere with the core lockscreen UI.

pragma Singleton

import Quickshell
import QtQuick

Singleton {
    id: root

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
    readonly property real volume: 1.0
    readonly property bool canControl: false
    readonly property bool canTogglePlaying: false
    readonly property bool canGoNext: false
    readonly property bool canGoPrevious: false
    readonly property bool canSeek: false
    readonly property bool canSetVolume: false
    property string volumeMode: "none"

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
    function setVolume(level) {}
    function seekTo(seconds) {}
}
