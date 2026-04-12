// PowerMenu.qml - Power Options Dropdown Molecule
//
// Menu with power options including shutdown, reboot, suspend, and logout.
// Includes confirmation dialogs for destructive actions.
//
// Usage:
//   PowerMenu {
//       onShutdown: PowerManager.shutdown()
//   }
//
// Properties:
//   - showIcons: bool - Show icons in menu items
//
// Signals:
//   - shutdown(): Shutdown requested
//   - reboot(): Reboot requested
//   - suspend(): Suspend requested
//   - logout(): Logout requested

import QtQuick
import QtQuick.Layouts
import "../atoms" as Atoms
import "../services" as Services
import "../theme" as Theme

ColumnLayout {
    id: root

    // Public API
    property bool showIcons: true

    signal shutdown()
    signal reboot()
    signal suspend()
    signal logout()

    // Internal
    property bool _showingConfirm: false
    property string _confirmAction: ""

    // Layout
    spacing: 4

    // Power menu card
    Atoms.Card {
        Layout.fillWidth: true
        padding: 8
        glassmorphic: true
        elevated: true

        ColumnLayout {
            anchors.fill: parent
            spacing: 4

            // Power options
            Repeater {
                model: [
                    { id: "suspend", label: "Suspend", icon: "moon", color: Theme.ThemeEngine.colors.textSecondary },
                    { id: "logout", label: "Log Out", icon: "logout", color: Theme.ThemeEngine.colors.textSecondary },
                    { id: "reboot", label: "Restart", icon: "refresh", color: Theme.ThemeEngine.colors.warning },
                    { id: "shutdown", label: "Shut Down", icon: "power", color: Theme.ThemeEngine.colors.error }
                ]

                Rectangle {
                    Layout.fillWidth: true
                    height: 40
                    radius: Theme.ThemeEngine.radius.small
                    color: mouseArea.containsMouse ? Theme.ThemeEngine.colors.glassHover : "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 12

                        Atoms.Icon {
                            visible: root.showIcons
                            source: modelData.icon
                            size: 18
                            color: modelData.color
                        }

                        Atoms.Label {
                            Layout.fillWidth: true
                            text: modelData.label
                            fontSize: Theme.ThemeEngine.typography.sizeMd
                            color: modelData.color
                        }
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: {
                            if (modelData.id === "shutdown" || modelData.id === "reboot") {
                                root._confirmAction = modelData.id
                                root._showingConfirm = true
                            } else {
                                root._executeAction(modelData.id)
                            }
                        }
                    }
                }
            }
        }
    }

    // Confirmation dialog
    Atoms.Card {
        Layout.fillWidth: true
        visible: root._showingConfirm
        padding: 16
        glassmorphic: true
        elevated: true
        border.color: Theme.ThemeEngine.colors.error

        ColumnLayout {
            anchors.fill: parent
            spacing: 16

            Atoms.Label {
                Layout.fillWidth: true
                text: root._confirmAction === "shutdown" ?
                      "Are you sure you want to shut down?" :
                      "Are you sure you want to restart?"
                fontSize: Theme.ThemeEngine.typography.sizeMd
                color: Theme.ThemeEngine.colors.textPrimary
                wrap: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Atoms.TextButton {
                    text: "Cancel"
                    variant: "tertiary"
                    onClicked: root._showingConfirm = false
                }

                Item { Layout.fillWidth: true }

                Atoms.TextButton {
                    text: root._confirmAction === "shutdown" ? "Shut Down" : "Restart"
                    variant: "primary"
                    destructive: true
                    onClicked: {
                        root._executeAction(root._confirmAction)
                        root._showingConfirm = false
                    }
                }
            }
        }
    }

    // Execute power action
    function _executeAction(action) {
        switch (action) {
            case "shutdown":
                Services.PowerManager.shutdown()
                root.shutdown()
                break
            case "reboot":
                Services.PowerManager.reboot()
                root.reboot()
                break
            case "suspend":
                Services.PowerManager.suspend()
                root.suspend()
                break
            case "logout":
                Services.SessionService.endSession()
                root.logout()
                break
        }
    }
}
