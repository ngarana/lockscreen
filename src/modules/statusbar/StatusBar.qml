// StatusBar.qml - Main Status Bar Component (macOS-style)
//
// Main status bar component with macOS-inspired layout:
// - Left: App launcher, workspace indicators, active window title
// - Center: Clock and date display (floating pill)
// - Right: System tray, network, battery, volume, control center
//
// Features:
// - Glassmorphic floating pill design
// - macOS-style centered clock
// - Configurable sections
// - Multi-monitor support
// - Auto-hide behavior
//
// Usage:
//   StatusBar {
//       anchors.fill: parent
//   }

import QtQuick
import QtQuick.Layouts
import Quickshell
import "../../atoms" as Atoms
import "../../molecules" as Molecules
import "../../components" as Components
import "../../services" as Services
import "../../theme" as Theme

Rectangle {
    id: root

    // ========================================================================
    // Public Properties
    // ========================================================================

    // Bar height (from controller)
    property int barHeight: Services.BarController.barHeight

    // Layout mode
    property string layoutMode: Services.BarController.layoutMode

    // Section visibility
    property bool showLeft: true
    property bool showCenter: Services.BarController.showClock
    property bool showRight: true

    // macOS-style floating pill
    readonly property bool isMacOSStyle: layoutMode === "macos"

    // ========================================================================
    // Visual Configuration
    // ========================================================================

    color: "transparent"

    implicitHeight: root.barHeight

    // ========================================================================
    // Background - macOS floating pill or full-width bar
    // ========================================================================

    Rectangle {
        id: background
        anchors.fill: parent

        // macOS style: floating pill with margins
        anchors.leftMargin: root.isMacOSStyle ? 12 : 0
        anchors.rightMargin: root.isMacOSStyle ? 12 : 0
        anchors.topMargin: root.isMacOSStyle ? 6 : 0
        anchors.bottomMargin: root.isMacOSStyle ? 6 : 0

        radius: root.isMacOSStyle ? Theme.ThemeEngine.radius.large : 0

        color: Theme.ThemeEngine.colors.surface0
        opacity: 0.85

        // Glassmorphic border
        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: parent.radius
            color: "transparent"
            border.width: 1
            border.color: Theme.ThemeEngine.colors.glassBorder
            opacity: 0.5
        }
    }

    // ========================================================================
    // Main Layout
    // ========================================================================

    RowLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.leftMargin: root.isMacOSStyle ? 16 : 12
        anchors.rightMargin: root.isMacOSStyle ? 16 : 12
        spacing: 8

        // ====================================================================
        // Left Section: App launcher, workspaces, window title
        // ====================================================================

        StatusBarLeft {
            id: leftSection
            visible: root.showLeft
            Layout.fillWidth: root.layoutMode === "windows"
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        }

        // Spacer for macOS layout (pushes center to middle)
        Item {
            Layout.fillWidth: root.layoutMode === "macos"
            visible: root.layoutMode === "macos"
        }

        // ====================================================================
        // Center Section: Clock and date (centered in macOS mode)
        // ====================================================================

        StatusBarCenter {
            id: centerSection
            visible: root.showCenter
            Layout.alignment: Qt.AlignCenter
        }

        // Spacer for macOS layout
        Item {
            Layout.fillWidth: root.layoutMode === "macos"
            visible: root.layoutMode === "macos"
        }

        // ====================================================================
        // Right Section: System tray, indicators, control center
        // ====================================================================

        StatusBarRight {
            id: rightSection
            visible: root.showRight
            Layout.fillWidth: root.layoutMode === "windows"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        }
    }

    // ========================================================================
    // Auto-hide Timer
    // ========================================================================

    Timer {
        id: autoHideTimer
        interval: Services.BarController.autoHideDelay
        running: Services.BarController.autoHide && !Services.BarController.isHovered
        repeat: false
        onTriggered: {
            Services.BarController.hide()
        }
    }

    // ========================================================================
    // Hover Area for Auto-hide
    // ========================================================================

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: Services.BarController.autoHide
        propagateComposedEvents: true

        onContainsMouseChanged: {
            Services.BarController.isHovered = containsMouse
            if (containsMouse) {
                Services.BarController.show()
                autoHideTimer.restart()
            }
        }
    }

    // ========================================================================
    // Controller Connections
    // ========================================================================

    Connections {
        target: Services.BarController
        function onVisibilityChanged(visible) {
            root.visible = visible
        }
        function onHeightChanged(height) {
            root.barHeight = height
        }
        function onLayoutModeChanged(mode) {
            root.layoutMode = mode
        }
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        root.visible = Services.BarController.visible
    }
}
