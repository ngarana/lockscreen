// VideoBackground.qml - Lock Screen Background
//
// Gradient background that replaces the video player.
// Video playback is disabled because QtMultimedia's FFmpeg/PipeWire
// backend crashes in Quickshell (no QCoreApplication for audio thread).
// Re-enable with MediaPlayer when the upstream issue is resolved.

import QtQuick

Item {
    anchors.fill: parent

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#1e1e2e" }
            GradientStop { position: 0.5; color: "#181825" }
            GradientStop { position: 1.0; color: "#11111b" }
        }
    }
}
