// Constants.qml - System Constants and Configuration
//
// Centralized constants for app-wide configuration values.
// Prevents magic numbers and strings throughout the codebase.

import QtQuick

QtObject {
    id: root

    // ========================================================================
    // Application Metadata
    // ========================================================================

    readonly property string appName: "Qypr"
    readonly property string appVersion: "0.1.0"
    readonly property string appDescription: "Modular desktop shell system for Hyprland/Wayland"

    // ========================================================================
    // Configuration Keys
    // ========================================================================

    readonly property var configKeys: QtObject {
        readonly property string theme: "theme.current"
        readonly property string autoLock: "lockscreen.autoLock"
        readonly property string videoPlaylist: "lockscreen.videoPlaylist"
        readonly property string barVisibility: "statusbar.visible"
        readonly property string barHeight: "statusbar.height"
        readonly property string launcherShortcut: "launcher.shortcut"
        readonly property string dndMode: "notifications.dnd"
        readonly property string controlCenterVisible: "controlcenter.visible"
    }

    // ========================================================================
    // Environment Variables
    // ========================================================================

    readonly property var envVars: QtObject {
        readonly property string autoLock: "QUICKSHELL_LOCKSCREEN_AUTO_LOCK"
        readonly property string mode: "QUICKSHELL_MODE"
        readonly property string debug: "QUICKSHELL_DEBUG"
        readonly property string configPath: "QUICKSHELL_CONFIG"
    }

    // ========================================================================
    // IPC Targets
    // ========================================================================

    readonly property var ipcTargets: QtObject {
        readonly property string lockscreen: "lockscreen"
        readonly property string statusbar: "statusbar"
        readonly property string launcher: "launcher"
        readonly property string notifications: "notifications"
        readonly property string controlcenter: "controlcenter"
    }

    // ========================================================================
    // Default Values
    // ========================================================================

    readonly property var defaults: QtObject {
        readonly property bool autoLock: false
        readonly property string mode: "full"
        readonly property string theme: "catppuccin-mocha"
        readonly property int barHeight: 32
        readonly property string launcherShortcut: "Super+Space"
        readonly property bool dndMode: false
    }

    // ========================================================================
    // Module Names
    // ========================================================================

    readonly property var modules: QtObject {
        readonly property string lockscreen: "lockscreen"
        readonly property string statusbar: "statusbar"
        readonly property string launcher: "launcher"
        readonly property string notifications: "notifications"
        readonly property string controlcenter: "controlcenter"
    }

    // ========================================================================
    // File Paths
    // ========================================================================

    readonly property var paths: QtObject {
        readonly property string themeDir: "themes/"
        readonly property string playlistDir: "playlists/"
        readonly property string configDir: "~/.config/qypr/"
        readonly property string logFile: "~/.local/share/qypr/qypr.log"
    }
}
