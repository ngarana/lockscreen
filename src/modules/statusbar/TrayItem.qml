// TrayItem.qml - System Tray Item Component
//
// Individual system tray item with icon, tooltip, and click handling.
// Integrates with SystemTrayService for activation and context menus.
//
// Usage:
//   TrayItem {
//       itemId: "nm-applet"
//       itemIcon: "network-wireless"
//       itemTooltip: "Network Manager"
//       needsAttention: false
//   }

import QtQuick
import "../../services" as Services
import "../../theme" as Theme

Rectangle {
    id: root

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

    color: "transparent"
    radius: Theme.Theme.radius.small

    property bool hovered: mouseArea.containsMouse
    property bool pressed: false

    // Attention animation
    property real _attentionOpacity: needsAttention ? 1.0 : 0.0

    // ========================================================================
    // Icon Display
    // ========================================================================

    Text {
        id: iconText
        anchors.centerIn: parent
        text: root.itemIcon
        font.pixelSize: Math.min(root.width, root.height) - 4
        color: root.hovered ? Theme.Theme.colors.textPrimary : Theme.Theme.colors.textSecondary

        // Attention pulse animation
        SequentialAnimation on opacity {
            id: attentionPulse
            running: root.needsAttention && root.visible
            loops: Animation.Infinite

            NumberAnimation {
                from: 1.0
                to: 0.4
                duration: 800
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                from: 0.4
                to: 1.0
                duration: 800
                easing.type: Easing.InOutQuad
            }
        }
    }

    // ========================================================================
    // Tooltip
    // ========================================================================

    Rectangle {
        id: tooltip
        visible: root.hovered && root.itemTooltip !== ""
        anchors.bottom: parent.top
        anchors.bottomMargin: 6
        anchors.horizontalCenter: parent.horizontalCenter

        width: tooltipText.implicitWidth + 12
        height: tooltipText.implicitHeight + 8
        radius: Theme.Theme.radius.small

        color: Theme.Theme.colors.surface0
        opacity: 0.95

        // Border
        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: parent.radius
            color: "transparent"
            border.width: 1
            border.color: Theme.Theme.colors.glassBorder
        }

        Text {
            id: tooltipText
            anchors.centerIn: parent
            text: root.itemTooltip
            font.pixelSize: 11
            color: Theme.Theme.colors.textPrimary
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
        onReleased: {
            root.pressed = false
        }

        onClicked: function(mouse) {
            if (mouse.button === Qt.LeftButton) {
                // Left click - activate
                Services.SystemTrayService.activateItem(root.itemId)
            } else if (mouse.button === Qt.RightButton) {
                // Right click - context menu
                Services.SystemTrayService.secondaryActivateItem(root.itemId)
            }
        }

        onEntered: {
            // Show tooltip
        }

        onExited: {
            // Hide tooltip
        }
    }

    // ========================================================================
    // Service Connections
    // ========================================================================

    Connections {
        target: Services.SystemTrayService
        function onTrayItemsChanged() {
            // Update if item data changed
        }
    }
}
