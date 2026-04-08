// ActionButton.qml - Power Action Button Component
//
// A styled button for power actions (suspend, reboot, shutdown).
// Features hover effects and an icon + label layout.
//
// Visual Structure:
//   +----------+
//   |   (⏻)   |  <- Icon (large)
//   | Shutdown |  <- Label (smaller)
//   +----------+
//
// Features:
// - Icon + label vertical layout
// - Hover state styling
// - Smooth color transitions
// - Pointing hand cursor
// - Theme-aware styling
//
// Usage:
//   ActionButton {
//       icon: "⏻"
//       label: "Shutdown"
//       onClicked: PowerManager.shutdown()
//   }
//
// Properties:
//   icon - Unicode symbol or text for the icon
//   label - Button label text
//   hovered - Read-only, true when mouse is over button
//
// Signals:
//   clicked - Emitted when button is clicked

import QtQuick
import "../services"

Rectangle {
    id: root

    // ========================================================================
    // Layout
    // ========================================================================

    // Square button size
    implicitWidth: 80
    implicitHeight: 80

    // ========================================================================
    // Public Properties
    // ========================================================================

    // Icon text/symbol (Unicode characters work well)
    // Examples: "⏻" (power), "↻" (reboot), "⏾" (suspend)
    property string icon: ""

    // Button label text
    // Short labels work best (e.g., "Shutdown", "Reboot")
    property string label: ""

    // Indicates if mouse is hovering over the button
    // Read-only, used internally for styling
    property bool hovered: mouseArea.containsMouse

    // ========================================================================
    // Signals
    // ========================================================================

    // Emitted when the button is clicked
    signal clicked()

    // ========================================================================
    // Styling
    // ========================================================================

    // Background color changes on hover
    color: hovered ? Theme.colors.surfaceHover : Theme.colors.surface
    radius: Theme.radius.large
    border.width: 2
    border.color: hovered ? Theme.colors.primary : Theme.colors.surfaceHover

    // Smooth color transitions for hover effect
    Behavior on color {
        ColorAnimation {
            duration: Theme.animation.fast
        }
    }

    Behavior on border.color {
        ColorAnimation {
            duration: Theme.animation.fast
        }
    }

    // ========================================================================
    // Content Layout
    // ========================================================================

    Column {
        anchors.centerIn: parent
        spacing: Theme.spacing.small

        // Icon - Larger text
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.icon
            font.pixelSize: Theme.fonts.textSizeLarge  // 24px
            font.family: "Noto Sans"  // Ensures Unicode symbols render
            color: Theme.colors.text
        }

        // Label - Smaller text
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.label
            font.pixelSize: Theme.fonts.textSize - 4  // 12px
            color: Theme.colors.textSubtle
        }
    }

    // ========================================================================
    // Mouse Interaction
    // ========================================================================

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        // Enable hover detection for styling changes
        hoverEnabled: true

        // Show pointing hand cursor to indicate clickability
        cursorShape: Qt.PointingHandCursor

        // Emit clicked signal on mouse press
        onClicked: root.clicked()
    }
}
