pragma Singleton
import Quickshell
import Quickshell.Io
import QtQuick

QtObject {
    id: root

    signal actionTriggered(string action)
    signal actionFailed(string action, string error)

    function suspend() {
        root._executeCommand(["systemctl", "suspend"])
    }

    function reboot() {
        root._executeCommand(["systemctl", "reboot"])
    }

    function shutdown() {
        root._executeCommand(["systemctl", "poweroff"])
    }

    function hibernate() {
        root._executeCommand(["systemctl", "hibernate"])
    }

    function _executeCommand(cmd) {
        process.command = cmd
        process.running = true
    }

    property var process: Process {
        id: process

        onExited: function(code, status) {
            if (code !== 0) {
                root.actionFailed(process.command[0], "Exit code: " + code)
            }
        }
    }
}
