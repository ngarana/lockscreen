// NetworkService.qml - Network and Connectivity Service
//
// Singleton for monitoring WiFi status, network connections, and
// connection state changes. Provides signal-based updates for UI
// components to react to network changes.
//
// Responsibilities:
// - WiFi status monitoring (connected/disconnected)
// - Network name (SSID) retrieval
// - Signal strength indication
// - Connection state change notifications
// - Available networks scanning
//
// Usage:
// import "../services"
//
// Connections {
//     target: NetworkService
//     function onNetworkStatusChanged() { updateNetworkIcon() }
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

    // Update interval in milliseconds
    property int updateInterval: 5000

    // ========================================================================
    // State Properties (Read-Only)
    // ========================================================================

    // Whether WiFi is enabled
    property bool wifiEnabled: false

    // Whether currently connected to a network
    property bool isConnected: false

    // Current network SSID (empty if not connected)
    property string ssid: ""

    // Current network IP address
    property string ipAddress: ""

    // Signal strength (0-100)
    property int signalStrength: 0

    // Signal strength category: "excellent", "good", "fair", "poor", "none"
    property string signalLevel: "none"

    // Connection type: "wifi", "ethernet", "none"
    property string connectionType: "none"

    // Whether an internet connection is available
    property bool hasInternet: false

    // List of available WiFi networks
    property var availableNetworks: []

    // ========================================================================
    // Signals
    // ========================================================================

    // Emitted when network status changes (connection/disconnection)
    signal networkStatusChanged()

    // Emitted when signal strength changes (parameter carries new value)
    signal strengthUpdated(int strength)

    // Emitted when available networks change
    signal networksUpdated()

    // ========================================================================
    // Private Properties
    // ========================================================================

    property var _timer: Timer {
        interval: root.updateInterval
        running: true
        repeat: true
        onTriggered: root._updateStatus()
    }

    property var _statusProcess: Process {
        id: statusProcess
        command: []

        onExited: function(code, status) {
            if (code === 0) {
                root._parseStatus(stdout)
            }
        }
    }

    property var _scanProcess: Process {
        id: scanProcess
        command: []

        onExited: function(code, status) {
            if (code === 0) {
                root._parseNetworks(stdout)
            }
        }
    }

    // ========================================================================
    // Public Methods
    // ========================================================================

    // Connect to a WiFi network
    function connectToNetwork(ssid, password) {
        Core.Logger.info("Connecting to WiFi: " + ssid, "NetworkService")
        // Implementation depends on available tools (nmcli, iwd, etc.)
        // This is a placeholder for actual implementation
    }

    // Disconnect from current network
    function disconnect() {
        Core.Logger.info("Disconnecting from network", "NetworkService")
        // Implementation depends on available tools
    }

    // Scan for available networks
    function scanNetworks() {
        // Use nmcli if available
        scanProcess.command = ["nmcli", "-t", "-f", "SSID,SIGNAL,SECURITY", "device", "wifi", "list", "--rescan", "yes"]
        scanProcess.running = true
    }

    // Get icon name based on current state
    function getIconName() {
        if (!isConnected) {
            return "network-offline"
        }

        if (connectionType === "ethernet") {
            return "network-wired"
        }

        switch (signalLevel) {
            case "excellent": return "network-wireless-signal-excellent"
            case "good": return "network-wireless-signal-good"
            case "fair": return "network-wireless-signal-ok"
            case "poor": return "network-wireless-signal-weak"
            default: return "network-wireless-signal-none"
        }
    }

    // Force refresh of network status
    function refresh() {
        _updateStatus()
    }

    // ========================================================================
    // Private Methods
    // ========================================================================

    function _updateStatus() {
        // Check using nmcli
        statusProcess.command = ["nmcli", "-t", "-f", "DEVICE,TYPE,STATE,CONNECTION", "device", "status"]
        statusProcess.running = true
    }

    function _parseStatus(output) {
        if (!output) {
            Core.Logger.warning("NetworkService: null output from nmcli", "NetworkService")
            return
        }
        const lines = output.split("\n")
        let wasConnected = root.isConnected
        let wasSsid = root.ssid

        root.isConnected = false
        root.connectionType = "none"
        root.ssid = ""
        root.wifiEnabled = false

        for (let i = 0; i < lines.length; i++) {
            const line = lines[i].trim()
            if (line.length === 0) continue

            const parts = line.split(":");
            if (parts.length < 4) continue

            const device = parts[0]
            const type = parts[1]
            const state = parts[2]
            const connection = parts[3]

            if (type === "wifi") {
                root.wifiEnabled = true
                if (state === "connected" && connection) {
                    root.isConnected = true
                    root.connectionType = "wifi"
                    root.ssid = connection
                }
            } else if (type === "ethernet") {
                if (state === "connected" && connection) {
                    root.isConnected = true
                    root.connectionType = "ethernet"
                    root.ssid = connection
                }
            }
        }

        // Update signal strength if connected via WiFi
        if (root.isConnected && root.connectionType === "wifi") {
            _updateSignalStrength()
        } else {
            root.signalStrength = 0
            root.signalLevel = "none"
        }

        // Check internet connectivity
        _checkInternet()

        // Emit signals if changed
        if (wasConnected !== root.isConnected || wasSsid !== root.ssid) {
            networkStatusChanged()
        }
    }

    function _updateSignalStrength() {
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["nmcli", "-t", "-f", "ACTIVE,SIGNAL", "device", "wifi"]; running: true; onExited: function(c) { if (c === 0) root._parseSignalStrength(stdout) } }',
            root
        )
    }

    function _parseSignalStrength(output) {
        const lines = output.split("\n")
        for (let i = 0; i < lines.length; i++) {
            const line = lines[i].trim()
            if (line.length === 0) continue

            const parts = line.split(":")
            if (parts.length >= 2 && parts[0] === "yes") {
                const strength = parseInt(parts[1]) || 0
                if (strength !== root.signalStrength) {
                    root.signalStrength = strength
                    strengthUpdated(strength)

                    // Determine signal level
                    if (strength >= 80) {
                        root.signalLevel = "excellent"
                    } else if (strength >= 60) {
                        root.signalLevel = "good"
                    } else if (strength >= 40) {
                        root.signalLevel = "fair"
                    } else if (strength > 0) {
                        root.signalLevel = "poor"
                    } else {
                        root.signalLevel = "none"
                    }
                }
                break
            }
        }
    }

    function _parseNetworks(output) {
        const networks = []
        const lines = output.split("\n")

        for (let i = 0; i < lines.length; i++) {
            const line = lines[i].trim()
            if (line.length === 0) continue

            const parts = line.split(":")
            if (parts.length < 2) continue

            const ssid = parts[0]
            const signal = parseInt(parts[1]) || 0
            const security = parts[2] || ""

            if (ssid) {
                networks.push({
                    ssid: ssid,
                    signal: signal,
                    secured: security !== "" && security !== "--"
                })
            }
        }

        // Sort by signal strength
        networks.sort(function(a, b) { return b.signal - a.signal })

        root.availableNetworks = networks
        networksUpdated()
    }

    function _checkInternet() {
        // Quick check by attempting to reach a well-known host
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["ping", "-c", "1", "-W", "1", "1.1.1.1"]; running: true; onExited: function(c) { root.hasInternet = (c === 0) } }',
            root
        )
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        _updateStatus()
        Core.Logger.info("NetworkService initialized", "NetworkService")
    }
}
