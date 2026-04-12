// StatusBar.qml - Main Status Bar Component (macOS-style)
//
// Main status bar component with macOS-inspired layout:
// - Left: Active application name (e.g. "Finder"), optional menu items
// - Right: System tray icons, indicators (network, battery, etc.), clock
//
// Features:
// - Full-width flat bar (no floating pill)
// - Compact spacing, minimal padding
// - macOS-style right-to-left indicator order
// - Multi-monitor support
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

    // Section visibility
    property bool showLeft: true
    property bool showRight: true

    // ========================================================================
    // Visual Configuration
    // ========================================================================

    color: "transparent"
    implicitHeight: root.barHeight

    // ========================================================================
    // Background - macOS-style frosted glass (like PasswordField)
    // ========================================================================

    Rectangle {
        id: background
        anchors.fill: parent
        color: Theme.ThemeEngine.colors.glass
        radius: 0
        border.width: 0

        // Subtle bottom border line
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: Theme.ThemeEngine.colors.glassBorder
            opacity: 0.4
        }

        // Top highlight gradient (frosted glass reflection)
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height * 0.4
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.06) }
                GradientStop { position: 1.0; color: Qt.rgba(1, 1, 1, 0.0) }
            }
            radius: 0
            z: 1
        }
    }

    // ========================================================================
    // Main Layout
    // ========================================================================

    RowLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 6

        // ====================================================================
        // Left Section: App name (Finder-style)
        // ====================================================================

        StatusBarLeft {
            id: leftSection
            visible: root.showLeft
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        }

        // Spacer - pushes everything else to the right
        Item {
            Layout.fillWidth: true
        }

        // ====================================================================
        // Right Section: System tray, indicators, clock
        // ====================================================================

        StatusBarRight {
            id: rightSection
            visible: root.showRight
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        }
    }

    // ========================================================================
    // Controller Connections
    // ========================================================================

    Connections {
        target: Services.BarController
        function onBarVisibilityChanged(visible) {
            root.visible = visible
        }
        function onHeightUpdated(height) {
            root.barHeight = height
        }
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        root.visible = Services.BarController.isVisible
    }
}
