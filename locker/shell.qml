// shell.qml - Standalone Locker Entry Point
//
// Minimal entry point for running the locker independently:
//   qs -p /path/to/qypr/locker/
//
// Imports the lockscreen module directly without the full shell.

import Quickshell
import Quickshell.Wayland
import QtQuick

import "../src/modules/lockscreen"

ShellRoot {
    id: root

    readonly property bool autoLock: Quickshell.env("QUICKSHELL_LOCKSCREEN_AUTO_LOCK") === "1"

    WlSessionLock {
        id: sessionLock
        locked: root.autoLock

        Component.onCompleted: LockController.setLockInstance(sessionLock)

        WlSessionLockSurface {
            color: "transparent"

            LockScreen {
                anchors.fill: parent
                onUnlockRequested: sessionLock.locked = false
            }
        }
    }
}
