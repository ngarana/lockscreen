// AudioMetadata.qml - Track Information and Progress Display
//
// Glassmorphic panel showing track title, artist, album, and playback progress.
// Uses ColumnLayout/RowLayout for predictable sizing and alignment.

import QtQuick
import QtQuick.Layouts
import "../services"

Item {
    id: root

    // ====== Public Properties ======

    property string title: ""
    property string artist: ""
    property string album: ""
    property string sourceLabel: ""
    property real positionSeconds: 0
    property real durationSeconds: 0
    property bool showProgress: true
    property bool showSourceLabel: false
    property bool isLive: durationSeconds <= 0

    implicitWidth: contentLayout.implicitWidth + (Theme.audio.panelPadding * 2)
    implicitHeight: contentLayout.implicitHeight + (Theme.audio.panelPadding * 2)

    // ====== Glassmorphic Panel ======

    Rectangle {
        id: panel

        anchors.fill: parent

        color: Theme.colors.glass
        radius: Theme.radius.large
        border.width: 1
        border.color: Theme.colors.glassBorder

        clip: true

        // ====== Layout ======

        ColumnLayout {
            id: contentLayout
            anchors.fill: parent
            anchors.margins: Theme.audio.panelPadding
            spacing: Theme.audio.spacing

            // Source label (optional)
            RowLayout {
                visible: root.showSourceLabel && root.sourceLabel !== ""
                spacing: 0

                Text {
                    text: root.sourceLabel
                    font.pixelSize: 11
                    font.family: Theme.fonts.fontFamily
                    font.weight: Font.Medium
                    color: Theme.colors.textMuted
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }

            // "Now Playing" header
            Text {
                text: "Now Playing"
                font.pixelSize: 11
                font.family: Theme.fonts.fontFamily
                font.weight: Font.Medium
                color: Theme.colors.textMuted
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            // Track title
            Text {
                text: root.title !== "" ? root.title : "Unknown Track"
                font.pixelSize: Theme.fonts.textSizeLarge
                font.family: Theme.fonts.fontFamily
                font.weight: Font.Bold
                color: Theme.colors.text
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            // Artist - Album
            Text {
                text: _formatArtistAlbum()
                font.pixelSize: Theme.fonts.textSize
                font.family: Theme.fonts.fontFamily
                color: Theme.colors.textSubtle
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            // Progress bar and time
            ColumnLayout {
                visible: root.showProgress && !root.isLive
                spacing: Theme.spacing.small
                Layout.fillWidth: true

                // Progress track
                Rectangle {
                    id: progressTrack
                    width: parent.width
                    height: Theme.audio.progressHeight
                    color: Theme.colors.glassBorder
                    radius: height / 2

                    // Progress fill
                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: parent.width * _progressRatio()
                        color: Theme.colors.primary
                        radius: height / 2
                    }
                }

                // Time labels
                RowLayout {
                    spacing: 0
                    Layout.fillWidth: true

                    Text {
                        text: _formatTime(root.positionSeconds)
                        font.pixelSize: 11
                        font.family: Theme.fonts.fontFamily
                        color: Theme.colors.textMuted
                        Layout.fillWidth: true
                    }

                    Text {
                        text: _formatTime(root.durationSeconds)
                        font.pixelSize: 11
                        font.family: Theme.fonts.fontFamily
                        color: Theme.colors.textMuted
                        horizontalAlignment: Text.AlignRight
                        Layout.fillWidth: true
                    }
                }
            }

            // Live indicator
            RowLayout {
                visible: root.showProgress && root.isLive
                spacing: Theme.spacing.small
                Layout.fillWidth: true

                Rectangle {
                    implicitWidth: 8
                    implicitHeight: 8
                    color: Theme.colors.error
                    radius: 4
                }

                Text {
                    text: "LIVE"
                    font.pixelSize: 11
                    font.family: Theme.fonts.fontFamily
                    font.weight: Font.Bold
                    color: Theme.colors.error
                }
            }
        }
    }

    // ====== Internal Helpers ======

    function _formatArtistAlbum() {
        var parts = []
        if (root.artist !== "") parts.push(root.artist)
        if (root.album !== "") parts.push(root.album)
        if (parts.length > 0) return parts.join(" \u2014 ")
        return "Unknown Artist"
    }

    function _progressRatio() {
        if (root.durationSeconds <= 0) return 0
        return Math.min(1.0, root.positionSeconds / root.durationSeconds)
    }

    function _formatTime(seconds) {
        if (seconds < 0) seconds = 0
        var totalSec = Math.floor(seconds)
        var mins = Math.floor(totalSec / 60)
        var secs = totalSec % 60
        return mins + ":" + (secs < 10 ? "0" : "") + secs
    }
}
