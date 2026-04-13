// StatusBarLeft.qml - Left Section of Status Bar
//
// Left section showing active application name (macOS "Finder" style).
// Displays the Apple logo, active app name, and macOS-style menus.
//
// Signals:
//  - menuClicked(string name) - Emitted when menu item is clicked
//  - logoClicked() - Emitted when Apple logo is clicked

import QtQuick
import QtQuick.Layouts
import "../../atoms" as Atoms
import "../../services" as Services
import "../../theme" as Theme

RowLayout {
    id: root

    spacing: 16

    // ========================================================================
    // Public Properties
    // ========================================================================

    // Active app name (from HyprlandService)
    property string activeAppName: "Finder"

    // ========================================================================
    // Signals
    // ========================================================================

    signal logoClicked()
    signal menuClicked(string menuName)

    // ========================================================================
    // Hyprland Logo
    // ========================================================================

    Rectangle {
        id: logoRect
        implicitWidth: 32
        implicitHeight: 28
        radius: Theme.ThemeEngine.radius.small
        Layout.alignment: Qt.AlignVCenter
        Layout.leftMargin: 4
        
        property bool _isHovered: logoMouseArea.containsMouse
        property bool _isPressed: logoMouseArea.pressed
        
        color: _isPressed ? Theme.ThemeEngine.colors.glassActive :
               _isHovered ? Theme.ThemeEngine.colors.glassHover : "transparent"
               
        Behavior on color { ColorAnimation { duration: Theme.ThemeEngine.animation.fast } }
        
        scale: _isPressed ? 0.95 : 1.0
        Behavior on scale { NumberAnimation { duration: Theme.ThemeEngine.animation.fast; easing.type: Easing.OutQuad } }
        
        Text {
            id: logoText
            text: Theme.ThemeEngine.icons.applications
            font.family: Theme.ThemeEngine.fonts.iconFontFamily
            font.pixelSize: 16
            color: Theme.ThemeEngine.colors.primary
            anchors.centerIn: parent
        }

        MouseArea {
            id: logoMouseArea
            anchors.fill: parent
            hoverEnabled: true

            onClicked: root.logoClicked()

            cursorShape: Qt.PointingHandCursor
        }
    }

    // ========================================================================
    // Active Application Name (macOS "Finder" style)
    // ========================================================================

    Rectangle {
        id: appNameRect
        implicitWidth: Math.min(appNameLabel.implicitWidth + 16, 180)
        implicitHeight: 28
        radius: Theme.ThemeEngine.radius.small
        Layout.alignment: Qt.AlignVCenter
        Layout.maximumWidth: 180
        
        property bool _isHovered: appNameMouseArea.containsMouse
        property bool _isPressed: appNameMouseArea.pressed
        
        color: _isPressed ? Theme.ThemeEngine.colors.glassActive :
               _isHovered ? Theme.ThemeEngine.colors.glassHover : "transparent"
               
        Behavior on color { ColorAnimation { duration: Theme.ThemeEngine.animation.fast } }
        
        scale: _isPressed ? 0.98 : 1.0
        Behavior on scale { NumberAnimation { duration: Theme.ThemeEngine.animation.fast; easing.type: Easing.OutQuad } }

        Atoms.Label {
            id: appNameLabel
            text: root.activeAppName
            fontSize: Theme.ThemeEngine.typography.sizeSm
            fontWeight: Theme.ThemeEngine.typography.weightBold
            color: Theme.ThemeEngine.colors.textPrimary
            truncate: true
            anchors.centerIn: parent
            width: parent.width - 16
            horizontalAlignment: Text.AlignHCenter
        }
        
        MouseArea {
            id: appNameMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: root.menuClicked(root.activeAppName)
            cursorShape: Qt.PointingHandCursor
        }
    }

    // ========================================================================
    // Mac Menu Items
    // ========================================================================

    RowLayout {
        id: menuBar
        spacing: 4
        Layout.alignment: Qt.AlignVCenter

        Repeater {
            model: ["File", "Edit", "View", "Go", "Window", "Help"]

            Rectangle {
                id: menuItemRect
                implicitWidth: menuItem.implicitWidth + 16
                implicitHeight: 28
                radius: Theme.ThemeEngine.radius.small
                
                property bool _isHovered: mouseArea.containsMouse
                property bool _isPressed: mouseArea.pressed
                
                color: _isPressed ? Theme.ThemeEngine.colors.glassActive :
                       _isHovered ? Theme.ThemeEngine.colors.glassHover : "transparent"
                       
                Behavior on color { ColorAnimation { duration: Theme.ThemeEngine.animation.fast } }
                
                scale: _isPressed ? 0.98 : 1.0
                Behavior on scale { NumberAnimation { duration: Theme.ThemeEngine.animation.fast; easing.type: Easing.OutQuad } }

                Atoms.Label {
                    id: menuItem
                    text: modelData
                    fontSize: Theme.ThemeEngine.typography.sizeSm
                    fontWeight: Theme.ThemeEngine.typography.weightMedium
                    color: Theme.ThemeEngine.colors.textPrimary
                    anchors.centerIn: parent
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked: root.menuClicked(modelData)
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }
    }

    // ========================================================================
    // Hyprland Service Connections
    // ========================================================================

    function _updateActiveAppName() {
        if (Services.HyprlandService && Services.HyprlandService.activeWindow &&
            Services.HyprlandService.activeWindow.title) {
            const title = Services.HyprlandService.activeWindow.title
            const parts = title.split(" - ")
            if (parts.length > 1) {
                root.activeAppName = parts[parts.length - 1]
            } else {
                root.activeAppName = title
            }
        } else {
            root.activeAppName = "Finder"
        }
    }

    Connections {
        target: Services.HyprlandService
        enabled: target !== null

        function onWindowFocusChanged(window) {
            _updateActiveAppName()
        }

        function onWindowTitleChanged(window) {
            _updateActiveAppName()
        }

        function onWindowOpened(window) {
            _updateActiveAppName()
        }

        function onWindowClosed(window) {
            _updateActiveAppName()
        }
    }

    Component.onCompleted: {
        _updateActiveAppName()
    }
}
