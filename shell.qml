import Quickshell
import Quickshell.Wayland
import Quickshell.Io
import QtQuick
import "src/services"
import "src/widgets"

ShellRoot {
    id: root

    readonly property bool autoLock: Quickshell.env("QUICKSHELL_LOCKSCREEN_AUTO_LOCK") === "1"

    WlSessionLock {
        id: sessionLock
        locked: root.autoLock

        WlSessionLockSurface {
            id: lockSurface

            LockScreen {
                anchors.fill: parent
            }
        }
    }

    IpcHandler {
        target: "lockscreen"

        function lock(): void {
            LockController.lock()
        }

        function status(): string {
            if (!LockController.isLocked) {
                return "unlocked"
            }

            return LockController.isSecure ? "locked" : "locking"
        }
    }

    Component.onCompleted: {
        LockController.setLockInstance(sessionLock)
    }
}
