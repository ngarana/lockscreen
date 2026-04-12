// ConfigService.qml - Configuration Management Service
//
// Singleton for loading, saving, and managing user preferences.
// Provides persistent storage via JSON configuration files with
// schema validation and change notifications.
//
// Responsibilities:
// - Load/save user configuration from JSON file
// - Provide default configuration values
// - Emit signals on configuration changes
// - Validate configuration against schema
//
// Usage:
// import "../services"
//
// Connections {
//     target: ConfigService
//     function onConfigChanged(key, value) { console.log(key + " changed to " + value) }
// }
//
// Component.onCompleted: {
//     ConfigService.setValue("theme.name", "catppuccin-mocha")
//     ConfigService.save()
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

    // Path to configuration file
    readonly property string configPath: Quickshell.env("XDG_CONFIG_HOME", Quickshell.env("HOME") + "/.config") + "/qypr/config.json"

    // ========================================================================
    // Signals
    // ========================================================================

    // Emitted when any configuration value changes
    signal configChanged(string key, var value)

    // Emitted when configuration is loaded
    signal configLoaded()

    // Emitted when configuration is saved
    signal configSaved()

    // Emitted when configuration fails to load/save
    signal configError(string error)

    // ========================================================================
    // Public Properties
    // ========================================================================

    // Current configuration object
    property var _config: ({})

    // Whether configuration has been loaded
    property bool isLoaded: false

    // ========================================================================
    // Default Configuration Schema
    // ========================================================================

    readonly property var _defaultConfig: ({
        // Theme settings
        theme: {
            name: "catppuccin-mocha",
            accentColor: "blue",
            autoSwitch: false,
            dayTheme: "catppuccin-latte",
            nightTheme: "catppuccin-mocha"
        },

        // Status bar settings
        statusbar: {
            enabled: true,
            position: "top",
            height: 36,
            modules: {
                workspaces: true,
                windowTitle: true,
                clock: true,
                systemTray: true,
                network: true,
                bluetooth: true,
                battery: true,
                volume: true,
                notifications: true
            }
        },

        // Launcher settings
        launcher: {
            enabled: true,
            shortcut: "Super+Space",
            gridColumns: 6,
            showCategories: true,
            showFavorites: true,
            maxRecentApps: 10
        },

        // Notification settings
        notifications: {
            enabled: true,
            doNotDisturb: false,
            popupTimeout: 5000,
            maxVisible: 5,
            showPreviews: true,
            soundEnabled: false
        },

        // Control center settings
        controlcenter: {
            enabled: true,
            width: 360,
            quickToggles: ["wifi", "bluetooth", "dnd", "nightlight"]
        },

        // Lockscreen settings
        lockscreen: {
            enabled: true,
            autoLockDelay: 300,
            showClock: true,
            showMediaControls: true,
            backgroundType: "video",
            backgroundPath: ""
        },

        // Audio settings
        audio: {
            defaultSink: "",
            defaultSource: "",
            volumeStep: 5,
            showOSD: true
        },

        // Network settings
        network: {
            showSavedNetworks: true
        },

        // Bluetooth settings
        bluetooth: {
            autoConnect: []
        },

        // Power settings
        power: {
            idleDelay: 600,
            suspendWhenIdle: false,
            lidCloseAction: "suspend"
        }
    })

    // ========================================================================
    // Public Methods
    // ========================================================================

    // Get a configuration value by key path (e.g., "theme.name")
    function getValue(key, defaultValue) {
        const parts = key.split(".")
        let current = _config

        for (let i = 0; i < parts.length; i++) {
            if (current === undefined || current === null) {
                return defaultValue !== undefined ? defaultValue : undefined
            }
            current = current[parts[i]]
        }

        return current !== undefined ? current : defaultValue
    }

    // Set a configuration value by key path (e.g., "theme.name")
    function setValue(key, value) {
        const parts = key.split(".")
        let current = _config

        // Create nested objects if they don't exist
        for (let i = 0; i < parts.length - 1; i++) {
            if (!(parts[i] in current) || typeof current[parts[i]] !== "object") {
                current[parts[i]] = {}
            }
            current = current[parts[i]]
        }

        // Set the value
        const lastKey = parts[parts.length - 1]
        const oldValue = current[lastKey]
        current[lastKey] = value

        // Emit signal if value changed
        if (JSON.stringify(oldValue) !== JSON.stringify(value)) {
            configChanged(key, value)
        }
    }

    // Load configuration from file
    function load() {
        configProcess.command = ["cat", configPath]
        configProcess.running = true
    }

    // Save configuration to file
    function save() {
        const json = JSON.stringify(_config, null, 2)
        const escaped = json.replace(/'/g, "'\\''")
        saveProcess.command = ["sh", "-c", "mkdir -p $(dirname " + configPath + ") && echo '" + escaped + "' > " + configPath]
        saveProcess.running = true
    }

    // Reset configuration to defaults
    function reset() {
        _config = JSON.parse(JSON.stringify(_defaultConfig))
        configChanged("*", null)
        Core.Logger.info("Configuration reset to defaults", "ConfigService")
    }

    // Check if a module is enabled
    function isModuleEnabled(moduleName) {
        return getValue(moduleName + ".enabled", false)
    }

    // ========================================================================
    // Private Methods
    // ========================================================================

    // Merge loaded config with defaults
    function _mergeWithDefaults(loaded) {
        function merge(target, source) {
            for (const key in source) {
                if (typeof source[key] === "object" && source[key] !== null && !Array.isArray(source[key])) {
                    if (!(key in target) || typeof target[key] !== "object") {
                        target[key] = {}
                    }
                    merge(target[key], source[key])
                } else if (!(key in target)) {
                    target[key] = source[key]
                }
            }
        }

        const result = JSON.parse(JSON.stringify(loaded))
        merge(result, _defaultConfig)
        return result
    }

    // ========================================================================
    // Process Handlers
    // ========================================================================

    property var configProcess: Process {
        id: configProcess
        command: []

        onExited: function(code, status) {
            if (code === 0 && stdout.length > 0) {
                try {
                    const loaded = JSON.parse(stdout)
                    root._config = root._mergeWithDefaults(loaded)
                    root.isLoaded = true
                    root.configLoaded()
                    Core.Logger.info("Configuration loaded from " + root.configPath, "ConfigService")
                } catch (e) {
                    Core.Logger.error("Failed to parse config: " + e, "ConfigService")
                    root._config = JSON.parse(JSON.stringify(root._defaultConfig))
                    root.isLoaded = true
                    root.configError("Parse error: " + e)
                }
            } else {
                // File doesn't exist or is empty, use defaults
                root._config = JSON.parse(JSON.stringify(root._defaultConfig))
                root.isLoaded = true
                root.configLoaded()
                Core.Logger.info("Using default configuration", "ConfigService")
            }
        }
    }

    property var saveProcess: Process {
        id: saveProcess
        command: []

        onExited: function(code, status) {
            if (code === 0) {
                root.configSaved()
                Core.Logger.info("Configuration saved to " + root.configPath, "ConfigService")
            } else {
                root.configError("Failed to save configuration")
                Core.Logger.error("Failed to save configuration", "ConfigService")
            }
        }
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        load()
    }
}
