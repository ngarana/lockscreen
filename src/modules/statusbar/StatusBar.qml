// StatusBar.qml - Main Status Bar Component (macOS-style)
//
// Main status bar component with macOS-inspired layout:
// - Left: Active application name (e.g. "Finder"), optional menu items
// - Right: System tray icons, indicators (network, battery, etc.), clock
//
// Features:
// - Full-width translucent menu bar
// - Compact spacing and subtle separators
// - macOS-style indicator grouping
// - Multi-monitor support
//
// Usage:
//   StatusBar {
//       anchors.fill: parent
//   }

import QtQuick
import QtQuick.Layouts
import Quickshell
import "../../services" as Services
import "../../theme" as Theme

Rectangle {
    id: root

    // ========================================================================
    // Public Properties
    // ========================================================================

    // Bar height (from controller) with default fallback
    property int barHeight: (Services.BarController && Services.BarController.barHeight) || 26

    // Section visibility
    property bool showLeft: true
    property bool showRight: true

    // ========================================================================
    // Visual Configuration
    // ========================================================================

    color: "transparent"
    implicitHeight: root.barHeight

    // ========================================================================
    // Background - Flat Menu Bar
    // ========================================================================

    Rectangle {
        anchors.fill: parent
        color: Qt.alpha(Theme.ThemeEngine.colors.base, 0.48)
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop {
                position: 0.0
                color: Qt.alpha(Theme.ThemeEngine.colors.rosewater, 0.06)
            }
            GradientStop {
                position: 0.35
                color: Qt.alpha(Theme.ThemeEngine.colors.surface0, 0.06)
            }
            GradientStop {
                position: 1.0
                color: Qt.alpha(Theme.ThemeEngine.colors.crust, 0.03)
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 1
        color: Qt.alpha(Theme.ThemeEngine.colors.rosewater, 0.06)
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: Qt.alpha(Theme.ThemeEngine.colors.overlay1, 0.18)
    }

    // ========================================================================
    // Main Layout
    // ========================================================================

    RowLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.topMargin: 1
        anchors.bottomMargin: 1
        spacing: 6

        // ====================================================================
        // Left Section: App name (Finder-style)
        // ====================================================================

        StatusBarLeft {
            id: leftSection
            visible: root.showLeft
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        }

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
        enabled: target !== null
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
        root.visible = Services.BarController && Services.BarController.isVisible
    }
}
