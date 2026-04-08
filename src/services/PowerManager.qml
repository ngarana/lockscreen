// PowerManager.qml - System Power Management Service
//
// This singleton handles system power actions via systemctl.
// Provides a clean API for suspend, reboot, shutdown, and hibernate.
//
// Responsibilities:
// - Execute system power commands via systemctl
// - Track command execution status
// - Emit signals for success/failure
//
// Requirements:
// - systemd (for systemctl commands)
// - Appropriate polkit permissions for power actions
//
// Usage:
//   PowerManager.suspend()   // Suspend to RAM
//   PowerManager.reboot()    // Restart system
//   PowerManager.shutdown()  // Power off system
//   PowerManager.hibernate() // Suspend to disk
//
// Signals:
//   actionTriggered(action) - Emitted when a power action starts
//   actionFailed(action, error) - Emitted when a power action fails

pragma Singleton
import Quickshell
import Quickshell.Io
import QtQuick

QtObject {
    id: root

    // ========================================================================
    // Signals
    // ========================================================================

    // Emitted when a power action is initiated
    // action: Name of the action (e.g., "suspend", "reboot")
    signal actionTriggered(string action)

    // Emitted when a power action fails
    // action: Name of the action that failed
    // error: Description of the error
    signal actionFailed(string action, string error)

    // ========================================================================
    // Public Methods - Power Actions
    // ========================================================================

    // Suspend the system to RAM (sleep mode)
    // System state is preserved in RAM, low power consumption
    // Fast resume, but data lost if power is cut
    function suspend() {
        root._executeCommand(["systemctl", "suspend"])
    }

    // Reboot the system
    // Restarts the operating system
    function reboot() {
        root._executeCommand(["systemctl", "reboot"])
    }

    // Shutdown the system
    // Powers off the machine completely
    function shutdown() {
        root._executeCommand(["systemctl", "poweroff"])
    }

    // Hibernate the system to disk
    // System state is written to swap space
    // Zero power consumption when hibernated, slower resume than suspend
    // Requires sufficient swap space
    function hibernate() {
        root._executeCommand(["systemctl", "hibernate"])
    }

    // ========================================================================
    // Internal Methods
    // ========================================================================

    // Execute a systemctl command
    // cmd: Array of command arguments (e.g., ["systemctl", "suspend"])
    function _executeCommand(cmd) {
        process.command = cmd
        process.running = true
    }

    // ========================================================================
    // Process Handler (Internal)
    // ========================================================================

    property var process: Process {
        id: process

        // Called when the process exits
        // code: Exit code (0 = success, non-zero = failure)
        // status: Exit status description
        onExited: function(code, status) {
            if (code !== 0) {
                // Command failed, emit error signal
                root.actionFailed(process.command[0], "Exit code: " + code)
            }
        }
    }
}
