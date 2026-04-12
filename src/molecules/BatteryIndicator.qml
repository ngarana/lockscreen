// BatteryIndicator.qml - Battery Icon + Percentage Molecule
//
// Displays battery level with icon and percentage.
// Color changes based on level and charging state.
//
// Usage:
//   BatteryIndicator {
//       showPercentage: true
//   }
//
// Properties:
//   - showPercentage: bool - Show percentage text
//   - showIcon: bool - Show battery icon
//   - criticalThreshold: int - Level considered critical (default: 15)
//
// Signals:
//   - clicked(): Emitted when indicator is clicked

import QtQuick
import QtQuick.Layouts
import "../atoms" as Atoms
import "../services" as Services
import "../theme" as Theme

RowLayout {
    id: root

    // Public API
    property bool showPercentage: true
    property bool showIcon: true
    property int criticalThreshold: 15

    signal clicked()

    // Internal state from service
    readonly property int _percentage: Services.BatteryService.percentage
    readonly property bool _isCharging: Services.BatteryService.isCharging
    readonly property bool _isCritical: _percentage <= root.criticalThreshold && !_isCharging
    readonly property bool _isLow: _percentage <= 30 && !_isCharging

    // Layout
    spacing: 6

    // Battery icon
    Item {
        id: batteryIconContainer
        visible: root.showIcon
        width: 22
        height: 12

        // Battery outline
        Rectangle {
            anchors.fill: parent
            radius: 2
            color: "transparent"
            border.width: 1.5
            border.color: root._getBatteryColor()
        }

        // Battery terminal
        Rectangle {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: 2
            height: 6
            radius: 1
            color: root._getBatteryColor()
        }

        // Fill level
        Rectangle {
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
                margins: 2
            }
            width: (parent.width - 6) * (root._percentage / 100)
            radius: 1
            color: root._getBatteryColor()

            Behavior on width {
                NumberAnimation { duration: Theme.Theme.animation.medium }
            }
        }

        // Charging indicator
        Atoms.Icon {
            anchors.centerIn: parent
            source: "lightning-bolt"
            size: 8
            color: Theme.Theme.colors.crust
            visible: root._isCharging
        }
    }

    // Percentage label
    Atoms.Label {
        id: percentageLabel
        visible: root.showPercentage
        text: root._percentage + "%"
        fontSize: Theme.Theme.typography.sizeSm
        color: root._getBatteryColor()
        fontWeight: root._isCritical ? Theme.Theme.typography.weightBold : Theme.Theme.typography.weightNormal
    }

    // Click area
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }

    // Get battery color based on state
    function _getBatteryColor() {
        if (root._isCharging) return Theme.Theme.colors.success
        if (root._isCritical) return Theme.Theme.colors.error
        if (root._isLow) return Theme.Theme.colors.warning
        return Theme.Theme.colors.textSecondary
    }
}
