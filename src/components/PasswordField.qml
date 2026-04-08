// PasswordField.qml - Password Input Component
//
// A styled password input field with focus indicators and keyboard shortcuts.
// Designed for use in lockscreen authentication.
//
// Visual Structure:
//   +---------------------------+
//   |    Enter password...      |  <- Placeholder text
//   +---------------------------+
//          (border changes on focus)
//
// Features:
// - Password masking (dots instead of characters)
// - Focus state styling (accent border)
// - Enter key to submit
// - Escape key to clear
// - Theme-aware styling
//
// Usage:
//   PasswordField {
//       placeholderText: "Enter password"
//       onSubmitted: (password) => authenticate(password)
//       onEscaped: clear()
//   }
//
// Signals:
//   submitted(password) - Emitted when Enter is pressed
//   escaped()           - Emitted when Escape is pressed
//
// Methods:
//   clear()  - Clear the password field
//   setFocus() - Focus the field for input

import QtQuick
import QtQuick.Controls
import "../services"

Item {
    id: root

    // ========================================================================
    // Layout
    // ========================================================================

    // Default size for a password field
    implicitWidth: 400
    implicitHeight: 50

    // ========================================================================
    // Public Properties
    // ========================================================================

    // Placeholder text shown when field is empty
    property string placeholderText: "Enter password"

    // Current password text (read/write)
    // Use 'text' property alias to access password value
    property alias text: passwordField.text

    // Indicates if the field has keyboard focus
    // Used to style the border differently when focused
    property bool hasFocus: passwordField.activeFocus

    // ========================================================================
    // Signals
    // ========================================================================

    // Emitted when user presses Enter with non-empty text
    // password: The entered password
    signal submitted(string password)

    // Emitted when user presses Escape
    signal escaped()

    // ========================================================================
    // Public Methods
    // ========================================================================

    // Clear the password field
    // Should be called after failed authentication
    function clear() {
        passwordField.text = ""
    }

    // Focus the password field
    // Call this to ready the field for user input
    function setFocus() {
        passwordField.forceActiveFocus()
    }

    // ========================================================================
    // Container (Background + Border)
    // ========================================================================

    Rectangle {
        id: container
        anchors.fill: parent

        // Background color
        color: Theme.colors.surface
        radius: Theme.radius.medium

        // Border styling changes based on focus state
        // Thicker, accent-colored border when focused
        border.width: root.hasFocus ? 2 : 1
        border.color: root.hasFocus ? Theme.colors.primary : Theme.colors.surfaceHover

        // Smooth border color transition
        Behavior on border.color {
            ColorAnimation {
                duration: Theme.animation.fast
            }
        }
    }

    // ========================================================================
    // Text Field (Input)
    // ========================================================================

    TextField {
        id: passwordField
        anchors.fill: parent
        anchors.margins: Theme.spacing.medium

        // Password masking - shows dots instead of characters
        echoMode: TextInput.Password

        // Placeholder configuration
        placeholderText: root.placeholderText
        font.pixelSize: Theme.fonts.textSize

        // Text colors
        color: Theme.colors.text              // Entered text
        placeholderTextColor: Theme.colors.textMuted  // Placeholder

        // Center the text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        // Transparent background (container provides styling)
        background: null

        // ====================================================================
        // Keyboard Handlers
        // ====================================================================

        // Enter key - Submit password
        onAccepted: {
            if (text.length > 0) {
                root.submitted(text)
            }
        }

        // Escape key - Clear field
        Keys.onEscapePressed: {
            root.escaped()
        }
    }
}
