// LockScreen.qml - Main Lockscreen UI Widget
//
// The primary lockscreen interface combining all components.
// Displays clock, password field, status, and power buttons.
//
// Visual Layout:
//   +------------------------------------------+
//   |                                          |
//   |              14:32                       |  <- Clock
//   |         Wednesday, April 9               |
//   |                                          |
//   |      +--------------------+              |
//   |      |  Enter password... |              |  <- Password Field
//   |      +--------------------+              |
//   |                                          |
//   |       Authenticating...                  |  <- Status Message
//   |                                          |
//   |  +------+ +-------+ +------+ +--------+  |
//   |  | ⏾   | | ⏻    | | ↻   | | ⏼    |      |  <- Power Buttons
//   |  |Suspnd| |Hybrnt| |Rebt| |Shutdn|      |
//   |  +------+ +-------+ +------+ +--------+  |
//   |                                          |
//   +------------------------------------------+
//
// Features:
// - Centered layout with responsive sizing
// - Clock with time and date
// - Password input with authentication
// - Status message display
// - Power action buttons (suspend, hibernate, reboot, shutdown)
// - Click anywhere to focus password field
//
// Architecture:
// - Uses LockController for authentication
// - Uses PowerManager for power actions
// - Uses Theme for styling
// - Composes Clock, PasswordField, StatusMessage, ActionButton
//
// Usage:
//   LockScreen {
//       anchors.fill: parent
//       onUnlockRequested: sessionLock.locked = false
//   }
//
// Signals:
//   unlockRequested - Emitted when authentication succeeds

import QtQuick
import QtQuick.Layouts
import "../services"
import "../components"

Rectangle {
    id: root

    // ========================================================================
    // Signals
    // ========================================================================

    // Emitted when authentication succeeds
    // Parent should set WlSessionLock.locked = false in response
    signal unlockRequested()

    // ========================================================================
    // Visual Configuration
    // ========================================================================

    // Background color from theme
    color: Theme.colors.background

    // ========================================================================
    // Internal State
    // ========================================================================

    // Tracks if currently unlocking (for animation/state purposes)
    property bool isUnlocking: false

    // ========================================================================
    // Controller Connections
    // ========================================================================
    // React to authentication events from LockController

    Connections {
        target: LockController

        // Authentication succeeded
        function onUnlockSuccess() {
            root.isUnlocking = true
            root.unlockRequested()
        }

        // Authentication started - clear field for new attempt
        function onAuthenticationStarted() {
            passwordField.clear()
        }

        // Authentication failed - clear and refocus for retry
        function onUnlockFailed(message) {
            passwordField.clear()
            passwordField.setFocus()
        }
    }

    // ========================================================================
    // Background Click Handler
    // ========================================================================
    // Allow clicking anywhere to focus the password field

    MouseArea {
        anchors.fill: parent
        onClicked: passwordField.setFocus()
    }

    // ========================================================================
    // Main Content Layout
    // ========================================================================

    ColumnLayout {
        id: mainLayout
        anchors.centerIn: parent

        // Responsive width: 40% of parent, max 400px
        width: Math.min(parent.width * 0.4, 400)

        // Generous spacing between elements
        spacing: Theme.spacing.xlarge

        // ====================================================================
        // Clock Display
        // ====================================================================

        Clock {
            id: clock
            Layout.alignment: Qt.AlignHCenter
        }

        // Visual separator
        Item {
            Layout.preferredHeight: Theme.spacing.large
        }

        // ====================================================================
        // Password Input
        // ====================================================================

        PasswordField {
            id: passwordField
            Layout.fillWidth: true
            Layout.preferredHeight: implicitHeight

            // Submit password to LockController on Enter
            onSubmitted: function(password) {
                LockController.authenticate(password)
            }

            // Clear field on Escape
            onEscaped: {
                clear()
            }

            // Focus field on creation
            Component.onCompleted: {
                setFocus()
            }
        }

        // ====================================================================
        // Status Message
        // ====================================================================

        StatusMessage {
            Layout.alignment: Qt.AlignHCenter
            message: LockController.statusMessage
            isError: LockController.hasError
            visible: LockController.statusMessage !== ""
        }

        // Visual separator
        Item {
            Layout.preferredHeight: Theme.spacing.large
        }

        // ====================================================================
        // Power Action Buttons
        // ====================================================================

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Theme.spacing.medium

            // Suspend button - Sleep mode, fast resume
            ActionButton {
                icon: "⏾"
                label: "Suspend"
                onClicked: PowerManager.suspend()
            }

            // Hibernate button - Suspend to disk
            ActionButton {
                icon: "⏻"
                label: "Hibernate"
                onClicked: PowerManager.hibernate()
            }

            // Reboot button - Restart system
            ActionButton {
                icon: "↻"
                label: "Reboot"
                onClicked: PowerManager.reboot()
            }

            // Shutdown button - Power off
            ActionButton {
                icon: "⏼"
                label: "Shutdown"
                onClicked: PowerManager.shutdown()
            }
        }
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        // Ensure password field has focus on load
        passwordField.setFocus()
    }
}
