// ShellIpc.qml - IPC handlers singleton
//
// Handles IPC calls from external tools for screenshot, audio, launcher, etc.

pragma Singleton

import Quickshell
import Quickshell.Io
import Quickshell.Hyprland

Singleton {
    signal screenshot()
    signal volUp()
    signal volDown()
    signal toggleMic()
    signal launcherOpen()
    signal launcherClose()
    signal launcherToggle()
    signal lock()
    signal osd()
    signal clipboard()

    IpcHandler {
        target: "screenshot"
        function takeScreenshot() { screenshot() }
    }

    IpcHandler {
        target: "audio"
        function toggleMic() { toggleMic() }
        function volUp() { volUp() }
        function volDown() { volDown() }
    }

    IpcHandler {
        target: "launcher"
        function open() { launcherOpen() }
        function close() { launcherClose() }
        function toggle() { launcherToggle() }
    }

    IpcHandler {
        target: "lockscreen"
        function lock() { lock() }
    }

    IpcHandler {
        target: "qypr"
        function lock(): void { lock() }
        function screenshot(): void { screenshot() }
    }
}
