// SystemIndicators.qml - System status indicators row
//
// Compact row of system indicators: battery, network, volume, etc.
// Each indicator is interactive with hover/press feedback and tooltips.

import QtQuick
import QtQuick.Layouts
import "../../../services" as Services
import "../../../theme" as Theme
import "../../../atoms" as Atoms

RowLayout {
    id: root

    spacing: 6

    function _batteryColor() {
        const bs = Services.BatteryService
        if (!bs) return Theme.ThemeEngine.colors.textSecondary
        if (bs.isCharging) return Theme.ThemeEngine.colors.success
        if (bs.percentage <= 15) return Theme.ThemeEngine.colors.error
        if (bs.percentage <= 30) return Theme.ThemeEngine.colors.warning
        return Theme.ThemeEngine.colors.textSecondary
    }

    function _volumeIcon() {
        const as = Services.AudioService
        if (!as) return "\uf028"
        const vol = as.volume
        if (vol <= 0) return "\uf6a9"
        if (vol < 0.33) return "\uf027"
        if (vol < 0.66) return "\uf028"
        return "\uf026"
    }

    function _networkIcon() {
        const ns = Services.NetworkService
        if (!ns || !ns.isConnected) return "\uf7be"
        if (ns.connectionType === "ethernet") return "\uf6ff"
        return "\uf1eb"
    }

    function _batteryIcon() {
        const bs = Services.BatteryService
        if (!bs) return "\uf240"
        if (bs.isCharging) return "\uf0e7"
        if (bs.percentage <= 20) return "\uf243"
        return "\uf240"
    }

    // Volume indicator
    Item {
        id: volumeItem
        width: 24
        height: 16
        property bool hovered: volumeMouseArea.containsMouse
        property bool pressed: volumeMouseArea.pressed

        Rectangle {
            anchors.fill: parent
            radius: 4
            color: volumeItem.pressed ? Theme.ThemeEngine.colors.glassActive : (volumeItem.hovered ? Theme.ThemeEngine.colors.glassHover : "transparent")
            Behavior on color { ColorAnimation { duration: Theme.ThemeEngine.animation.fast } }
        }

        Text {
            anchors.centerIn: parent
            text: root._volumeIcon()
            font.pixelSize: 14
            font.family: Theme.ThemeEngine.fonts.iconFontFamily
            color: Theme.ThemeEngine.colors.textPrimary
        }

        MouseArea {
            id: volumeMouseArea
            anchors.fill: parent
            anchors.margins: -4
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true
            onClicked: Services.BarController && Services.BarController.toggleModule("quicksettings")
            onWheel: {
                var step = wheel.angleDelta.y > 0 ? 0.05 : -0.05
                if (Services.AudioService && Services.AudioService.canSetVolume) {
                    var newVol = Math.max(0.0, Math.min(1.0, Services.AudioService.volume + step))
                    Services.AudioService.setVolume(newVol)
                    wheel.accepted = true
                }
            }
        }

        Atoms.Tooltip {
            target: volumeItem
            visible: volumeItem.hovered
            text: "Volume"
            position: "bottom"
        }
    }

    // Network indicator
    Item {
        id: networkItem
        width: 24
        height: 16
        property bool hovered: networkMouseArea.containsMouse
        property bool pressed: networkMouseArea.pressed

        Rectangle {
            anchors.fill: parent
            radius: 4
            color: networkItem.pressed ? Theme.ThemeEngine.colors.glassActive : (networkItem.hovered ? Theme.ThemeEngine.colors.glassHover : "transparent")
            Behavior on color { ColorAnimation { duration: Theme.ThemeEngine.animation.fast } }
        }

        Text {
            anchors.centerIn: parent
            text: root._networkIcon()
            font.pixelSize: 14
            font.family: Theme.ThemeEngine.fonts.iconFontFamily
            color: Theme.ThemeEngine.colors.textPrimary
        }

        MouseArea {
            id: networkMouseArea
            anchors.fill: parent
            anchors.margins: -4
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true
            onClicked: Services.BarController && Services.BarController.toggleModule("quicksettings")
        }

        Atoms.Tooltip {
            target: networkItem
            visible: networkItem.hovered
            text: "Network"
            position: "bottom"
        }
    }

    // Battery indicator
    Item {
        id: batteryItem
        width: 40
        height: 16
        property bool hovered: batteryMouseArea.containsMouse
        property bool pressed: batteryMouseArea.pressed

        Rectangle {
            anchors.fill: parent
            radius: 4
            color: batteryItem.pressed ? Theme.ThemeEngine.colors.glassActive : (batteryItem.hovered ? Theme.ThemeEngine.colors.glassHover : "transparent")
            Behavior on color { ColorAnimation { duration: Theme.ThemeEngine.animation.fast } }
        }

        Text {
            id: batIcon
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            text: root._batteryIcon()
            font.pixelSize: 14
            font.family: Theme.ThemeEngine.fonts.iconFontFamily
            color: root._batteryColor()
        }

        Text {
            anchors.left: batIcon.right
            anchors.leftMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            text: Services.BatteryService ? Services.BatteryService.percentage + "%" : ""
            font.pixelSize: 10
            color: root._batteryColor()
            font.family: Theme.ThemeEngine.typography.fontFamily
        }

        MouseArea {
            id: batteryMouseArea
            anchors.fill: parent
            anchors.margins: -4
            cursorShape: Qt.PointingHandCursor
            hoverEnabled: true
            onClicked: Services.BarController && Services.BarController.toggleModule("quicksettings")
        }

        Atoms.Tooltip {
            target: batteryItem
            visible: batteryItem.hovered
            text: "Battery"
            position: "bottom"
        }
    }
}
