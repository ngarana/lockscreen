// SessionService.qml - User Session Management Service
//
// Singleton for managing user session information, user data, and
// session lifecycle events. Provides user avatar, name, and session state.
//
// Responsibilities:
// - User session information (name, avatar, home directory)
// - Session lifecycle events (start, end)
// - User preferences loading
// - Session state persistence
//
// Usage:
// import "../services"
//
// Connections {
//     target: SessionService
//     function onSessionStarted() { initializeUserInterface() }
// }

pragma Singleton
import Quickshell
import Quickshell.Io
import QtQuick
import "../core" as Core

QtObject {
    id: root

    // ========================================================================
    // State Properties (Read-Only)
    // ========================================================================

    // User name (login name)
    property string userName: ""

    // User display name (full name)
    property string displayName: ""

    // User home directory
    property string homeDirectory: ""

    // User ID
    property int userId: -1

    // Session ID
    property string sessionId: ""

    // Session type (wayland, x11, etc.)
    property string sessionType: ""

    // Desktop session
    property string desktopSession: ""

    // User avatar path
    property string avatarPath: ""

    // Whether session is active
    property bool isSessionActive: false

    // Session start time
    property var sessionStartTime: null

    // Current working directory
    property string currentDirectory: ""

    // Shell being used
    property string shell: ""

    // Terminal
    property string terminal: ""

    // Editor
    property string editor: ""

    // Browser
    property string browser: ""

    // ========================================================================
    // Signals
    // ========================================================================

    // Emitted when session starts
    signal sessionStarted()

    // Emitted when session is ending
    signal sessionEnding()

    // Emitted when user information changes
    signal userInfoChanged()

    // ========================================================================
    // Public Methods
    // ========================================================================

    // Get user name from system
    function getUserName() {
        return Quickshell.env("USER") || Quickshell.env("USERNAME") || "unknown"
    }

    // Get home directory
    function getHomeDirectory() {
        return Quickshell.env("HOME") || "/tmp"
    }

    // Load user information
    function loadUserInfo() {
        userName = getUserName()
        homeDirectory = getHomeDirectory()
        sessionType = Quickshell.env("XDG_SESSION_TYPE") || "unknown"
        desktopSession = Quickshell.env("DESKTOP_SESSION") || "unknown"
        sessionId = Quickshell.env("XDG_SESSION_ID") || ""
        currentDirectory = Quickshell.env("PWD") || homeDirectory
        shell = Quickshell.env("SHELL") || "/bin/sh"
        terminal = Quickshell.env("TERMINAL") || "xterm"
        editor = Quickshell.env("EDITOR") || "nano"
        browser = Quickshell.env("BROWSER") || "firefox"

        // Try to get display name from passwd
        _fetchDisplayName()

        // Look for avatar
        _findAvatar()

        userInfoChanged()
    }

    // Get config directory path
    function getConfigDir() {
        return Quickshell.env("XDG_CONFIG_HOME") || homeDirectory + "/.config"
    }

    // Get data directory path
    function getDataDir() {
        return Quickshell.env("XDG_DATA_HOME") || homeDirectory + "/.local/share"
    }

    // Get cache directory path
    function getCacheDir() {
        return Quickshell.env("XDG_CACHE_HOME") || homeDirectory + "/.cache"
    }

    // Get runtime directory
    function getRuntimeDir() {
        return Quickshell.env("XDG_RUNTIME_DIR") || "/tmp"
    }

    // Initialize session
    function initializeSession() {
        loadUserInfo()
        isSessionActive = true
        sessionStartTime = new Date()

        Core.Logger.info("Session initialized for user: " + userName, "SessionService")
        sessionStarted()
    }

    // End session
    function endSession() {
        isSessionActive = false
        sessionEnding()
        Core.Logger.info("Session ending for user: " + userName, "SessionService")
    }

    // Get session duration in seconds
    function getSessionDuration() {
        if (!sessionStartTime || !isSessionActive) {
            return 0
        }
        return Math.floor((new Date() - sessionStartTime) / 1000)
    }

    // Get session duration formatted string
    function getSessionDurationFormatted() {
        const duration = getSessionDuration()
        const hours = Math.floor(duration / 3600)
        const minutes = Math.floor((duration % 3600) / 60)
        const seconds = duration % 60

        if (hours > 0) {
            return hours + "h " + minutes + "m"
        } else if (minutes > 0) {
            return minutes + "m " + seconds + "s"
        }
        return seconds + "s"
    }

    // Check if running on specific desktop
    function isDesktop(name) {
        return desktopSession.toLowerCase().includes(name.toLowerCase())
    }

    // Get path relative to home
    function relativeToHome(path) {
        if (path.startsWith(homeDirectory)) {
            return "~" + path.substring(homeDirectory.length)
        }
        return path
    }

    // Expand path (handle ~)
    function expandPath(path) {
        if (path.startsWith("~/")) {
            return homeDirectory + path.substring(1)
        }
        return path
    }

    // ========================================================================
    // Private Methods
    // ========================================================================

    function _fetchDisplayName() {
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["getent", "passwd", "' + userName + '"]; running: true; onExited: function(c) { if (c === 0) root._parsePasswd(stdout) } }',
            root
        )
    }

    function _parsePasswd(output) {
        // Format: username:password:UID:GID:display_name:home:shell
        const parts = output.split(":")
        if (parts.length >= 5) {
            const gecos = parts[4]
            // GECOS field may contain comma-separated values
            const names = gecos.split(",")
            if (names.length > 0 && names[0].trim()) {
                displayName = names[0].trim()
            } else {
                displayName = userName
            }
        } else {
            displayName = userName
        }
    }

    function _findAvatar() {
        const possiblePaths = [
            getDataDir() + "/face",
            homeDirectory + "/.face",
            homeDirectory + "/.face.icon",
            "/var/lib/AccountsService/icons/" + userName,
            getConfigDir() + "/.face"
        ]

        for (let i = 0; i < possiblePaths.length; i++) {
            const path = possiblePaths[i]
            const checkProcess = Qt.createQmlObject(
                'import Quickshell.Io; Process { command: ["test", "-f", "' + path + '"]; running: true; onExited: function(c) { if (c === 0 && root.avatarPath === "") root.avatarPath = "' + path + '" } }',
                root
            )
        }
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        initializeSession()
    }
}
