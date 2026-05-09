// TrayItem.qml - System Tray Item Component
//
// Individual system tray item with icon, tooltip, and click handling.
// Integrates with SystemTrayService for activation and context menus.
// Compact macOS-style design.
//
// Usage:
//   TrayItem {
//       itemId: "nm-applet"
//       itemIcon: "network-wireless"
//       itemTooltip: "Network Manager"
//       needsAttention: false
//   }

import QtQuick
import "../../atoms" as Atoms
import "../../services" as Services
import "../../theme" as Theme

Rectangle {
    id: root
    implicitWidth: 20
    implicitHeight: 20

    // ========================================================================
    // Public Properties
    // ========================================================================

    // Item ID from SystemTrayService
    property string itemId: ""

    // Icon name or path
    property string itemIcon: ""

    // Tooltip text
    property string itemTooltip: ""

    // Whether item needs attention (e.g., notification)
    property bool needsAttention: false

    // ========================================================================
    // Visual Configuration
    // ========================================================================

    color: pressed ? Qt.alpha(Theme.ThemeEngine.colors.rosewater, 0.18)
                   : hovered ? Qt.alpha(Theme.ThemeEngine.colors.rosewater, 0.12) : "transparent"
    radius: 6

    property bool hovered: mouseArea.containsMouse
    property bool pressed: false

    // ========================================================================
    // Icon Display
    // ========================================================================

    Atoms.Icon {
        id: trayIcon
        anchors.centerIn: parent
        icon: root.itemIcon
        size: 14
        color: root.hovered ? Theme.ThemeEngine.colors.textPrimary : Theme.ThemeEngine.colors.textSecondary
        opacity: root.needsAttention ? (0.5 + 0.5 * Math.sin(Date.now() / 200)) : 1.0

        // Attention pulse via timer
        Timer {
            id: pulseTimer
            interval: 100
            running: root.needsAttention && root.visible
            repeat: true
            onTriggered: trayIcon.opacity = 0.5 + 0.5 * Math.sin(Date.now() / 200)
        }
    }

    // ========================================================================
    // Mouse Area
    // ========================================================================

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onPressed: root.pressed = true
        onReleased: root.pressed = false

        onClicked: function(mouse) {
            if (mouse.button === Qt.LeftButton) {
                if (Services.SystemTrayService) {
                    Services.SystemTrayService.activateItem(root.itemId)
                }
            } else if (mouse.button === Qt.RightButton) {
                if (Services.SystemTrayService) {
                    Services.SystemTrayService.secondaryActivateItem(root.itemId)
                }
            }
        }

        onWheel: function(wheel) {
            if (Services.SystemTrayService) {
                Services.SystemTrayService.scrollOnItem(root.itemId, wheel.angleDelta.y, "vertical")
                wheel.accepted = true
            }
        }
    }

    Atoms.Tooltip {
        target: root
        visible: root.itemTooltip !== "" && root.hovered
        text: root.itemTooltip
        position: "bottom"
    }

    // ========================================================================
    // Service Connections
    // ========================================================================

    Connections {
        target: Services.SystemTrayService
        enabled: target !== null
        function onTrayItemsChanged() {
            // Update if item data changed
        }
    }
}
