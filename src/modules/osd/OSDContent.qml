// OSDContent.qml - OSD Visual Content
//
// Displays volume, brightness, or microphone indicator
// with icon and progress bar.

import QtQuick
import QtQuick.Layouts
import "../../atoms" as Atoms
import "../../components" as Components
import "../../theme" as Theme

Components.GlassPanel {
    id: root

    required property string osdType
    required property real value
    required property bool muted
    required property real maxValue

    implicitWidth: 280
    implicitHeight: layout.implicitHeight + Theme.ThemeEngine.padding.large * 2
    radius: Theme.ThemeEngine.radius.large
    backgroundColor: Theme.ThemeEngine.colors.glass
    borderColor: Theme.ThemeEngine.colors.glassBorder
    elevation: 3

    readonly property string iconName: {
        if (osdType === "volume") {
            if (muted || value <= 0) return Theme.ThemeEngine.icons.volumeMuted
            if (value < 0.33) return Theme.ThemeEngine.icons.volumeLow
            if (value < 0.66) return Theme.ThemeEngine.icons.volumeMedium
            return Theme.ThemeEngine.icons.volumeHigh
        } else if (osdType === "brightness") {
            return Theme.ThemeEngine.icons.brightness
        } else if (osdType === "microphone") {
            return muted ? Theme.ThemeEngine.icons.micOff : Theme.ThemeEngine.icons.mic
        }
        return ""
    }

    readonly property color iconColor: {
        if (muted) return Theme.ThemeEngine.colors.textMuted
        if (osdType === "volume") return Theme.ThemeEngine.colors.primary
        if (osdType === "brightness") return Theme.ThemeEngine.colors.warning
        if (osdType === "microphone") return Theme.ThemeEngine.colors.error
        return Theme.ThemeEngine.colors.textPrimary
    }

    ColumnLayout {
        id: layout
        anchors.centerIn: parent
        width: parent.width - Theme.ThemeEngine.padding.large * 2
        spacing: Theme.ThemeEngine.spacing.medium

        Atoms.Icon {
            Layout.alignment: Qt.AlignHCenter
            icon: root.iconName
            size: 48
            color: root.iconColor
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Theme.ThemeEngine.spacing.medium

            Atoms.Slider {
                Layout.fillWidth: true
                from: 0
                to: root.maxValue
                value: root.value
                enabled: false
            }

            Atoms.Label {
                text: Math.round((root.value / root.maxValue) * 100) + "%"
                fontSize: Theme.ThemeEngine.typography.sizeMd
                fontWeight: Theme.ThemeEngine.typography.weightSemiBold
                color: Theme.ThemeEngine.colors.textPrimary
            }
        }
    }
}
