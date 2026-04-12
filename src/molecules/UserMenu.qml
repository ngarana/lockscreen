// UserMenu.qml - User Menu with Avatar Molecule
//
// Menu displaying user avatar, name, and session actions.
// Includes user information and quick session options.
//
// Usage:
//   UserMenu {
//       onSettingsClicked: openSettings()
//   }
//
// Properties:
//   - showAvatar: bool - Show user avatar (default: true)
//
// Signals:
//   - settingsClicked(): Settings action clicked
//   - lockClicked(): Lock screen clicked
//   - logoutClicked(): Logout clicked

import QtQuick
import QtQuick.Layouts
import "../atoms" as Atoms
import "../services" as Services
import "../theme" as Theme

Atoms.Card {
    id: root

    // Public API
    property bool showAvatar: true

    signal settingsClicked()
    signal lockClicked()
    signal logoutClicked()

    // Internal state from service
    readonly property string _userName: Services.SessionService.displayName || Services.SessionService.userName
    readonly property string _avatarPath: Services.SessionService.avatarPath

    // Layout
    implicitWidth: 240
    padding: 16
    glassmorphic: true
    elevated: true

    // Content
    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        // User info row
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            // Avatar
            Rectangle {
                visible: root.showAvatar
                Layout.preferredWidth: 48
                Layout.preferredHeight: 48
                radius: 24
                color: Theme.Theme.colors.surface0

                // Avatar image or initials
                Image {
                    anchors.fill: parent
                    source: root._avatarPath
                    fillMode: Image.PreserveAspectCrop
                    visible: root._avatarPath !== ""
                }

                Atoms.Label {
                    anchors.centerIn: parent
                    visible: root._avatarPath === ""
                    text: root._userName.charAt(0).toUpperCase()
                    fontSize: Theme.Theme.typography.sizeXl
                    color: Theme.Theme.colors.textSecondary
                }
            }

            // User name
            Atoms.Label {
                Layout.fillWidth: true
                text: root._userName
                fontSize: Theme.Theme.typography.sizeMd
                fontWeight: Theme.Theme.typography.weightSemiBold
                color: Theme.Theme.colors.textPrimary
                truncate: true
            }
        }

        Atoms.Divider {
            Layout.fillWidth: true
        }

        // Menu items
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            // Settings
            Rectangle {
                Layout.fillWidth: true
                height: 36
                radius: Theme.Theme.radius.small
                color: settingsMouse.containsMouse ? Theme.Theme.colors.glassHover : "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    spacing: 12

                    Atoms.Icon {
                        source: "settings"
                        size: 18
                        color: Theme.Theme.colors.textSecondary
                    }

                    Atoms.Label {
                        text: "Settings"
                        fontSize: Theme.Theme.typography.sizeMd
                        color: Theme.Theme.colors.textPrimary
                    }
                }

                MouseArea {
                    id: settingsMouse
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    onClicked: root.settingsClicked()
                }
            }

            // Lock
            Rectangle {
                Layout.fillWidth: true
                height: 36
                radius: Theme.Theme.radius.small
                color: lockMouse.containsMouse ? Theme.Theme.colors.glassHover : "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    spacing: 12

                    Atoms.Icon {
                        source: "lock"
                        size: 18
                        color: Theme.Theme.colors.textSecondary
                    }

                    Atoms.Label {
                        text: "Lock"
                        fontSize: Theme.Theme.typography.sizeMd
                        color: Theme.Theme.colors.textPrimary
                    }
                }

                MouseArea {
                    id: lockMouse
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    onClicked: {
                        Services.LockController.lock()
                        root.lockClicked()
                    }
                }
            }

            // Logout
            Rectangle {
                Layout.fillWidth: true
                height: 36
                radius: Theme.Theme.radius.small
                color: logoutMouse.containsMouse ? Theme.Theme.colors.glassHover : "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    spacing: 12

                    Atoms.Icon {
                        source: "logout"
                        size: 18
                        color: Theme.Theme.colors.textSecondary
                    }

                    Atoms.Label {
                        text: "Log Out"
                        fontSize: Theme.Theme.typography.sizeMd
                        color: Theme.Theme.colors.textPrimary
                    }
                }

                MouseArea {
                    id: logoutMouse
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    onClicked: {
                        Services.SessionService.endSession()
                        root.logoutClicked()
                    }
                }
            }
        }
    }
}
