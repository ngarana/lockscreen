// Errors.qml - Error Codes and Messages
//
// Centralized error definitions for consistent error handling.
//
// Usage:
//   import "../core"
//
//   Component.onCompleted: {
//       Logger.error(Errors.getMessage(Errors.Auth.PAM_FAILED))
//   }

import QtQuick

QtObject {
    id: root

    // ========================================================================
    // Authentication Errors
    // ========================================================================

    readonly property var Auth: QtObject {
        readonly property int SUCCESS: 0
        readonly property int PAM_FAILED: 1001
        readonly property int PAM_ERROR: 1002
        readonly property int EMPTY_PASSWORD: 1003
        readonly property int ALREADY_AUTHENTICATING: 1004
        readonly property int LOCK_NOT_SET: 1005

        function getMessage(code) {
            switch (code) {
                case root.Auth.SUCCESS: return "Authentication successful"
                case root.Auth.PAM_FAILED: return "Authentication failed"
                case root.Auth.PAM_ERROR: return "Authentication error"
                case root.Auth.EMPTY_PASSWORD: return "Password cannot be empty"
                case root.Auth.ALREADY_AUTHENTICATING: return "Authentication already in progress"
                case root.Auth.LOCK_NOT_SET: return "Lock instance not configured"
                default: return "Unknown authentication error"
            }
        }
    }

    // ========================================================================
    // Configuration Errors
    // ========================================================================

    readonly property var Config: QtObject {
        readonly property int SUCCESS: 0
        readonly property int LOAD_FAILED: 2001
        readonly property int SAVE_FAILED: 2002
        readonly property int INVALID_SCHEMA: 2003
        readonly property int FILE_NOT_FOUND: 2004
        readonly property int PERMISSION_DENIED: 2005

        function getMessage(code) {
            switch (code) {
                case root.Config.SUCCESS: return "Configuration operation successful"
                case root.Config.LOAD_FAILED: return "Failed to load configuration"
                case root.Config.SAVE_FAILED: return "Failed to save configuration"
                case root.Config.INVALID_SCHEMA: return "Invalid configuration schema"
                case root.Config.FILE_NOT_FOUND: return "Configuration file not found"
                case root.Config.PERMISSION_DENIED: return "Permission denied for configuration"
                default: return "Unknown configuration error"
            }
        }
    }

    // ========================================================================
    // Service Errors
    // ========================================================================

    readonly property var Service: QtObject {
        readonly property int SUCCESS: 0
        readonly property int INIT_FAILED: 3001
        readonly property int NOT_AVAILABLE: 3002
        readonly property int TIMEOUT: 3003
        readonly property int CONNECTION_LOST: 3004

        function getMessage(code) {
            switch (code) {
                case root.Service.SUCCESS: return "Service operation successful"
                case root.Service.INIT_FAILED: return "Service initialization failed"
                case root.Service.NOT_AVAILABLE: return "Service not available"
                case root.Service.TIMEOUT: return "Service operation timed out"
                case root.Service.CONNECTION_LOST: return "Service connection lost"
                default: return "Unknown service error"
            }
        }
    }

    // ========================================================================
    // Module Errors
    // ========================================================================

    readonly property var Module: QtObject {
        readonly property int SUCCESS: 0
        readonly property int LOAD_FAILED: 4001
        readonly property int NOT_FOUND: 4002
        readonly property int ALREADY_ACTIVE: 4003
        readonly property int DEPENDENCY_MISSING: 4004

        function getMessage(code) {
            switch (code) {
                case root.Module.SUCCESS: return "Module operation successful"
                case root.Module.LOAD_FAILED: return "Failed to load module"
                case root.Module.NOT_FOUND: return "Module not found"
                case root.Module.ALREADY_ACTIVE: return "Module already active"
                case root.Module.DEPENDENCY_MISSING: return "Module dependency missing"
                default: return "Unknown module error"
            }
        }
    }

    // ========================================================================
    // Generic Error Helper
    // ========================================================================

    // Get message for any error code
    function getMessage(code) {
        if (code >= 1000 && code < 2000) return root.Auth.getMessage(code)
        if (code >= 2000 && code < 3000) return root.Config.getMessage(code)
        if (code >= 3000 && code < 4000) return root.Service.getMessage(code)
        if (code >= 4000 && code < 5000) return root.Module.getMessage(code)
        return "Unknown error (code: " + code + ")"
    }

    // Check if error code indicates success
    function isSuccess(code) {
        return code === 0
    }

    // Check if error code indicates failure
    function isFailure(code) {
        return code !== 0
    }
}
