// VideoBackground.qml - Aerial Video Background Component
//
// Full-screen video background with dual-player crossfade.
// Implements the aerial-sddm-theme video playback pattern
// using Qt6 MediaPlayer API (no Playlist QML type in Qt6).
//
// Features:
// - Dual MediaPlayer for seamless crossfade transitions
// - Custom JS playlist manager (parses .m3u files via FileView)
// - Time-of-day playlist selection (day/night)
// - Random shuffle on load
// - Muted playback (lockscreen, not media app)
// - Fallback solid color when video unavailable

import QtQuick
import QtMultimedia
import Quickshell
import Quickshell.Io
import "../services"

Item {
    id: root
    anchors.fill: parent

    // ========================================================================
    // Playlist State
    // ========================================================================

    property var videoList: []
    property int currentIndex: -1
    property bool usePlayerA: true
    property bool preloaded: false

    // ========================================================================
    // Black Base (visible during load / fallback)
    // ========================================================================

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    // ========================================================================
    // Player A
    // ========================================================================

    MediaPlayer {
        id: playerA
        videoOutput: videoOutA
        autoPlay: false
        audioOutput: AudioOutput { muted: true }

        onMediaStatusChanged: {
            if (mediaStatus === MediaPlayer.EndOfMedia) {
                root.playNext()
            }
        }

        onErrorOccurred: function(error, errorString) {
            console.warn("[VideoBackground] PlayerA error:", error, errorString)
        }
    }

    VideoOutput {
        id: videoOutA
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectCrop
        opacity: root.usePlayerA ? 1.0 : 0.0

        Behavior on opacity {
            NumberAnimation {
                duration: VideoConfig.crossfadeDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    // ========================================================================
    // Player B
    // ========================================================================

    MediaPlayer {
        id: playerB
        videoOutput: videoOutB
        autoPlay: false
        audioOutput: AudioOutput { muted: true }

        onMediaStatusChanged: {
            if (mediaStatus === MediaPlayer.EndOfMedia) {
                root.playNext()
            }
        }

        onErrorOccurred: function(error, errorString) {
            console.warn("[VideoBackground] PlayerB error:", error, errorString)
        }
    }

    VideoOutput {
        id: videoOutB
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectCrop
        opacity: root.usePlayerA ? 0.0 : 1.0

        Behavior on opacity {
            NumberAnimation {
                duration: VideoConfig.crossfadeDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    // ========================================================================
    // Crossfade Timer
    // ========================================================================

    Timer {
        id: crossfadeTimer
        interval: 1000
        running: root.videoList.length > 0
        repeat: true

        onTriggered: {
            var active = root.usePlayerA ? playerA : playerB
            var standby = root.usePlayerA ? playerB : playerA

            if (active.duration > 0 && active.position > 0) {
                var remaining = active.duration - active.position

                // Preload next video on standby player
                if (remaining < VideoConfig.preloadLeadTime * 1000 && !root.preloaded) {
                    var nextIdx = (root.currentIndex + 1) % root.videoList.length
                    var nextSource = root.resolveVideoPath(root.videoList[nextIdx])
                    standby.source = nextSource
                    root.preloaded = true
                }

                // Initiate crossfade
                if (remaining < VideoConfig.crossfadeDuration) {
                    root.crossfade()
                }
            }
        }
    }

    // ========================================================================
    // Playlist File Reader (Quickshell FileView)
    // ========================================================================
    // FileView.text() is a function, not a property.
    // FileView.path accepts URLs (Qt.resolvedUrl works directly).
    // Use the `loaded` signal to know when file is ready.

    FileView {
        id: playlistFile
        // path accepts a URL string, Qt.resolvedUrl returns a URL
        path: Qt.resolvedUrl("../../playlists/" + (VideoConfig.isDaytime ? "day" : "night") + ".m3u")
        blockLoading: true

        onLoaded: {
            root.parsePlaylist(text())
        }

        onLoadFailed: function(error) {
            console.warn("[VideoBackground] Playlist load failed:", error)
        }
    }

    // ========================================================================
    // Methods
    // ========================================================================

    function parsePlaylist(content) {
        var lines = content.split("\n")
        var videos = []

        for (var i = 0; i < lines.length; i++) {
            var line = lines[i].trim()
            // Skip empty lines and comments
            if (line.length > 0 && !line.startsWith("#")) {
                videos.push(line)
            }
        }



        // Shuffle using Fisher-Yates
        for (var j = videos.length - 1; j > 0; j--) {
            var k = Math.floor(Math.random() * (j + 1))
            var tmp = videos[j]
            videos[j] = videos[k]
            videos[k] = tmp
        }

        root.videoList = videos
        root.currentIndex = 0

        if (videos.length > 0) {
            playFirst()
        }
    }

    function resolveVideoPath(relativePath) {
        // The playlist entries are like "videos/filename.mov"
        // Resolve relative to the project root (this file is in src/components/)
        var resolved = Qt.resolvedUrl("../../" + relativePath)
        return resolved
    }

    function playFirst() {
        var source = resolveVideoPath(videoList[0])
        playerA.source = source
        playerA.play()
        root.usePlayerA = true
        root.preloaded = false
    }

    function playNext() {
        root.currentIndex = (root.currentIndex + 1) % root.videoList.length
        root.preloaded = false

        var source = resolveVideoPath(root.videoList[root.currentIndex])

        if (root.usePlayerA) {
            playerB.source = source
            playerB.play()
            root.usePlayerA = false
            cleanupTimer.targetPlayer = playerA
            cleanupTimer.start()
        } else {
            playerA.source = source
            playerA.play()
            root.usePlayerA = true
            cleanupTimer.targetPlayer = playerB
            cleanupTimer.start()
        }
    }

    function crossfade() {
        playNext()
    }

    // ========================================================================
    // Cleanup Timer
    // ========================================================================

    Timer {
        id: cleanupTimer
        interval: VideoConfig.crossfadeDuration + 500
        running: false
        repeat: false
        property var targetPlayer: null

        onTriggered: {
            if (targetPlayer) {
                targetPlayer.stop()
                targetPlayer.source = ""
            }
        }
    }
}
