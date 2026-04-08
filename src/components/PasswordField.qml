import QtQuick
import QtQuick.Controls
import "../services"

Item {
    id: root

    implicitWidth: 400
    implicitHeight: 50

    property string placeholderText: "Enter password"
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

    Rectangle {
        id: container
        anchors.fill: parent
        color: Theme.colors.surface
        radius: Theme.radius.medium
        border.width: root.hasFocus ? 2 : 1
        border.color: root.hasFocus ? Theme.colors.primary : Theme.colors.surfaceHover

        Behavior on border.color {
            ColorAnimation {
                duration: Theme.animation.fast
            }
        }

        TextField {
            id: passwordField
            anchors.fill: parent
            anchors.margins: Theme.spacing.medium
            echoMode: TextInput.Password
            placeholderText: root.placeholderText
            font.pixelSize: Theme.fonts.textSize
            color: Theme.colors.text
            placeholderTextColor: Theme.colors.textMuted
            horizontalAlignment: Text.AlignHCenter
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
}
