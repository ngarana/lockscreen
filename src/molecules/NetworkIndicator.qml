// NetworkIndicator.qml - Network Status Icon + Menu Molecule
//
// Displays network status with icon and signal strength.
// Click to expand menu with network options.
//
// Usage:
//   NetworkIndicator {
//       onMenuRequested: showNetworkMenu()
//   }
//
// Properties:
//   - showLabel: bool - Show network name label
//
// Signals:
//   - clicked(): Emitted when indicator is clicked
//   - menuRequested(): Emitted to request menu expansion

import QtQuick
import QtQuick.Layouts
import "../atoms" as Atoms
import "../services" as Services
import "../theme" as Theme

RowLayout {
    id: root

    // Public API
    property bool showLabel: false

    signal clicked()
    signal menuRequested()

    // Internal state from service
    readonly property bool _isConnected: Services.NetworkService.isConnected
    readonly property string _ssid: Services.NetworkService.ssid
    readonly property int _signalStrength: Services.NetworkService.signalStrength
    readonly property string _connectionType: Services.NetworkService.connectionType

    // Layout
    spacing: 6

    // Network icon
    Atoms.Icon {
        id: networkIcon
        source: Services.NetworkService.getIconName()
        size: 20
        color: root._isConnected ? Theme.Theme.colors.textPrimary : Theme.Theme.colors.textMuted
    }

    // Signal strength indicator (WiFi only)
    Rectangle {
        id: signalIndicator
        visible: root._isConnected && root._connectionType === "wifi"
        width: 16
        height: 12
        color: "transparent"

        // Draw signal bars
        Row {
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 2

            Repeater {
                model: 4
                Rectangle {
                    width: 3
                    height: 3 + (index * 3)
                    radius: 1
                    color: index < root._getSignalBars() ?
                           Theme.Theme.colors.textPrimary : Theme.Theme.colors.surface1
                }
            }
        }
    }

    // Network name label
    Atoms.Label {
        id: ssidLabel
        visible: root.showLabel && root._isConnected
        text: root._ssid
        fontSize: Theme.Theme.typography.sizeSm
        color: Theme.Theme.colors.textSecondary
    }

    // Click area
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            root.clicked()
            root.menuRequested()
        }
    }

    // Update icon when service changes
    Connections {
        target: Services.NetworkService
        function onNetworkStatusChanged() {
            networkIcon.source = Services.NetworkService.getIconName()
        }
    }

    // Helper to get number of signal bars
    function _getSignalBars() {
        if (root._signalStrength >= 80) return 4
        if (root._signalStrength >= 60) return 3
        if (root._signalStrength >= 40) return 2
        if (root._signalStrength > 0) return 1
        return 0
    }
}
