// Logger.qml - Structured Logging Utility
//
// Provides consistent logging with levels, timestamps, and formatting.
//
// Usage:
//   import "../core"
//
//   Component.onCompleted: {
//       Logger.debug("Component initialized")
//       Logger.info("User action detected")
//       Logger.warning("Deprecated API usage")
//       Logger.error("Failed to load resource")
//   }

pragma Singleton
import QtQuick

QtObject {
    id: root

    // ========================================================================
    // Configuration
    // ========================================================================

    // Enable/disable all logging output
    property bool enabled: true

    // Minimum log level to display
    // 0=debug, 1=info, 2=warning, 3=error
    property int minLevel: 0

    // Include timestamps in log output
    property bool showTimestamp: true

    // Include component context in log output
    property bool showContext: true

    // ========================================================================
    // Log Methods
    // ========================================================================

    // Debug level - verbose diagnostic information
    function debug(message, context) {
        if (!root.enabled || root.minLevel > 0) return
        root._print("DEBUG", message, context)
    }

    // Info level - informational messages
    function info(message, context) {
        if (!root.enabled || root.minLevel > 1) return
        root._print("INFO", message, context)
    }

    // Warning level - potential issues
    function warning(message, context) {
        if (!root.enabled || root.minLevel > 2) return
        root._print("WARNING", message, context)
    }

    // Error level - errors and failures
    function error(message, context) {
        if (!root.enabled || root.minLevel > 3) return
        root._print("ERROR", message, context)
    }

    // ========================================================================
    // Internal Implementation
    // ========================================================================

    // Format and print a log message
    function _print(level, message, context) {
        let parts = []

        // Timestamp
        if (root.showTimestamp) {
            parts.push(root._timestamp())
        }

        // Level
        parts.push("[" + level + "]")

        // Context
        if (root.showContext && context) {
            parts.push("[" + context + "]")
        }

        // Message
        parts.push(message)

        console.log(parts.join(" "))
    }

    // Generate ISO-like timestamp string
    function _timestamp() {
        const now = new Date()
        return now.toTimeString().split(" ")[0] + "." + String(now.getMilliseconds()).padStart(3, "0")
    }

    // ========================================================================
    // Utility Methods
    // ========================================================================

    // Enable only errors (for production)
    function productionMode() {
        root.enabled = true
        root.minLevel = 3
        root.showTimestamp = false
        root.showContext = true
    }

    // Enable all logging (for development)
    function developmentMode() {
        root.enabled = true
        root.minLevel = 0
        root.showTimestamp = true
        root.showContext = true
    }

    // Disable all logging
    function silentMode() {
        root.enabled = false
    }
}
