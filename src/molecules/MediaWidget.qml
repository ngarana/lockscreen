// MediaWidget.qml - Media Player Mini Widget Molecule
//
// Displays currently playing media with album art, title, artist,
// and playback controls. Integrates with AudioService.
//
// Usage:
//   MediaWidget {
//       compact: true
//       onPlayClicked: AudioService.togglePlaying()
//   }
//
// Properties:
//   - compact: bool - Compact mode for small spaces (default: false)
//   - showControls: bool - Show playback controls (default: true)
//
// Signals:
//   - playClicked(): Emitted when play/pause is clicked
//   - nextClicked(): Emitted when next is clicked
//   - previousClicked(): Emitted when previous is clicked
//   - clicked(): Emitted when widget is clicked

import QtQuick
import QtQuick.Layouts
import "../atoms" as Atoms
import "../services" as Services
import "../theme" as Theme

Atoms.Card {
    id: root

    // Public API
    property bool compact: false
    property bool showControls: true

    signal playClicked()
    signal nextClicked()
    signal previousClicked()
    signal clicked()

    // Internal state from service
    readonly property bool _hasPlayer: Services.AudioService.hasPlayer
    readonly property bool _isPlaying: Services.AudioService.isPlaying
    readonly property string _title: Services.AudioService.title
    readonly property string _artist: Services.AudioService.artist
    readonly property string _album: Services.AudioService.album
    readonly property string _source: Services.AudioService.source
    readonly property real _progress: Services.AudioService.positionSeconds
    readonly property real _duration: Services.AudioService.durationSeconds

    // Layout
    width: root.compact ? 280 : 320
    implicitHeight: contentColumn.implicitHeight + (padding * 2)
    padding: root.compact ? 12 : 16
    hoverable: true
    elevated: false

    // Visible only when there's a player
    visible: root._hasPlayer

    // Content column
    ColumnLayout {
        id: contentColumn
        anchors.fill: parent
        spacing: root.compact ? 8 : 12

        // Media info row
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            // Album art placeholder
            Rectangle {
                Layout.preferredWidth: root.compact ? 48 : 64
                Layout.preferredHeight: root.compact ? 48 : 64
                radius: Theme.ThemeEngine.radius.small
                color: Theme.ThemeEngine.colors.surface0

                // Music note icon
                Atoms.Icon {
                    anchors.centerIn: parent
                    source: "audio"
                    size: root.compact ? 24 : 32
                    color: Theme.ThemeEngine.colors.textMuted
                }
            }

            // Track info
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Atoms.Label {
                    Layout.fillWidth: true
                    text: root._title || "Unknown Title"
                    fontSize: Theme.ThemeEngine.typography.sizeMd
                    fontWeight: Theme.ThemeEngine.typography.weightMedium
                    color: Theme.ThemeEngine.colors.textPrimary
                    truncate: true
                }

                Atoms.Label {
                    Layout.fillWidth: true
                    text: root._artist || "Unknown Artist"
                    fontSize: Theme.ThemeEngine.typography.sizeSm
                    color: Theme.ThemeEngine.colors.textSecondary
                    truncate: true
                }

                Atoms.Label {
                    visible: !root.compact && root._album !== ""
                    text: root._album
                    fontSize: Theme.ThemeEngine.typography.sizeXs
                    color: Theme.ThemeEngine.colors.textMuted
                    truncate: true
                }
            }
        }

        // Progress bar
        Atoms.ProgressBar {
            Layout.fillWidth: true
            visible: root._duration > 0
            height: 3
            value: root._duration > 0 ? root._progress / root._duration : 0
            barHeight: 3
            barRadius: 1
        }

        // Controls
        RowLayout {
            Layout.fillWidth: true
            visible: root.showControls
            spacing: 8

            // Previous button
            Atoms.IconButton {
                size: 36
                iconSize: 18
                icon: "skip-back"
                tooltip: "Previous"
                onClicked: {
                    Services.AudioService.previous()
                    root.previousClicked()
                }
            }

            // Play/Pause button
            Atoms.IconButton {
                size: 44
                iconSize: 22
                icon: root._isPlaying ? "pause" : "play"
                tooltip: root._isPlaying ? "Pause" : "Play"
                onClicked: {
                    Services.AudioService.togglePlaying()
                    root.playClicked()
                }
            }

            // Next button
            Atoms.IconButton {
                size: 36
                iconSize: 18
                icon: "skip-forward"
                tooltip: "Next"
                onClicked: {
                    Services.AudioService.next()
                    root.nextClicked()
                }
            }

            Item { Layout.fillWidth: true }

            // Source indicator
            Atoms.Label {
                text: root._source
                fontSize: Theme.ThemeEngine.typography.sizeXs
                color: Theme.ThemeEngine.colors.textMuted
            }
        }
    }

    // Click to expand
    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
