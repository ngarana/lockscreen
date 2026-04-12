// HyprlandService.qml - Hyprland Window Manager Integration
//
// Singleton for integrating with Hyprland compositor via Hyprland IPC.
// Provides workspace management, window tracking, and monitor information.
//
// Responsibilities:
// - Workspace management (create, remove, focus, rename)
// - Window tracking (open, close, focus, title changes)
// - Monitor information (layout, resolution, scale)
// - Hyprland IPC integration
//
// Usage:
// import "../services"
//
// Connections {
//     target: HyprlandService
//     function onWorkspaceChanged(id) { updateWorkspaceIndicator() }
// }

pragma Singleton
import Quickshell
import Quickshell.Io
import QtQuick
import "../core" as Core

QtObject {
    id: root

    // ========================================================================
    // Configuration
    // ========================================================================

    // Hyprland IPC socket path
    property string socketPath: _getSocketPath()

    // Update interval for polling (ms) - IPC events are preferred
    property int updateInterval: 1000

    // ========================================================================
    // State Properties (Read-Only)
    // ========================================================================

    // Whether connected to Hyprland
    property bool isConnected: false

    // Hyprland version
    property string version: ""

    // List of workspaces
    property var workspaces: []

    // Current active workspace ID
    property int activeWorkspaceId: 0

    // Current active workspace name
    property string activeWorkspaceName: ""

    // List of windows
    property var windows: []

    // Currently focused window
    property var activeWindow: null

    // List of monitors
    property var monitors: []

    // Number of monitors
    property int monitorCount: 0

    // List of special workspaces (scratchpads)
    property var specialWorkspaces: []

    // Layout mode
    property string layoutMode: "dwindle"

    // ========================================================================
    // Signals
    // ========================================================================

    // Emitted when workspace changes
    signal workspaceChanged(int id)

    // Emitted when a window is opened
    signal windowOpened(var window)

    // Emitted when a window is closed
    signal windowClosed(var window)

    // Emitted when window focus changes
    signal windowFocusChanged(var window)

    // Emitted when monitor configuration changes
    signal monitorChanged()

    // Emitted when window title changes
    signal windowTitleChanged(var window)

    // ========================================================================
    // Private Properties
    // ========================================================================

    property var _updateTimer: Timer {
        interval: root.updateInterval
        running: true
        repeat: true
        onTriggered: root._refreshState()
    }

    // ========================================================================
    // Public Methods
    // ========================================================================

    // Send a command to Hyprland
    function dispatch(command) {
        if (!isConnected) return

        Core.Logger.debug("Hyprland dispatch: " + command, "HyprlandService")

        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["hyprctl", "dispatch", "' + command + '"]; running: true }',
            root
        )
    }

    // Send a Hyprland keyword
    function keyword(key, value) {
        if (!isConnected) return

        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["hyprctl", "keyword", "' + key + '", "' + value + '"]; running: true }',
            root
        )
    }

    // Switch to workspace
    function switchToWorkspace(workspaceId) {
        dispatch("workspace " + workspaceId)
    }

    // Move window to workspace
    function moveToWorkspace(workspaceId) {
        dispatch("movetoworkspace " + workspaceId)
    }

    // Move workspace to monitor
    function moveWorkspaceToMonitor(workspaceId, monitor) {
        dispatch("moveworkspacetomonitor " + workspaceId + " " + monitor)
    }

    // Toggle fullscreen
    function toggleFullscreen(mode) {
        dispatch("fullscreen " + (mode || "0"))
    }

    // Kill active window
    function killActiveWindow() {
        dispatch("killactive")
    }

    // Toggle floating
    function toggleFloating() {
        dispatch("togglefloating")
    }

    // Focus window
    function focusWindow(address) {
        dispatch("focuswindow address:" + address)
    }

    // Focus monitor
    function focusMonitor(monitor) {
        dispatch("focusmonitor " + monitor)
    }

    // Toggle special workspace
    function toggleSpecialWorkspace(name) {
        dispatch("togglespecialworkspace " + (name || ""))
    }

    // Change split ratio
    function changeSplitRatio(delta) {
        dispatch("splitratio " + delta)
    }

    // Toggle pseudo tiling
    function togglePseudo() {
        dispatch("pseudo")
    }

    // Get window class from address
    function getWindowClass(address) {
        for (let i = 0; i < windows.length; i++) {
            if (windows[i].address === address) {
                return windows[i].class
            }
        }
        return ""
    }

    // Get window title from address
    function getWindowTitle(address) {
        for (let i = 0; i < windows.length; i++) {
            if (windows[i].address === address) {
                return windows[i].title
            }
        }
        return ""
    }

    // Get windows on a specific workspace
    function getWindowsOnWorkspace(workspaceId) {
        return windows.filter(function(w) { return w.workspace.id === workspaceId })
    }

    // Get workspace by ID
    function getWorkspace(id) {
        for (let i = 0; i < workspaces.length; i++) {
            if (workspaces[i].id === id) {
                return workspaces[i]
            }
        }
        return null
    }

    // Refresh state
    function refresh() {
        _refreshState()
    }

    // ========================================================================
    // Private Methods
    // ========================================================================

    function _getSocketPath() {
        const hyprlandDir = Quickshell.env("XDG_RUNTIME_DIR", "/tmp") + "/hypr"
        const sig = Quickshell.env("HYPRLAND_INSTANCE_SIGNATURE")

        if (!sig) {
            return ""
        }

        return hyprlandDir + "/" + sig + "/.socket.sock"
    }

    function _checkConnection() {
        const socket = root.socketPath
        if (!socket) {
            isConnected = false
            return
        }

        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["hyprctl", "version"]; running: true; onExited: function(c) { root._onVersionCheck(c, stdout) } }',
            root
        )
    }

    function _onVersionCheck(exitCode, output) {
        if (exitCode === 0) {
            isConnected = true
            const lines = output.split("\n")
            if (lines.length > 0) {
                version = lines[0].replace("Hyprland,", "").trim()
            }
            Core.Logger.info("Connected to Hyprland " + version, "HyprlandService")
            _refreshState()
        } else {
            isConnected = false
            Core.Logger.warning("Failed to connect to Hyprland", "HyprlandService")
        }
    }

    function _refreshState() {
        if (!isConnected) {
            _checkConnection()
            return
        }

        _fetchWorkspaces()
        _fetchWindows()
        _fetchMonitors()
        _fetchActiveWorkspace()
    }

    function _fetchWorkspaces() {
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["hyprctl", "-j", "workspaces"]; running: true; onExited: function(c) { if (c === 0) root._parseWorkspaces(stdout) } }',
            root
        )
    }

    function _parseWorkspaces(output) {
        try {
            const data = JSON.parse(output)
            if (JSON.stringify(data) !== JSON.stringify(workspaces)) {
                workspaces = data
            }
        } catch (e) {
            Core.Logger.error("Failed to parse workspaces: " + e, "HyprlandService")
        }
    }

    function _fetchWindows() {
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["hyprctl", "-j", "clients"]; running: true; onExited: function(c) { if (c === 0) root._parseWindows(stdout) } }',
            root
        )
    }

    function _parseWindows(output) {
        try {
            const data = JSON.parse(output)

            // Check for new and closed windows
            const oldWindows = windows
            const newWindows = data

            // Find new windows
            for (let i = 0; i < newWindows.length; i++) {
                let found = false
                for (let j = 0; j < oldWindows.length; j++) {
                    if (newWindows[i].address === oldWindows[j].address) {
                        found = true

                        // Check if title changed
                        if (newWindows[i].title !== oldWindows[j].title) {
                            windowTitleChanged(newWindows[i])
                        }
                        break
                    }
                }
                if (!found) {
                    windowOpened(newWindows[i])
                }
            }

            // Find closed windows
            for (let i = 0; i < oldWindows.length; i++) {
                let found = false
                for (let j = 0; j < newWindows.length; j++) {
                    if (oldWindows[i].address === newWindows[j].address) {
                        found = true
                        break
                    }
                }
                if (!found) {
                    windowClosed(oldWindows[i])
                }
            }

            windows = data
        } catch (e) {
            Core.Logger.error("Failed to parse windows: " + e, "HyprlandService")
        }
    }

    function _fetchMonitors() {
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["hyprctl", "-j", "monitors"]; running: true; onExited: function(c) { if (c === 0) root._parseMonitors(stdout) } }',
            root
        )
    }

    function _parseMonitors(output) {
        try {
            const data = JSON.parse(output)
            const oldCount = monitorCount
            monitors = data
            monitorCount = data.length

            if (oldCount !== monitorCount) {
                monitorChanged()
            }
        } catch (e) {
            Core.Logger.error("Failed to parse monitors: " + e, "HyprlandService")
        }
    }

    function _fetchActiveWorkspace() {
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["hyprctl", "-j", "activeworkspace"]; running: true; onExited: function(c) { if (c === 0) root._parseActiveWorkspace(stdout) } }',
            root
        )
    }

    function _parseActiveWorkspace(output) {
        try {
            const data = JSON.parse(output)
            const oldId = activeWorkspaceId
            activeWorkspaceId = data.id
            activeWorkspaceName = data.name

            if (oldId !== activeWorkspaceId) {
                workspaceChanged(activeWorkspaceId)
            }
        } catch (e) {
            Core.Logger.error("Failed to parse active workspace: " + e, "HyprlandService")
        }
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        _checkConnection()
        Core.Logger.info("HyprlandService initialized", "HyprlandService")
    }
}
