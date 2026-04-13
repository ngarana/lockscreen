// LockScreen.qml - Modern Lockscreen Widget
//
// Premium lockscreen with aerial video background.
// Default state: clock + power button visible.
// On interaction (hover/keypress): reveals password field + power options.
//
// Layout:
//   +------------------------------------------+
//   |          (Video Background)              |
//   |                                          |
//   |              21:47                       |  <- Always visible
//   |        Wednesday, April 9               |
//   |                                          |
//   |      +------------------------+         |  <- Revealed on interaction
//   |      |  🔒  Enter password   |         |
//   |      +------------------------+         |
//   |        Authentication failed            |
//   |                                          |
//   |    ○ ⏾   ○ ⏻   ○ ↻   ○ ⏼             |  <- Revealed on interaction
//   |                           ○ ⏻           |  <- Always visible (single power btn)
//   +------------------------------------------+

import QtQuick
import QtQuick.Layouts
import "../../services"
import "../../components"
import "../../theme"

Item {
    id: root

    // ========================================================================
    // Signals
    // ========================================================================

    signal unlockRequested()

    // ========================================================================
    // State
    // ========================================================================

    // UI revealed when user interacts (hover/keypress)
    property bool uiRevealed: false
    property bool isUnlocking: false

    // Auto-hide timer: hide UI after inactivity
    Timer {
        id: hideTimer
        interval: 15000
        running: root.uiRevealed && !passwordField.hasFocus
        repeat: false
        onTriggered: {
            if (passwordField.text === "") {
                root.uiRevealed = false
            }
        }
    }

    // ========================================================================
    // Controller Connections
    // ========================================================================

    Connections {
        target: LockController
        enabled: target !== null

        function onUnlockSuccess() {
            root.isUnlocking = true
            root.unlockRequested()
        }

        function onAuthenticationStarted() {
            passwordField.clear()
        }

        function onUnlockFailed(message) {
            passwordField.clear()
            passwordField.setFocus()
        }
    }

    // ========================================================================
    // Layer 0: Video Background
    // ========================================================================

    VideoBackground {
        id: videoBackground
        anchors.fill: parent
    }

    // ========================================================================
    // Layer 1: Dark Overlay (subtle, improves text readability)
    // ========================================================================

    Rectangle {
        anchors.fill: parent
        color: "black"
        opacity: root.uiRevealed ? 0.35 : 0.15

        Behavior on opacity {
            NumberAnimation {
                duration: Theme.animation.reveal
                easing.type: Easing.InOutQuad
            }
        }
    }

    // ========================================================================
    // Interaction Capture
    // ========================================================================
    // Any mouse movement or keypress reveals the UI

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        propagateComposedEvents: true

        onPositionChanged: {
            root.revealUI()
        }

        onClicked: function(mouse) {
            root.revealUI()
            if (root.uiRevealed) {
                passwordField.setFocus()
            }
            mouse.accepted = false
        }
    }

    // Global key capture
    Keys.onPressed: function(event) {
        if (!root.uiRevealed) {
            root.revealUI()
            // Forward printable characters to password field
            if (event.text.length > 0 && !event.modifiers) {
                passwordField.setFocus()
            }
        }
    }

    // ========================================================================
    // Main Content
    // ========================================================================

    Item {
        anchors.fill: parent

        // ====================================================================
        // Clock - Always Visible (upper center)
        // ====================================================================

        Clock {
            id: clock
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height * 0.22
            z: 3

            // Slight upward shift when UI reveals to make room
            Behavior on y {
                NumberAnimation {
                    duration: Theme.animation.reveal
                    easing.type: Easing.InOutQuad
                }
            }
        }

        // ====================================================================
        // Password Field - Revealed on Interaction
        // ====================================================================

        PasswordField {
            id: passwordField
            anchors.horizontalCenter: parent.horizontalCenter
            y: clock.y + clock.height + Theme.spacing.xxlarge
            width: Math.min(parent.width * 0.35, 420)

            opacity: root.uiRevealed ? 1.0 : 0.0
            visible: opacity > 0

            Behavior on opacity {
                NumberAnimation {
                    duration: Theme.animation.reveal
                    easing.type: Easing.InOutQuad
                }
            }

            onSubmitted: function(password) {
                if (LockController) {
                    LockController.authenticate(password)
                }
            }

            onEscaped: {
                clear()
                root.uiRevealed = false
            }
        }

        // ====================================================================
        // Status Message - Below Password (revealed)
        // ====================================================================

        StatusMessage {
            id: statusMessage
            anchors.horizontalCenter: parent.horizontalCenter
            y: passwordField.y + passwordField.height + Theme.spacing.medium
            message: (LockController && LockController.statusMessage) || ""
            isError: LockController && LockController.hasError

            opacity: root.uiRevealed ? 1.0 : 0.0

            Behavior on opacity {
                NumberAnimation {
                    duration: Theme.animation.reveal
                    easing.type: Easing.InOutQuad
                }
            }
        }

        // ====================================================================
        // Audio Controller - Revealed when audio is active
        // ====================================================================

        AudioController {
            id: audioController
            anchors.horizontalCenter: parent.horizontalCenter
            y: statusMessage.y + statusMessage.height + Theme.spacing.medium
            width: Math.min(parent.width - Theme.spacing.xlarge * 2, Theme.audio.maxWidth)

            revealed: root.uiRevealed
            showVolume: true
            showProgress: true
            z: 2
        }

        // ====================================================================
        // Power Buttons Row - Revealed on Interaction
        // ====================================================================

        Row {
            id: powerButtonsRow
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Theme.spacing.xxlarge
            spacing: Theme.spacing.large

            opacity: root.uiRevealed ? 1.0 : 0.0
            visible: opacity > 0

            Behavior on opacity {
                NumberAnimation {
                    duration: Theme.animation.reveal
                    easing.type: Easing.InOutQuad
                }
            }

            ActionButton {
                icon: "⏾"
                label: "Suspend"
                onClicked: { if (PowerManager) PowerManager.suspend() }
            }

            ActionButton {
                icon: "⏻"
                label: "Hibernate"
                onClicked: { if (PowerManager) PowerManager.hibernate() }
            }

            ActionButton {
                icon: "↻"
                label: "Reboot"
                onClicked: { if (PowerManager) PowerManager.reboot() }
            }

            ActionButton {
                icon: "⏼"
                label: "Shutdown"
                onClicked: { if (PowerManager) PowerManager.shutdown() }
            }
        }

        // ====================================================================
        // Single Power Button - Always Visible (bottom right)
        // ====================================================================

        ActionButton {
            id: alwaysVisiblePower
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: Theme.spacing.xlarge
            anchors.bottomMargin: Theme.spacing.xlarge

            icon: "⏻"
            label: "Power"

            // Hide when full power row is visible to avoid duplication
            opacity: root.uiRevealed ? 0.0 : 0.7

            Behavior on opacity {
                NumberAnimation {
                    duration: Theme.animation.reveal
                    easing.type: Easing.InOutQuad
                }
            }

            onClicked: {
                root.revealUI()
            }
        }
    }

    // ========================================================================
    // Methods
    // ========================================================================

    function revealUI() {
        root.uiRevealed = true
        hideTimer.restart()
        passwordField.setFocus()
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        root.forceActiveFocus()
    }
}
