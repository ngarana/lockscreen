// AudioController.qml - Main Audio Controller Widget
//
// Combines metadata, transport controls, and volume slider into one
// glassmorphic lockscreen widget. Auto-shows when audio is active and
// UI is revealed.
//
// Placement: above power buttons, below status message.

import QtQuick
import QtQuick.Layouts
import "../audio" as Audio
import "../services" as Services
import "../components"

Item {
    id: root

    implicitWidth: panel.implicitWidth
    implicitHeight: panel.implicitHeight

    // ====== Public Properties ======

    property bool revealed: false   // bound to root.uiRevealed
    property bool showVolume: true
    property bool showProgress: true

    // ====== Visibility Rules ======

    // Must be both revealed AND have an active player
    visible: root.revealed && Audio.AudioService.isActive
    enabled: visible
    opacity: visible ? 1.0 : 0.0

    // ====== Glassmorphic Panel ======

    Rectangle {
        id: panel

        anchors.fill: parent

        color: Services.Theme.colors.glass
        radius: Services.Theme.radius.large
        border.width: 1
        border.color: Services.Theme.colors.glassBorder

        clip: true

        implicitWidth: Services.Theme.audio.minWidth
        implicitHeight: contentLayout.implicitHeight + (Services.Theme.audio.panelPadding * 2)

        // ====== Content Layout ======

        ColumnLayout {
            id: contentLayout
            anchors.fill: parent
            anchors.margins: Services.Theme.audio.panelPadding
            spacing: Services.Theme.audio.spacing

            // Metadata section
            AudioMetadata {
                id: metadata
                title: Audio.AudioService.title
                artist: Audio.AudioService.artist
                album: Audio.AudioService.album
                sourceLabel: Audio.AudioService.sourceLabel
                positionSeconds: Audio.AudioService.positionSeconds
                durationSeconds: Audio.AudioService.durationSeconds
                showProgress: root.showProgress
                showSourceLabel: true
                Layout.fillWidth: true
            }

            // Transport buttons row
            RowLayout {
                spacing: Services.Theme.audio.spacing
                Layout.alignment: Qt.AlignHCenter

                // Previous
                AudioPlayerButton {
                    action: "previous"
                    icon: "\u23EE"   // ⏮ black right-pointing double triangle
                    label: "Previous"
                    enabled: Audio.AudioService.canGoPrevious
                    onClicked: Audio.AudioService.previous()
                }

                // Play/Pause
                AudioPlayerButton {
                    action: "playPause"
                    icon: Audio.AudioService.isPlaying ? "\u23F8" : "\u25B6"  // ⏸ / ▶
                    label: Audio.AudioService.isPlaying ? "Pause" : "Play"
                    enabled: Audio.AudioService.canTogglePlaying
                    onClicked: Audio.AudioService.togglePlaying()
                }

                // Next
                AudioPlayerButton {
                    action: "next"
                    icon: "\u23ED"   // ⏭ black right-pointing double triangle
                    label: "Next"
                    enabled: Audio.AudioService.canGoNext
                    onClicked: Audio.AudioService.next()
                }
            }

            // Volume slider row (optional)
            RowLayout {
                id: volumeRow
                visible: root.showVolume
                    && Audio.AudioService.volumeMode !== "none"
                    && Audio.AudioService.canSetVolume
                spacing: Services.Theme.spacing.small
                Layout.fillWidth: true

                Text {
                    text: "Vol"
                    font.pixelSize: 11
                    font.family: Services.Theme.fonts.fontFamily
                    color: Services.Theme.colors.textMuted
                }

                // Custom styled slider — avoids platform-default Slider styling
                Rectangle {
                    id: volumeTrack
                    implicitWidth: Services.Theme.audio.volumeSliderWidth
                    implicitHeight: Services.Theme.audio.progressHeight * 3
                    color: Services.Theme.colors.glassBorder
                    radius: height / 2

                    // Fill
                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: parent.width * Audio.AudioService.volume
                        color: Services.Theme.colors.primary
                        radius: height / 2
                    }

                    // Handle (visible on hover)
                    Rectangle {
                        id: volumeHandle
                        x: (volumeTrack.width - width) * Audio.AudioService.volume - (width / 2)
                        y: (volumeTrack.height - height) / 2
                        width: Services.Theme.audio.progressHeight * 3
                        height: width
                        color: Services.Theme.colors.primary
                        radius: width / 2
                        opacity: volumeArea.containsMouse ? 1.0 : 0.0

                        Behavior on opacity {
                            NumberAnimation { duration: Services.Theme.animation.fast }
                        }
                    }

                    MouseArea {
                        id: volumeArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onPressed: _setVolumeFromMouse(mouse.x)
                        onPositionChanged: _setVolumeFromMouse(mouse.x)

                        function _setVolumeFromMouse(xPos) {
                            var ratio = Math.max(0, Math.min(1, xPos / volumeTrack.width))
                            Audio.AudioService.setVolume(ratio)
                        }
                    }
                }

                Text {
                    text: Math.round(Audio.AudioService.volume * 100) + "%"
                    font.pixelSize: 11
                    font.family: Services.Theme.fonts.fontFamily
                    color: Services.Theme.colors.textMuted
                    Layout.minimumWidth: 35
                    horizontalAlignment: Text.AlignRight
                }
            }
        }
    }

    // ====== Reveal Animation ======

    Behavior on opacity {
        NumberAnimation {
            duration: Services.Theme.animation.reveal
            easing.type: Easing.InOutQuad
        }
    }
}
