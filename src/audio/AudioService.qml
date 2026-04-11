// AudioService.qml
// Singleton wrapper around Quickshell MPRIS service for lockscreen audio controls.
// Kept in a dedicated module so optional audio integration cannot break core UI.

pragma Singleton

import Quickshell
import Quickshell.Services.Mpris
import QtQuick

Singleton {
    id: root

    property var playerPriority: [
        "org.mpris.MediaPlayer2.mpv",
        "org.mpris.MediaPlayer2.mpd"
    ]

    property var playerPrefixes: [
        "org.mpris.MediaPlayer2.firefox",
        "org.mpris.MediaPlayer2.chromium",
        "org.mpris.MediaPlayer2.spotify"
    ]

    property bool acceptAnyPlayer: true
    property string volumeMode: "player"

    readonly property var players: Mpris.players.values

    readonly property var activePlayer: {
        if (!root.players)
            return null

        var prioritized = []
        var allPlaying = []

        for (var i = 0; i < root.players.length; i++) {
            var player = root.players[i]
            if (!player)
                continue

            if (_matchesPriority(player.dbusName))
                prioritized.push(player)

            if (player.isPlaying)
                allPlaying.push(player)
        }

        for (var j = 0; j < prioritized.length; j++) {
            if (prioritized[j].isPlaying)
                return prioritized[j]
        }

        if (prioritized.length > 0)
            return prioritized[0]

        if (root.acceptAnyPlayer && allPlaying.length > 0)
            return allPlaying[0]

        if (root.acceptAnyPlayer && root.players.length > 0)
            return root.players[0]

        return null
    }

    readonly property bool hasPlayer: activePlayer !== null
    readonly property bool isActive: hasPlayer && activePlayer.playbackState !== 0
    readonly property bool isPlaying: hasPlayer && activePlayer.isPlaying

    readonly property string title: hasPlayer ? activePlayer.trackTitle : ""
    readonly property string artist: hasPlayer ? activePlayer.trackArtist : ""
    readonly property string album: hasPlayer ? activePlayer.trackAlbum : ""
    readonly property string source: hasPlayer ? _sourceFromDbusName(activePlayer.dbusName) : ""
    readonly property string sourceLabel: hasPlayer ? activePlayer.identity : ""

    readonly property bool hasMetadata: title !== "" || artist !== "" || album !== ""

    readonly property real durationSeconds: hasPlayer && activePlayer.lengthSupported ? activePlayer.length : 0
    readonly property real positionSeconds: hasPlayer && activePlayer.positionSupported ? activePlayer.position : 0
    readonly property real volume: hasPlayer && activePlayer.volumeSupported ? activePlayer.volume : 1.0

    readonly property bool canControl: hasPlayer && activePlayer.canControl
    readonly property bool canTogglePlaying: hasPlayer && activePlayer.canTogglePlaying
    readonly property bool canGoNext: hasPlayer && activePlayer.canGoNext
    readonly property bool canGoPrevious: hasPlayer && activePlayer.canGoPrevious
    readonly property bool canSeek: hasPlayer && activePlayer.canSeek && activePlayer.positionSupported
    readonly property bool canSetVolume: {
        if (volumeMode === "none")
            return false

        return hasPlayer && activePlayer.canControl && activePlayer.volumeSupported
    }

    signal playbackStateChanged(bool playing)
    signal metadataChanged()
    signal positionChanged(real positionSeconds)
    signal playerChanged(string source)

    function play() {
        if (!hasPlayer || !activePlayer.canControl)
            return

        activePlayer.play()
    }

    function pause() {
        if (!hasPlayer || !activePlayer.canControl)
            return

        activePlayer.pause()
    }

    function togglePlaying() {
        if (!hasPlayer || !activePlayer.canTogglePlaying)
            return

        if (activePlayer.isPlaying)
            activePlayer.pause()
        else
            activePlayer.play()
    }

    function next() {
        if (!hasPlayer || !activePlayer.canGoNext)
            return

        activePlayer.next()
    }

    function previous() {
        if (!hasPlayer || !activePlayer.canGoPrevious)
            return

        activePlayer.previous()
    }

    function stop() {
        if (!hasPlayer || !activePlayer.canControl)
            return

        activePlayer.stop()
    }

    function setVolume(level) {
        var clamped = Math.max(0.0, Math.min(1.0, level))

        if (!hasPlayer || !activePlayer.volumeSupported)
            return

        activePlayer.volume = clamped
    }

    function seekTo(seconds) {
        if (!hasPlayer || !activePlayer.canSeek || !activePlayer.positionSupported)
            return

        var clamped = Math.max(0, Math.min(activePlayer.length, seconds))
        activePlayer.position = clamped
    }

    function _matchesPriority(dbusName) {
        if (root.playerPriority.indexOf(dbusName) !== -1)
            return true

        for (var i = 0; i < root.playerPrefixes.length; i++) {
            if (dbusName.indexOf(root.playerPrefixes[i]) === 0)
                return true
        }

        return false
    }

    function _sourceFromDbusName(dbusName) {
        if (dbusName.indexOf("mpv") !== -1)
            return "mpv"
        if (dbusName.indexOf("mpd") !== -1)
            return "mpd"
        if (dbusName.indexOf("firefox") !== -1)
            return "firefox"
        if (dbusName.indexOf("chromium") !== -1)
            return "chromium"
        if (dbusName.indexOf("spotify") !== -1)
            return "spotify"

        var match = dbusName.match(/MediaPlayer2\.([^.]+)/)
        return match ? match[1] : dbusName
    }

    onActivePlayerChanged: {
        root.playerChanged(root.source)
        positionTimer.restart()
    }

    Connections {
        target: root.activePlayer
        enabled: root.hasPlayer

        function onIsPlayingChanged() {
            if (root.hasPlayer)
                root.playbackStateChanged(root.activePlayer.isPlaying)
        }
    }

    Connections {
        target: root.activePlayer
        enabled: root.hasPlayer

        function onTrackTitleChanged() {
            root.metadataChanged()
        }

        function onTrackArtistChanged() {
            root.metadataChanged()
        }

        function onTrackAlbumChanged() {
            root.metadataChanged()
        }
    }

    Timer {
        id: positionTimer
        interval: 1000
        running: root.hasPlayer && root.activePlayer.isPlaying && root.activePlayer.positionSupported
        repeat: true
        triggeredOnStart: false

        onTriggered: {
            if (root.hasPlayer && root.activePlayer.positionSupported)
                root.positionChanged(root.activePlayer.position)
        }
    }

    onHasPlayerChanged: {
        if (root.hasPlayer)
            console.log("AudioService: Active player:", root.activePlayer.dbusName, "Source:", root.source, "Playing:", root.activePlayer.isPlaying)
        else
            console.log("AudioService: No active player")
    }
}
