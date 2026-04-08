// PasswordField.qml - Glassmorphic Password Input
//
// Modern pill-shaped password field with frosted glass effect.
// Features focus glow, smooth animations, and text shadow.

import QtQuick
import QtQuick.Controls
import "../services"

Item {
    id: root

    implicitWidth: 420
    implicitHeight: 56

    property string placeholderText: "Enter password..."
    property alias text: passwordField.text
    property bool hasFocus: passwordField.activeFocus

    signal submitted(string password)
    signal escaped()

    function clear() {
        passwordField.text = ""
    }

    function setFocus() {
        passwordField.forceActiveFocus()
    }

    // ========================================================================
    // Glassmorphic Container
    // ========================================================================

    Rectangle {
        id: container
        anchors.fill: parent

        color: Theme.colors.glass
        radius: Theme.radius.round
        border.width: 1
        border.color: root.hasFocus ? Theme.colors.primary : Theme.colors.glassBorder

        Behavior on border.color {
            ColorAnimation { duration: Theme.animation.medium }
        }

        // Focus glow effect
        Rectangle {
            anchors.fill: parent
            anchors.margins: -3
            radius: parent.radius + 3
            color: "transparent"
            border.width: root.hasFocus ? 2 : 0
            border.color: Theme.colors.primaryGlow
            opacity: root.hasFocus ? 1.0 : 0.0

            Behavior on opacity {
                NumberAnimation { duration: Theme.animation.medium }
            }
        }
    }

    // ========================================================================
    // Lock Icon
    // ========================================================================

    Text {
        id: lockIcon
        anchors.left: parent.left
        anchors.leftMargin: Theme.spacing.large
        anchors.verticalCenter: parent.verticalCenter
        text: "🔒"
        font.pixelSize: Theme.fonts.textSize
        opacity: 0.7
    }

    // ========================================================================
    // Text Field
    // ========================================================================

    TextField {
        id: passwordField
        anchors.fill: parent
        anchors.leftMargin: lockIcon.width + Theme.spacing.large + Theme.spacing.medium
        anchors.rightMargin: Theme.spacing.large

        echoMode: TextInput.Password
        placeholderText: root.placeholderText
        font.pixelSize: Theme.fonts.textSize
        font.family: Theme.fonts.fontFamily

        color: Theme.colors.text
        placeholderTextColor: Theme.colors.textMuted

        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter

        background: null

        onAccepted: {
            if (text.length > 0) {
                root.submitted(text)
            }
        }

        Keys.onEscapePressed: {
            root.escaped()
        }
    }
}
