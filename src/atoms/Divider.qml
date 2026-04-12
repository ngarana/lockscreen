// Divider.qml - Visual Separator Component
//
// Horizontal or vertical divider line with optional label.
// Supports different thicknesses and colors.
//
// Usage:
//   Divider {
//       orientation: Qt.Horizontal
//       label: "Section"
//   }
//
// Properties:
//   - orientation: int - Qt.Horizontal or Qt.Vertical
//   - thickness: int - Line thickness in pixels (default: 1)
//   - color: color - Line color
//   - label: string - Optional label text
//   - labelPosition: string - "left", "center", "right" (for horizontal)
//   - spacing: int - Space around label

import QtQuick
import QtQuick.Layouts
import "../theme" as Theme

Item {
    id: root

    // Public API
    property int orientation: Qt.Horizontal
    property int thickness: 1
    property color lineColor: Theme.Theme.colors.surface1
    property string label: ""
    property string labelPosition: "center" // left, center, right
    property int spacing: 8

    // Layout
    implicitWidth: orientation === Qt.Horizontal ? 100 : thickness
    implicitHeight: orientation === Qt.Horizontal ? (label !== "" ? 24 : thickness) : 100

    // Horizontal divider with optional label
    RowLayout {
        id: horizontalLayout
        visible: root.orientation === Qt.Horizontal
        anchors.fill: parent
        spacing: root.spacing

        Rectangle {
            id: leftLine
            Layout.fillWidth: true
            Layout.minimumWidth: root.label !== "" && root.labelPosition !== "left" ? 16 : 0
            height: root.thickness
            color: root.lineColor
        }

        Label {
            id: labelItem
            visible: root.label !== ""
            text: root.label
            fontSize: Theme.Theme.typography.sizeSm
            color: Theme.Theme.colors.textSecondary
        }

        Rectangle {
            id: rightLine
            Layout.fillWidth: true
            Layout.minimumWidth: root.label !== "" && root.labelPosition !== "right" ? 16 : 0
            height: root.thickness
            color: root.lineColor
        }

        // Adjust line widths based on label position
        Component.onCompleted: {
            if (root.label !== "") {
                if (root.labelPosition === "left") {
                    leftLine.Layout.fillWidth = false
                    leftLine.Layout.minimumWidth = 0
                    rightLine.Layout.fillWidth = true
                } else if (root.labelPosition === "right") {
                    leftLine.Layout.fillWidth = true
                    rightLine.Layout.fillWidth = false
                    rightLine.Layout.minimumWidth = 0
                }
            }
        }
    }

    // Vertical divider
    Rectangle {
        id: verticalLine
        visible: root.orientation === Qt.Vertical
        anchors.centerIn: parent
        width: root.thickness
        height: parent.height
        color: root.lineColor
    }
}
