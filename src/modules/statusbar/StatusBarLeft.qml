// StatusBarLeft.qml - Left Section of Status Bar
//
// Left section containing:
// - App launcher button (Apple menu style)
// - Workspace indicators (Hyprland workspaces)
// - Active window title (optional)
//
// macOS-style layout with icon-based launcher and compact workspace dots.

import QtQuick
import QtQuick.Layouts
import "../../atoms" as Atoms
import "../../molecules" as Molecules
import "../../services" as Services
import "../../theme" as Theme

RowLayout {
    id: root

    spacing: 10

    // ========================================================================
    // App Launcher Button
    // ========================================================================

    // macOS-style app launcher button with icon
    Rectangle {
        id: launcherButton
        color: "transparent"
        radius: Theme.Theme.radius.small

        property bool hovered: launcherMouse.containsMouse
        property bool pressed: false

        Layout.preferredWidth: 28
        Layout.preferredHeight: 28

        Text {
            anchors.centerIn: parent
            text: Theme.Theme.icons.applications
            font.pixelSize: 18
            color: launcherButton.hovered ? Theme.Theme.colors.textPrimary : Theme.Theme.colors.textSecondary
        }

        MouseArea {
            id: launcherMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onPressed: launcherButton.pressed = true
            onReleased: {
                launcherButton.pressed = false
                if (containsMouse) {
                    // TODO: Trigger launcher visibility toggle
                    // This will be connected when Launcher module is implemented
                }
            }
        }
    }

    Divider {
        Layout.preferredWidth: 1
        Layout.preferredHeight: 18
        color: Theme.Theme.colors.glassBorder
    }

    // ========================================================================
    // Workspace Indicators
    // ========================================================================

    Molecules.WorkspaceIndicator {
        id: workspaceIndicator
        visible: Services.BarController.showWorkspaceIndicator
        maxWorkspaces: 10

        Layout.alignment: Qt.AlignVCenter
    }

    // ========================================================================
    // Active Window Title (Optional)
    // ========================================================================

    Rectangle {
        id: windowTitleContainer
        visible: root.showWindowTitle
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignVCenter
        Layout.preferredHeight: 20
        color: "transparent"

        Atoms.Label {
            id: windowTitle
            anchors.fill: parent
            text: root.activeWindowTitle
            fontSize: Theme.Theme.typography.sizeSm
            color: Theme.Theme.colors.textSecondary
            truncate: true
            horizontalAlignment: Text.AlignLeft
        }
    }

    // ========================================================================
    // Public Properties
    // ========================================================================

    // Show active window title
    property bool showWindowTitle: false

    // Active window title (from HyprlandService)
    readonly property string activeWindowTitle: {
        if (Services.HyprlandService.activeWindow &&
            Services.HyprlandService.activeWindow.title) {
            return Services.HyprlandService.activeWindow.title
        }
        return ""
    }

    // ========================================================================
    // Divider Component
    // ========================================================================

    component Divider: Rectangle {
        color: Theme.Theme.colors.surface1
        radius: 1
    }

    // ========================================================================
    // Hyprland Service Connections
    // ========================================================================

    Connections {
        target: Services.HyprlandService
        function onWindowFocusChanged(window) {
            // Force update of window title
        }
        function onWindowTitleChanged(window) {
            // Force update of window title
        }
    }
}
