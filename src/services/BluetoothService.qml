// BluetoothService.qml - Bluetooth Management Service
//
// Singleton for managing Bluetooth adapters, paired devices, and
// connections. Provides signal-based updates for UI components.
//
// Responsibilities:
// - Bluetooth adapter status monitoring
// - Paired devices list management
// - Device connection/disconnection
// - Device discovery
//
// Usage:
// import "../services"
//
// Connections {
//     target: BluetoothService
//     function onBluetoothStatusChanged() { updateBluetoothIcon() }
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

    // Whether Bluetooth is powered on
    property bool isPowered: false

    // Whether Bluetooth is discoverable
    property bool isDiscoverable: false

    // Whether currently discovering devices
    property bool isDiscovering: false

    // List of paired devices
    property var pairedDevices: []

    // List of available (discovered) devices
    property var availableDevices: []

    // Currently connected device (if any)
    property var connectedDevice: null

    // Whether any device is connected
    property bool hasConnectedDevice: false

    // Number of connected devices
    property int connectedDeviceCount: 0

    // Bluetooth adapter name
    property string adapterName: ""

    // Bluetooth address
    property string adapterAddress: ""

    // ========================================================================
    // Signals
    // ========================================================================

    // Emitted when Bluetooth power state changes
    signal bluetoothStatusChanged()

    // Emitted when a device is connected
    signal deviceConnected(var device)

    // Emitted when a device is disconnected
    signal deviceDisconnected(var device)

    // Emitted when paired devices list changes
    signal pairedDevicesChanged()

    // Emitted when available devices list changes
    signal availableDevicesChanged()

    // ========================================================================
    // Private Properties
    // ========================================================================

    property var _updateTimer: Timer {
        interval: root.updateInterval
        running: true
        repeat: true
        onTriggered: root._updateStatus()
    }

    property var _statusProcess: Process {
        id: statusProcess
        command: []
        running: false

        onExited: function(code, status) {
            if (code === 0) {
                root._parseStatus(stdout)
            }
        }
    }

    property var _deviceProcess: Process {
        id: deviceProcess
        command: []
        running: false

        onExited: function(code, status) {
            if (code === 0) {
                root._parseDevices(stdout)
            }
        }
    }

    // ========================================================================
    // Public Methods
    // ========================================================================

    // Turn Bluetooth on
    function powerOn() {
        Core.Logger.info("Bluetooth power on requested", "BluetoothService")
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["bluetoothctl", "power", "on"]; running: true }',
            root
        )
        Qt.callLater(_updateStatus)
    }

    // Turn Bluetooth off
    function powerOff() {
        Core.Logger.info("Bluetooth power off requested", "BluetoothService")
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["bluetoothctl", "power", "off"]; running: true }',
            root
        )
        Qt.callLater(_updateStatus)
    }

    // Toggle Bluetooth power
    function togglePower() {
        if (isPowered) {
            powerOff()
        } else {
            powerOn()
        }
    }

    // Connect to a device by MAC address
    function connectDevice(address) {
        Core.Logger.info("Connecting to Bluetooth device: " + address, "BluetoothService")
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["bluetoothctl", "connect", "' + address + '"]; running: true; onExited: function(c) { root._updateStatus() } }',
            root
        )
    }

    // Disconnect from a device by MAC address
    function disconnectDevice(address) {
        Core.Logger.info("Disconnecting from Bluetooth device: " + address, "BluetoothService")
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["bluetoothctl", "disconnect", "' + address + '"]; running: true; onExited: function(c) { root._updateStatus() } }',
            root
        )
    }

    // Start device discovery
    function startDiscovery() {
        if (!isPowered || isDiscovering) return

        Core.Logger.info("Starting Bluetooth discovery", "BluetoothService")
        isDiscovering = true
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["bluetoothctl", "scan", "on"]; running: true }',
            root
        )

        // Stop discovery after 30 seconds
        Qt.callLater(function() {
            stopDiscovery()
        }, 30000)
    }

    // Stop device discovery
    function stopDiscovery() {
        if (!isDiscovering) return

        Core.Logger.info("Stopping Bluetooth discovery", "BluetoothService")
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["bluetoothctl", "scan", "off"]; running: true; onExited: function() { root.isDiscovering = false } }',
            root
        )
    }

    // Pair with a device
    function pairDevice(address) {
        Core.Logger.info("Pairing with Bluetooth device: " + address, "BluetoothService")
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["bluetoothctl", "pair", "' + address + '"]; running: true; onExited: function(c) { root._updateStatus() } }',
            root
        )
    }

    // Remove/forget a paired device
    function removeDevice(address) {
        Core.Logger.info("Removing Bluetooth device: " + address, "BluetoothService")
        const process = Qt.createQmlObject(
            'import Quickshell.Io; Process { command: ["bluetoothctl", "remove", "' + address + '"]; running: true; onExited: function(c) { root._updateStatus() } }',
            root
        )
    }

    // Get icon name based on current state
    function getIconName() {
        if (!isPowered) {
            return "bluetooth-disabled"
        }
        if (hasConnectedDevice) {
            return "bluetooth-active"
        }
        if (isDiscovering) {
            return "bluetooth-searching"
        }
        return "bluetooth"
    }

    // Force refresh
    function refresh() {
        _updateStatus()
    }

    // ========================================================================
    // Private Methods
    // ========================================================================

    function _updateStatus() {
        // Check adapter status using bluetoothctl
        statusProcess.command = ["bluetoothctl", "show"]
        statusProcess.running = true

        // Get device list
        deviceProcess.command = ["bluetoothctl", "devices"]
        deviceProcess.running = true
    }

    function _parseStatus(output) {
        const lines = output.split("\n")
        let wasPowered = root.isPowered

        for (let i = 0; i < lines.length; i++) {
            const line = lines[i].trim()

            if (line.startsWith("Powered:")) {
                root.isPowered = line.includes("yes")
            } else if (line.startsWith("Discoverable:")) {
                root.isDiscoverable = line.includes("yes")
            } else if (line.startsWith("Name:")) {
                root.adapterName = line.substring(6).trim()
            } else if (line.startsWith("Address:")) {
                root.adapterAddress = line.substring(9).trim()
            }
        }

        if (wasPowered !== root.isPowered) {
            bluetoothStatusChanged()
        }
    }

    function _parseDevices(output) {
        const devices = []
        const lines = output.split("\n")

        for (let i = 0; i < lines.length; i++) {
            const line = lines[i].trim()
            if (!line.startsWith("Device ")) continue

            // Parse: Device XX:XX:XX:XX:XX:XX DeviceName
            const match = line.match(/Device ([0-9A-F:]+) (.+)/i)
            if (match) {
                devices.push({
                    address: match[1],
                    name: match[2],
                    connected: false,  // Will be updated by info command
                    paired: true
                })
            }
        }

        // Get detailed info for each device
        root._updateDeviceDetails(devices)
    }

    function _updateDeviceDetails(devices) {
        let connectedCount = 0
        let connectedDev = null

        // Get info for each device to check connection status
        for (let i = 0; i < devices.length; i++) {
            const process = Qt.createQmlObject(
                'import Quickshell.Io; Process { command: ["bluetoothctl", "info", "' + devices[i].address + '"]; running: true; onExited: function(c) { if (c === 0) root._parseDeviceInfo(stdout, ' + i + ') } }',
                root
            )
        }

        root.pairedDevices = devices
        pairedDevicesChanged()
    }

    function _parseDeviceInfo(output, index) {
        if (index >= root.pairedDevices.length) return

        const lines = output.split("\n")
        let device = root.pairedDevices[index]

        for (let i = 0; i < lines.length; i++) {
            const line = lines[i].trim()

            if (line.startsWith("Name:")) {
                device.name = line.substring(6).trim()
            } else if (line.startsWith("Connected:")) {
                device.connected = line.includes("yes")
                if (device.connected) {
                    root.connectedDevice = device
                    root.hasConnectedDevice = true
                }
            } else if (line.startsWith("Icon:")) {
                device.icon = line.substring(6).trim()
            } else if (line.startsWith("Battery Percentage:")) {
                const match = line.match(/\((\d+)\)/)
                if (match) {
                    device.battery = parseInt(match[1])
                }
            }
        }

        // Recalculate connected count
        let count = 0
        for (let i = 0; i < root.pairedDevices.length; i++) {
            if (root.pairedDevices[i].connected) count++
        }
        root.connectedDeviceCount = count
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        _updateStatus()
        Core.Logger.info("BluetoothService initialized", "BluetoothService")
    }
}
