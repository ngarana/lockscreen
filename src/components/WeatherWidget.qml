// WeatherWidget.qml - Weather Display Component
//
// A glassmorphic weather widget showing current conditions
// and optional forecast data.
//
// Properties:
//   - location: string - Location name to display
//   - temperature: real - Current temperature
//   - condition: string - Weather condition text
//   - icon: string - Weather icon (emoji or icon name)
//   - humidity: int - Humidity percentage
//   - windSpeed: real - Wind speed
//   - unit: string - "celsius" or "fahrenheit"
//   - showForecast: bool - Show forecast row
//   - forecastData: array - Array of forecast objects
//
// Signals:
//   - refreshRequested(): Emitted when user requests refresh

import QtQuick
import QtQuick.Layouts
import "../services" as Services
import "../atoms" as Atoms

Item {
    id: root

    implicitWidth: 280
    implicitHeight: showForecast ? 180 : 120

    // ====== Public Properties ======

    property string location: "Unknown"
    property real temperature: 0
    property string condition: "Clear"
    property string icon: "☀️"
    property int humidity: 0
    property real windSpeed: 0
    property string unit: "celsius"
    property bool showForecast: false
    property var forecastData: []  // [{day, icon, high, low}, ...]

    // Loading state
    property bool loading: false
    property bool hasError: false
    property string errorMessage: ""

    // Signals
    signal refreshRequested()

    // ====== Internal ======

    readonly property string _tempUnit: unit === "celsius" ? "°C" : "°F"
    readonly property string _speedUnit: "km/h"

    // ====== Glassmorphic Panel ======

    Rectangle {
        id: panel
        anchors.fill: parent

        color: Services.Theme.colors.glass
        radius: Services.Theme.radius.large
        border.width: 1
        border.color: Services.Theme.colors.glassBorder

        ColumnLayout {
            id: mainLayout
            anchors.fill: parent
            anchors.margins: Services.Theme.spacing.medium
            spacing: Services.Theme.spacing.small

            // ====== Header: Location + Refresh ======

            RowLayout {
                id: headerLayout
                Layout.fillWidth: true
                spacing: Services.Theme.spacing.small

                Text {
                    id: locationText
                    text: root.location
                    font.pixelSize: 14
                    font.family: Services.Theme.fonts.fontFamily
                    font.weight: Font.Medium
                    color: Services.Theme.colors.text
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Atoms.IconButton {
                    implicitWidth: 28
                    implicitHeight: 28
                    icon: root.loading ? "⟳" : "↻"
                    iconSize: 14
                    onClicked: root.refreshRequested()

                    rotation: root.loading ? 360 : 0
                    Behavior on rotation {
                        NumberAnimation {
                            duration: root.loading ? 1000 : 0
                            loops: Animation.Infinite
                        }
                    }
                }
            }

            // ====== Error State ======

            RowLayout {
                visible: root.hasError
                Layout.fillWidth: true

                Text {
                    text: "⚠ " + root.errorMessage
                    font.pixelSize: 12
                    font.family: Services.Theme.fonts.fontFamily
                    color: Services.Theme.colors.warning
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }
            }

            // ====== Current Weather ======

            RowLayout {
                id: currentLayout
                visible: !root.hasError
                Layout.fillWidth: true
                spacing: Services.Theme.spacing.medium

                // Weather icon
                Text {
                    id: weatherIcon
                    text: root.icon
                    font.pixelSize: 48
                    Layout.alignment: Qt.AlignVCenter
                }

                // Temperature and condition
                ColumnLayout {
                    spacing: 4
                    Layout.fillWidth: true

                    Text {
                        id: tempText
                        text: Math.round(root.temperature) + root._tempUnit
                        font.pixelSize: 32
                        font.family: Services.Theme.fonts.fontFamily
                        font.weight: Font.Bold
                        color: Services.Theme.colors.text
                    }

                    Text {
                        id: conditionText
                        text: root.condition
                        font.pixelSize: 14
                        font.family: Services.Theme.fonts.fontFamily
                        color: Services.Theme.colors.textSubtle
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                }
            }

            // ====== Details: Humidity, Wind ======

            RowLayout {
                visible: !root.hasError
                Layout.fillWidth: true
                spacing: Services.Theme.spacing.large

                // Humidity
                Row {
                    spacing: 4
                    Text {
                        text: "💧"
                        font.pixelSize: 14
                    }
                    Text {
                        text: root.humidity + "%"
                        font.pixelSize: 12
                        font.family: Services.Theme.fonts.fontFamily
                        color: Services.Theme.colors.textMuted
                    }
                }

                // Wind
                Row {
                    spacing: 4
                    Text {
                        text: "💨"
                        font.pixelSize: 14
                    }
                    Text {
                        text: root.windSpeed.toFixed(1) + " " + root._speedUnit
                        font.pixelSize: 12
                        font.family: Services.Theme.fonts.fontFamily
                        color: Services.Theme.colors.textMuted
                    }
                }

                Item { Layout.fillWidth: true }
            }

            // ====== Forecast Row ======

            RowLayout {
                id: forecastRow
                visible: root.showForecast && root.forecastData.length > 0 && !root.hasError
                Layout.fillWidth: true
                spacing: Services.Theme.spacing.small

                Repeater {
                    model: root.forecastData.slice(0, 5)  // Max 5 days

                    delegate: Item {
                        implicitWidth: forecastColumn.implicitWidth
                        implicitHeight: forecastColumn.implicitHeight
                        Layout.fillWidth: true

                        Column {
                            id: forecastColumn
                            anchors.centerIn: parent
                            spacing: 4

                            Text {
                                text: modelData.day
                                font.pixelSize: 10
                                font.family: Services.Theme.fonts.fontFamily
                                color: Services.Theme.colors.textMuted
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Text {
                                text: modelData.icon
                                font.pixelSize: 20
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Text {
                                text: modelData.high + "°"
                                font.pixelSize: 11
                                font.family: Services.Theme.fonts.fontFamily
                                font.weight: Font.Medium
                                color: Services.Theme.colors.text
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Text {
                                text: modelData.low + "°"
                                font.pixelSize: 10
                                font.family: Services.Theme.fonts.fontFamily
                                color: Services.Theme.colors.textMuted
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }
                }
            }
        }
    }
}
