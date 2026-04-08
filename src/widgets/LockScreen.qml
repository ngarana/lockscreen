import QtQuick
import QtQuick.Layouts
import "../services"
import "../components"

Rectangle {
    id: root

    color: Theme.colors.background

    property bool isUnlocking: false

    Connections {
        target: LockController
        function onUnlockSuccess() {
            root.isUnlocking = true
        }
        function onAuthenticationStarted() {
            passwordField.clear()
        }
        function onUnlockFailed(message) {
            passwordField.clear()
            passwordField.focus()
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: passwordField.focus()
    }

    ColumnLayout {
        id: mainLayout
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.4, 400)
        spacing: Theme.spacing.xlarge

        Clock {
            id: clock
            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            Layout.preferredHeight: Theme.spacing.large
        }

        PasswordField {
            id: passwordField
            Layout.fillWidth: true
            Layout.preferredHeight: implicitHeight

            onSubmitted: function(password) {
                LockController.authenticate(password)
            }

            onEscaped: {
                clear()
            }

            Component.onCompleted: {
                setFocus()
            }
        }

        StatusMessage {
            Layout.alignment: Qt.AlignHCenter
            message: LockController.statusMessage
            isError: LockController.hasError
            visible: LockController.statusMessage !== ""
        }

        Item {
            Layout.preferredHeight: Theme.spacing.large
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Theme.spacing.medium

            ActionButton {
                icon: "⏾"
                label: "Suspend"
                onClicked: PowerManager.suspend()
            }

            ActionButton {
                icon: "↻"
                label: "Reboot"
                onClicked: PowerManager.reboot()
            }

            ActionButton {
                icon: "⏻"
                label: "Shutdown"
                onClicked: PowerManager.shutdown()
            }
        }
    }

    Component.onCompleted: {
        passwordField.setFocus()
    }
}
