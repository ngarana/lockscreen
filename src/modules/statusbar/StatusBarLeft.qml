// StatusBarLeft.qml - Left Section of Status Bar
//
// Left section showing active application name (macOS "Finder" style).
// Displays the Apple logo, active app name, and macOS-style menus.
//

import QtQuick
import QtQuick.Layouts
import "../../atoms" as Atoms
import "../../services" as Services
import "../../theme" as Theme

RowLayout {
    id: root

    spacing: 16

    // ========================================================================
    // Apple Logo
    // ========================================================================

    Text {
        text: ""
        font.pixelSize: 15
        color: Theme.ThemeEngine.colors.textPrimary
        Layout.alignment: Qt.AlignVCenter
        Layout.leftMargin: 8
    }

    // ========================================================================
    // Active Application Name (macOS "Finder" style)
    // ========================================================================

    Atoms.Label {
        id: appNameLabel
        text: root.activeAppName
        fontSize: Theme.ThemeEngine.typography.sizeSm
        fontWeight: Theme.ThemeEngine.typography.weightBold
        color: Theme.ThemeEngine.colors.textPrimary

        Layout.alignment: Qt.AlignVCenter
    }

    // ========================================================================
    // Mac Menu Items
    // ========================================================================

    RowLayout {
        spacing: 14
        Layout.alignment: Qt.AlignVCenter

        Repeater {
            model: ["File", "Edit", "View", "Go", "Window", "Help"]
            Atoms.Label {
                text: modelData
                fontSize: Theme.ThemeEngine.typography.sizeSm
                color: Theme.ThemeEngine.colors.textPrimary
                fontWeight: Theme.ThemeEngine.typography.weightRegular
            }
        }
    }

    // ========================================================================
    // Public Properties
    // ========================================================================

    // Active app name (from HyprlandService)
    readonly property string activeAppName: {
        if (Services.HyprlandService.activeWindow &&
            Services.HyprlandService.activeWindow.title) {
            const title = Services.HyprlandService.activeWindow.title
            const parts = title.split(" - ")
            if (parts.length > 1) {
                return parts[parts.length - 1]
            }
            return title
        }
        return "Finder"
    }

    // ========================================================================
    // Hyprland Service Connections
    // ========================================================================

    Connections {
        target: Services.HyprlandService
        function onWindowFocusChanged(window) {
            // Force update of app name
        }
        function onWindowTitleChanged(window) {
            // Force update of app name
        }
    }
}
