// StatusMessage.qml - Authentication Status Display Component
//
// Displays authentication status messages to the user.
// Shows different colors for success/error states.
//
// Visual Examples:
//   +-----------------------+
//   | Authenticating...     |  <- Normal (subtle color)
//   +-----------------------+
//
//   +-----------------------+
//   | Authentication failed |  <- Error (red color)
//   +-----------------------+
//
// Features:
// - Dynamic text content
// - Error/success styling
// - Smooth color transitions
// - Hidden when message is empty
// - Theme-aware styling
//
// Usage:
//   StatusMessage {
//       message: "Authentication failed"
//       isError: true
//   }
//
// Properties:
//   message - Status text to display
//   isError - True for error styling, false for normal

import QtQuick
import "../services"

Item {
    id: root

    // ========================================================================
    // Layout
    // ========================================================================

    // Size based on text content with padding
    implicitWidth: statusText.implicitWidth + Theme.spacing.medium * 2
    implicitHeight: statusText.implicitHeight + Theme.spacing.small

    // ========================================================================
    // Public Properties
    // ========================================================================

    // Status message text
    // Set to empty string to hide the message
    property string message: ""

    // Indicates if this is an error message
    // true = Red error color
    // false = Subtle text color
    property bool isError: false

    // ========================================================================
    // Status Text
    // ========================================================================

    Text {
        id: statusText
        anchors.centerIn: parent

        text: root.message
        font.pixelSize: Theme.fonts.textSize

        // Color based on message type
        // Error: Red, distinct from normal text
        // Normal: Subtle, less prominent
        color: root.isError ? Theme.colors.error : Theme.colors.textSubtle

        // Hide when no message
        visible: root.message !== ""

        // Smooth color transition when switching between states
        Behavior on color {
            ColorAnimation {
                duration: Theme.animation.fast
            }
        }
    }
}
