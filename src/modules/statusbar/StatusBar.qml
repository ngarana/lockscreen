// StatusBar.qml - Horizontal status bar visual component (outfoxxed-style)
//
// Glassmorphic horizontal bar with compact state for fullscreen workspaces.
// Contains workspace buttons, system indicators, and clock.
//
// Features:
// - Full-width horizontal layout
// - Glassmorphic styling with theme colors
// - Compact state animation for fullscreen workspaces
// - Default property for adding bar items
//
// Usage:
//   PanelWindow {
//       StatusBar { anchors.fill: parent }
//   }

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Quickshell.Hyprland
import "../../services" as Services
import "../../theme" as Theme
import "../../atoms" as Atoms
import "../../molecules" as Molecules
import "./widgets" as Widgets

Item {
    id: root

    default property alias barItems: containment.data

    property real baseHeight: 55
    property real compactMargin: root.compactState * 10

    readonly property bool isFullscreenWorkspace: {
        if (!Services.HyprlandService) return false
        try {
            const monitor = Hyprland.monitorFor(Services.HyprlandService.currentScreen)
            return monitor?.activeWorkspace?.hasFullscreen ?? false
        } catch(e) {
            return false
        }
    }

    property real compactState: isFullscreenWorkspace ? 0 : 1

    Behavior on compactState {
        NumberAnimation {
            duration: 600
            easing.type: Easing.BezierSpline
            easing.bezierCurve: [0.0, 0.75, 0.15, 1.0, 1.0, 1.0]
        }
    }

    Rectangle {
        id: barRect

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            topMargin: root.compactMargin
            bottomMargin: root.compactMargin
        }

        height: parent.height - (root.compactMargin * 2)

        color: Qt.alpha(Theme.ThemeEngine.colors.surface0, 0.85)
        radius: root.compactState * 5

        border.width: 1
        border.color: Qt.alpha(Theme.ThemeEngine.colors.overlay0, 0.3)

        Behavior on color {
            ColorAnimation { duration: Theme.ThemeEngine.animation.fast }
        }

        RowLayout {
            id: containment
            anchors {
                fill: parent
                leftMargin: 10
                rightMargin: 10
                topMargin: 5
                bottomMargin: 5
            }
            spacing: 16

            Widgets.WorkspaceButtons {
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Widgets.SystemIndicators {
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }

            Molecules.Clock {
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.leftMargin: 16
                fontSize: 12
                showDate: false
                format24Hour: true
                showSeconds: false
            }
        }
    }
}