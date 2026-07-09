// shell.qml - Locker Entry Point (stripped to lockscreen-only)

import Quickshell
import Quickshell.Wayland
import QtQuick

import "src/modules/lockscreen"

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
