pragma Singleton
import Quickshell
import QtQuick

QtObject {
    id: theme

    property var colors: QtObject {
        readonly property color background: "#1e1e2e"
        readonly property color surface: "#313244"
        readonly property color surfaceHover: "#45475a"
        readonly property color primary: "#89b4fa"
        readonly property color text: "#cdd6f4"
        readonly property color textSubtle: "#a6adc8"
        readonly property color textMuted: "#6c7086"
        readonly property color error: "#f38ba8"
        readonly property color success: "#a6e3a1"
        readonly property color warning: "#f9e2af"
    }

    property var fonts: QtObject {
        readonly property int textSize: 16
        readonly property int textSizeLarge: 24
        readonly property int textSizeClock: 72
        readonly property int textSizeDate: 24
    }

    property var spacing: QtObject {
        readonly property int small: 8
        readonly property int medium: 16
        readonly property int large: 24
        readonly property int xlarge: 40
    }

    property var radius: QtObject {
        readonly property int small: 4
        readonly property int medium: 8
        readonly property int large: 12
        readonly property int round: 9999
    }

    property var animation: QtObject {
        readonly property int fast: 150
        readonly property int medium: 250
        readonly property int slow: 400
    }
}
