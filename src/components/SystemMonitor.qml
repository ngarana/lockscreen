// SystemMonitor.qml - System Resource Monitor Component
//
// A glassmorphic system monitor showing CPU, RAM, and other
// resource usage with real-time updates and optional graphs.
//
// Properties:
//   - cpuUsage: real - CPU usage percentage (0-100)
//   - ramUsage: real - RAM usage percentage (0-100)
//   - ramUsed: real - RAM used in GB
//   - ramTotal: real - Total RAM in GB
//   - updateInterval: int - Update interval in ms
//   - showGraph: bool - Show usage graphs
//   - graphHistory: int - Number of data points to keep
//
// Signals:
//   - refreshRequested(): Emitted when user requests refresh

import QtQuick
import QtQuick.Layouts
import "../services" as Services
import "../atoms" as Atoms
import "../theme" as Theme

Item {
    id: root

    implicitWidth: 240
    implicitHeight: showGraph ? 200 : 140

    // ====== Public Properties ======

    property real cpuUsage: 0
    property real ramUsage: 0
    property real ramUsed: 0
    property real ramTotal: 0
    property int updateInterval: 1000
    property bool showGraph: true
    property int graphHistory: 60

    // Additional metrics
    property real diskUsage: 0
    property real networkDown: 0  // KB/s
    property real networkUp: 0    // KB/s

    // Internal state
    property list<real> _cpuHistory: []
    property list<real> _ramHistory: []

    // Signals
    signal refreshRequested()

    // ====== History Management ======

    function _addCpuPoint(value) {
        var history = _cpuHistory.slice()
        history.push(value)
        if (history.length > graphHistory) {
            history.shift()
        }
        _cpuHistory = history
    }

    function _addRamPoint(value) {
        var history = _ramHistory.slice()
        history.push(value)
        if (history.length > graphHistory) {
            history.shift()
        }
        _ramHistory = history
    }

    // ====== Auto-update ======

    Timer {
        id: updateTimer
        interval: root.updateInterval
        running: true
        repeat: true
        onTriggered: {
            root._addCpuPoint(root.cpuUsage)
            root._addRamPoint(root.ramUsage)
        }
    }

    // ====== Glassmorphic Panel ======

    Rectangle {
        id: panel
        anchors.fill: parent

        color: Theme.ThemeEngine.colors.glass
        radius: Theme.ThemeEngine.radius.large
        border.width: 1
        border.color: Theme.ThemeEngine.colors.glassBorder

        ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            anchors.margins: Theme.ThemeEngine.spacing.medium
            spacing: Theme.ThemeEngine.spacing.medium

            // ====== CPU Section ======

            ColumnLayout {
                id: cpuSection
                Layout.fillWidth: true
                spacing: 4

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.ThemeEngine.spacing.small

                    Text {
                        text: "CPU"
                        font.pixelSize: 12
                        font.family: Theme.ThemeEngine.fonts.fontFamily
                        font.weight: Font.Medium
                        color: Theme.ThemeEngine.colors.text
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        text: root.cpuUsage.toFixed(1) + "%"
                        font.pixelSize: 12
                        font.family: Theme.ThemeEngine.fonts.fontFamily
                        color: Theme.ThemeEngine.colors.textMuted
                    }
                }

                // CPU progress bar
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 6
                    radius: 3
                    color: Theme.ThemeEngine.colors.glassBorder

                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: parent.width * (root.cpuUsage / 100)
                        radius: 3
                        color: _getUsageColor(root.cpuUsage)
                    }
                }

                // CPU graph (optional)
                Canvas {
                    id: cpuGraph
                    visible: root.showGraph && root._cpuHistory.length > 1
                    Layout.fillWidth: true
                    implicitHeight: 40

                    onPaint: {
                        var ctx = getContext('2d')
                        ctx.clearRect(0, 0, width, height)

                        var history = root._cpuHistory
                        if (history.length < 2) return

                        ctx.beginPath()
                        ctx.strokeStyle = Theme.ThemeEngine.colors.primary
                        ctx.lineWidth = 2

                        var stepX = width / (root.graphHistory - 1)
                        for (var i = 0; i < history.length; i++) {
                            var x = i * stepX
                            var y = height - (history[i] / 100 * height)
                            if (i === 0) {
                                ctx.moveTo(x, y)
                            } else {
                                ctx.lineTo(x, y)
                            }
                        }
                        ctx.stroke()

                        // Fill gradient
                        ctx.lineTo(width, height)
                        ctx.lineTo(0, height)
                        ctx.closePath()
                        ctx.fillStyle = Qt.rgba(
                            Theme.ThemeEngine.colors.primary.r,
                            Theme.ThemeEngine.colors.primary.g,
                            Theme.ThemeEngine.colors.primary.b,
                            0.2
                        )
                        ctx.fill()
                    }

                    Connections {
                        target: root
                        function on_CpuHistoryChanged() {
                            cpuGraph.requestPaint()
                        }
                    }
                }
            }

            // ====== RAM Section ======

            ColumnLayout {
                id: ramSection
                Layout.fillWidth: true
                spacing: 4

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.ThemeEngine.spacing.small

                    Text {
                        text: "RAM"
                        font.pixelSize: 12
                        font.family: Theme.ThemeEngine.fonts.fontFamily
                        font.weight: Font.Medium
                        color: Theme.ThemeEngine.colors.text
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        text: root.ramUsed.toFixed(1) + " / " + root.ramTotal.toFixed(1) + " GB"
                        font.pixelSize: 12
                        font.family: Theme.ThemeEngine.fonts.fontFamily
                        color: Theme.ThemeEngine.colors.textMuted
                    }
                }

                // RAM progress bar
                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 6
                    radius: 3
                    color: Theme.ThemeEngine.colors.glassBorder

                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: parent.width * (root.ramUsage / 100)
                        radius: 3
                        color: _getUsageColor(root.ramUsage)
                    }
                }

                // RAM graph (optional)
                Canvas {
                    id: ramGraph
                    visible: root.showGraph && root._ramHistory.length > 1
                    Layout.fillWidth: true
                    implicitHeight: 40

                    onPaint: {
                        var ctx = getContext('2d')
                        ctx.clearRect(0, 0, width, height)

                        var history = root._ramHistory
                        if (history.length < 2) return

                        ctx.beginPath()
                        ctx.strokeStyle = Theme.ThemeEngine.colors.success
                        ctx.lineWidth = 2

                        var stepX = width / (root.graphHistory - 1)
                        for (var i = 0; i < history.length; i++) {
                            var x = i * stepX
                            var y = height - (history[i] / 100 * height)
                            if (i === 0) {
                                ctx.moveTo(x, y)
                            } else {
                                ctx.lineTo(x, y)
                            }
                        }
                        ctx.stroke()

                        // Fill gradient
                        ctx.lineTo(width, height)
                        ctx.lineTo(0, height)
                        ctx.closePath()
                        ctx.fillStyle = Qt.rgba(
                            Theme.ThemeEngine.colors.success.r,
                            Theme.ThemeEngine.colors.success.g,
                            Theme.ThemeEngine.colors.success.b,
                            0.2
                        )
                        ctx.fill()
                    }

                    Connections {
                        target: root
                        function on_RamHistoryChanged() {
                            ramGraph.requestPaint()
                        }
                    }
                }
            }

            // ====== Network/Disk Stats (optional) ======

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.ThemeEngine.spacing.large

                // Disk
                Row {
                    spacing: 4
                    visible: root.diskUsage > 0
                    Text {
                        text: "💾"
                        font.pixelSize: 12
                    }
                    Text {
                        text: root.diskUsage.toFixed(0) + "%"
                        font.pixelSize: 11
                        font.family: Theme.ThemeEngine.fonts.fontFamily
                        color: Theme.ThemeEngine.colors.textMuted
                    }
                }

                // Network
                Row {
                    spacing: 4
                    visible: root.networkDown > 0 || root.networkUp > 0
                    Text {
                        text: "↓"
                        font.pixelSize: 12
                        color: Theme.ThemeEngine.colors.success
                    }
                    Text {
                        text: (root.networkDown / 1024).toFixed(1) + " MB/s"
                        font.pixelSize: 11
                        font.family: Theme.ThemeEngine.fonts.fontFamily
                        color: Theme.ThemeEngine.colors.textMuted
                    }
                    Text {
                        text: "↑"
                        font.pixelSize: 12
                        color: Theme.ThemeEngine.colors.primary
                    }
                    Text {
                        text: (root.networkUp / 1024).toFixed(1) + " MB/s"
                        font.pixelSize: 11
                        font.family: Theme.ThemeEngine.fonts.fontFamily
                        color: Theme.ThemeEngine.colors.textMuted
                    }
                }

                Item { Layout.fillWidth: true }
            }
        }
    }

    // ====== Helper Functions ======

    function _getUsageColor(usage) {
        if (usage >= 90) return Theme.ThemeEngine.colors.error
        if (usage >= 70) return Theme.ThemeEngine.colors.warning
        return Theme.ThemeEngine.colors.primary
    }
}
