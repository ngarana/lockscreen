// VideoConfig.qml - Video Background Configuration Service
//
// Singleton providing configuration for the video background system.
// Controls playlist selection, day/night boundaries, and crossfade timing.

pragma Singleton
import Quickshell
import QtQuick

QtObject {
    id: root

    // ========================================================================
    // Video Source Configuration
    // ========================================================================

    // Base directory for video files (symlinked to aerial-sddm-theme)
    readonly property string videoDir: Qt.resolvedUrl("../../videos")

    // Playlist files (relative to project root)
    readonly property string dayPlaylist: Qt.resolvedUrl("../../playlists/day.m3u")
    readonly property string nightPlaylist: Qt.resolvedUrl("../../playlists/night.m3u")

    // ========================================================================
    // Day/Night Scheduling
    // ========================================================================

    // Day starts at 6:30 (6.5 in decimal hours)
    readonly property real dayTimeStart: 6.5

    // Day ends at 18:30 (18.5 in decimal hours)
    readonly property real dayTimeEnd: 18.5

    // Returns true if current time is within day hours
    readonly property bool isDaytime: {
        var now = new Date()
        var hour = now.getHours() + now.getMinutes() / 60.0
        return hour >= dayTimeStart && hour < dayTimeEnd
    }

    // Returns the appropriate playlist URL for current time of day
    readonly property string activePlaylist: isDaytime ? dayPlaylist : nightPlaylist

    // ========================================================================
    // Playback Configuration
    // ========================================================================

    // Duration of crossfade between videos (milliseconds)
    readonly property int crossfadeDuration: 3000

    // How many seconds before end of video to begin preloading next
    readonly property int preloadLeadTime: 10

    // Master toggle for video backgrounds
    readonly property bool enabled: true
}
