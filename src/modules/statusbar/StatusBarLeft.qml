// StatusBarLeft.qml - Left section of the macOS-style status bar
//
// Shows the system menu entry, active application name, and the
// standard top-level menu labels for the focused app.

import QtQuick
import QtQuick.Layouts
import "." as StatusBarModule
import "../../services" as Services
import "../../theme" as Theme

RowLayout {
    id: root

    property string activeAppName: "Desktop"
    property string menuItemsCsv: "File|Edit|View|Go|Window|Help"

    signal logoClicked()
    signal menuClicked(string menuName)

    spacing: 0

    StatusBarModule.MenuBarButton {
        leadingIcon: Theme.ThemeEngine.icons.grid
        label: "Qypr"
        tooltip: "Open launcher"
        textWeight: Theme.ThemeEngine.typography.weightBold
        textSize: 11
        horizontalPadding: 6
        contentSpacing: 3
        iconSize: 11
        itemHeight: 20
        onClicked: root.logoClicked()
    }

    StatusBarModule.MenuBarButton {
        label: root.activeAppName
        tooltip: "Focused application"
        textWeight: Theme.ThemeEngine.typography.weightBold
        textSize: 11
        horizontalPadding: 7
        itemHeight: 20
        onClicked: root.menuClicked(root.activeAppName)
    }

    Repeater {
        model: root.menuItemsCsv.split("|")

        StatusBarModule.MenuBarButton {
            label: modelData
            tooltip: modelData
            showBackground: true
            textSize: 11
            horizontalPadding: 6
            itemHeight: 20
            onClicked: root.menuClicked(modelData)
        }
    }

    function _sanitizeAppName(value) {
        const rawValue = value ? String(value).trim() : ""
        if (rawValue === "") {
            return "Desktop"
        }

        const segments = rawValue.split(" - ")
        const candidate = segments.length > 1 ? segments[segments.length - 1].trim() : rawValue
        return candidate !== "" ? candidate : "Desktop"
    }

    function _updateActiveAppName() {
        if (!Services.HyprlandService || !Services.HyprlandService.activeWindow) {
            root.activeAppName = "Desktop"
            return
        }

        const activeWindow = Services.HyprlandService.activeWindow
        if (activeWindow.class && String(activeWindow.class).trim() !== "") {
            root.activeAppName = _sanitizeAppName(activeWindow.class)
            return
        }

        root.activeAppName = _sanitizeAppName(activeWindow.title)
    }

    Connections {
        target: Services.HyprlandService
        enabled: target !== null

        function onWindowFocusChanged() {
            root._updateActiveAppName()
        }

        function onWindowTitleChanged() {
            root._updateActiveAppName()
        }

        function onWindowOpened() {
            root._updateActiveAppName()
        }

        function onWindowClosed() {
            root._updateActiveAppName()
        }
    }

    Component.onCompleted: {
        root._updateActiveAppName()
    }
}
