// Utils.qml - General Utility Functions
//
// Common helper functions used throughout the application.
//
// Usage:
//   import "../core"
//
//   Component.onCompleted: {
//       var formatted = Utils.formatTime(new Date())
//       var clamped = Utils.clamp(150, 0, 100)
//   }

pragma Singleton
import QtQuick

QtObject {
    id: root

    // ========================================================================
    // String Utilities
    // ========================================================================

    // Capitalize first letter of string
    function capitalize(str) {
        if (!str || str.length === 0) return ""
        return str.charAt(0).toUpperCase() + str.slice(1)
    }

    // Truncate string to max length with ellipsis
    function truncate(str, maxLength) {
        if (!str || str.length <= maxLength) return str
        return str.substring(0, maxLength - 3) + "..."
    }

    // Pad string on left to reach target length
    function padLeft(str, length, char) {
        str = String(str)
        while (str.length < length) {
            str = char + str
        }
        return str
    }

    // ========================================================================
    // Number Utilities
    // ========================================================================

    // Clamp number between min and max
    function clamp(value, min, max) {
        return Math.min(Math.max(value, min), max)
    }

    // Linear interpolation between two values
    function lerp(start, end, t) {
        return start + (end - start) * t
    }

    // Map value from one range to another
    function mapRange(value, inMin, inMax, outMin, outMax) {
        return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin
    }

    // Format percentage (0-100)
    function formatPercent(value, decimals) {
        if (decimals === undefined) decimals = 0
        return (value * 100).toFixed(decimals) + "%"
    }

    // ========================================================================
    // Time Utilities
    // ========================================================================

    // Format date to HH:MM
    function formatTime(date) {
        const hours = root.padLeft(date.getHours(), 2, "0")
        const minutes = root.padLeft(date.getMinutes(), 2, "0")
        return hours + ":" + minutes
    }

    // Format date to Day, Month Date
    function formatDate(date) {
        const days = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
        const months = ["January", "February", "March", "April", "May", "June",
                       "July", "August", "September", "October", "November", "December"]
        return days[date.getDay()] + ", " + months[date.getMonth()] + " " + date.getDate()
    }

    // Format date to short format (MM/DD)
    function formatDateShort(date) {
        const month = root.padLeft(date.getMonth() + 1, 2, "0")
        const day = root.padLeft(date.getDate(), 2, "0")
        return month + "/" + day
    }

    // Format duration from seconds to MM:SS
    function formatDuration(seconds) {
        const mins = Math.floor(seconds / 60)
        const secs = Math.floor(seconds % 60)
        return root.padLeft(mins, 2, "0") + ":" + root.padLeft(secs, 2, "0")
    }

    // Format duration from seconds to human-readable
    function formatDurationHuman(seconds) {
        if (seconds < 60) return seconds + "s"
        if (seconds < 3600) {
            const mins = Math.floor(seconds / 60)
            return mins + "m " + (seconds % 60) + "s"
        }
        const hours = Math.floor(seconds / 3600)
        const mins = Math.floor((seconds % 3600) / 60)
        return hours + "h " + mins + "m"
    }

    // ========================================================================
    // Array Utilities
    // ========================================================================

    // Check if array contains value (case-insensitive for strings)
    function contains(array, value) {
        for (let i = 0; i < array.length; i++) {
            if (typeof value === "string" && typeof array[i] === "string") {
                if (array[i].toLowerCase() === value.toLowerCase()) return true
            } else if (array[i] === value) {
                return true
            }
        }
        return false
    }

    // Remove duplicates from array
    function unique(array) {
        const result = []
        for (let i = 0; i < array.length; i++) {
            if (!root.contains(result, array[i])) {
                result.push(array[i])
            }
        }
        return result
    }

    // ========================================================================
    // UI Utilities
    // ========================================================================

    // Debounce function call (returns debounced function wrapper)
    // Note: QML doesn't support closures well, use Timer instead
    function debounceDelay(timer, delay) {
        timer.interval = delay
        timer.restart()
    }

    // Generate random ID string
    function generateId() {
        return Math.random().toString(36).substring(2, 10)
    }

    // Format file size in human-readable form
    function formatFileSize(bytes) {
        if (bytes < 1024) return bytes + " B"
        if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + " KB"
        if (bytes < 1024 * 1024 * 1024) return (bytes / (1024 * 1024)).toFixed(1) + " MB"
        return (bytes / (1024 * 1024 * 1024)).toFixed(2) + " GB"
    }
}
