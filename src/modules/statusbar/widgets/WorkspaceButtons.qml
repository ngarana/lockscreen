// WorkspaceButtons.qml - Workspace indicator buttons
//
// Horizontal workspace buttons with active state highlighting.
// Click to switch workspaces.

import QtQuick
import QtQuick.Layouts
import "../../../services" as Services
import "../../../theme" as Theme
import "../../../atoms" as Atoms

RowLayout {
    id: root

    spacing: 4

    property int maxWorkspaces: 10

    readonly property var _workspaces: Services.HyprlandService ? Services.HyprlandService.workspaces : []
    readonly property int _activeId: Services.HyprlandService ? Services.HyprlandService.activeWorkspaceId : 0

    // Hover detector for tooltip (non-blocking)
    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
    }

    Repeater {
        model: Math.max(_workspaces.length, 1)

        Item {
            id: wsItem
            readonly property var _data: _workspaces.length > index ? _workspaces[index] : null
            readonly property int _id: _data ? _data.id : (index + 1)
            readonly property bool _isActive: _id === _activeId
            readonly property bool _hasWindows: _data ? (_data.windows > 0) : false
            readonly property bool _hovered: delegateMouseArea.containsMouse

            Layout.preferredWidth: _isActive ? 20 : (_hasWindows ? 10 : 6)
            Layout.preferredHeight: 6

            Rectangle {
                anchors.fill: parent
                radius: 3
                color: {
                    if (_isActive) return Theme.ThemeEngine.colors.primary
                    if (_hasWindows) return Theme.ThemeEngine.colors.textSecondary
                    if (_hovered) return Theme.ThemeEngine.colors.textMuted
                    return Theme.ThemeEngine.colors.surface1
                }

                Behavior on width {
                    NumberAnimation { duration: Theme.ThemeEngine.animation.fast }
                }
                Behavior on color {
                    ColorAnimation { duration: Theme.ThemeEngine.animation.fast }
                }
            }

            MouseArea {
                id: delegateMouseArea
                anchors.fill: parent
                anchors.margins: -4
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true
                onClicked: {
                    if (Services.HyprlandService) {
                        Services.HyprlandService.switchToWorkspace(_id)
                    }
                }
            }
        }
    }

    Atoms.Tooltip {
        target: root
        visible: hoverArea.containsMouse
        text: "Workspaces"
        position: "bottom"
    }
}
