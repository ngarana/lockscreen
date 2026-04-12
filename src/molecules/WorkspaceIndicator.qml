// WorkspaceIndicator.qml - Workspace Dots/Buttons Molecule
//
// Displays workspace indicators with active workspace highlighting.
// Click to switch workspaces. Integrates with HyprlandService.
//
// Usage:
//   WorkspaceIndicator {
//       maxWorkspaces: 10
//   }
//
// Properties:
//   - maxWorkspaces: int - Maximum number of workspaces to show (default: 10)
//   - orientation: int - Qt.Horizontal or Qt.Vertical
//
// Signals:
//   - workspaceClicked(id): Emitted when a workspace is clicked

import QtQuick
import QtQuick.Layouts
import "../atoms" as Atoms
import "../services" as Services
import "../theme" as Theme

RowLayout {
    id: root

    // Public API
    property int maxWorkspaces: 10
    property int orientation: Qt.Horizontal

    signal workspaceClicked(int id)

    // Internal state
    readonly property var _workspaces: Services.HyprlandService.workspaces
    readonly property int _activeId: Services.HyprlandService.activeWorkspaceId

    // Layout
    spacing: 8

    // Workspace indicators
    Repeater {
        model: Math.min(root._workspaces.length, root.maxWorkspaces)

        Rectangle {
            id: workspaceDot
            readonly property var _workspace: root._workspaces[index]
            readonly property int _id: _workspace ? _workspace.id : 0
            readonly property bool _isActive: _id === root._activeId
            readonly property bool _hasWindows: _workspace ? _workspace.windows > 0 : false

            width: root._isActive ? 24 : (_hasWindows ? 8 : 6)
            height: 6
            radius: 3

            color: root._isActive ? Theme.ThemeEngine.colors.primary :
                  _hasWindows ? Theme.ThemeEngine.colors.textSecondary :
                  Theme.ThemeEngine.colors.surface1

            Behavior on width {
                NumberAnimation {
                    duration: Theme.ThemeEngine.animation.fast
                    easing.type: Easing.OutQuad
                }
            }

            Behavior on color {
                ColorAnimation { duration: Theme.ThemeEngine.animation.fast }
            }

            // Click to switch workspace
            MouseArea {
                anchors.fill: parent
                anchors.margins: -4 // Larger click target
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    Services.HyprlandService.switchToWorkspace(workspaceDot._id)
                    root.workspaceClicked(workspaceDot._id)
                }
            }
        }
    }

    // Update when workspace changes
    Connections {
        target: Services.HyprlandService
        function onWorkspaceChanged() {
            // Force update
        }
    }
}
