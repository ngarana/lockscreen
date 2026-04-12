// PanelLayout.qml - Horizontal Panel Layout
//
// A horizontal panel layout for status bars and toolbars.
// Provides configurable spacing, alignment, and item management.
//
// Usage:
// PanelLayout {
//     spacing: 8
//     alignment: "center"
//     Item { ... }
// }
//
// Properties:
// - spacing: int - Space between items (default: 4)
// - alignment: string - "left", "center", "right" (default: "left")
// - leftMargin: int - Left padding (default: 0)
// - rightMargin: int - Right padding (default: 0)
// - verticalAlignment: string - "top", "center", "bottom" (default: "center")

import QtQuick
import QtQuick.Layouts
import "../theme" as Theme

Item {
    id: root

    property int spacing: 4
    property string alignment: "left"
    property int leftMargin: 0
    property int rightMargin: 0
    property string verticalAlignment: "center"

    implicitWidth: rowLayout.implicitWidth + leftMargin + rightMargin
    implicitHeight: rowLayout.implicitHeight

    RowLayout {
        id: rowLayout
        anchors.fill: parent
        anchors.leftMargin: root.leftMargin
        anchors.rightMargin: root.rightMargin

        spacing: root.spacing

        // Push items based on alignment
        Item {
            visible: root.alignment === "center" || root.alignment === "right"
            Layout.fillWidth: true
        }

        // Container for children - re-parent them here
        Item {
            id: contentContainer
            Layout.fillWidth: root.alignment === "left"
            implicitWidth: childrenRect.width
            implicitHeight: childrenRect.height

            Row {
                id: contentRow
                spacing: root.spacing

                property int _vAlignOffset: {
                    switch (root.verticalAlignment) {
                        case "top": return 0
                        case "bottom": return root.height - height
                        default: return (root.height - height) / 2
                    }
                }

                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: root.verticalAlignment === "center" ? 0 :
                    root.verticalAlignment === "top" ? -(root.height - height) / 2 :
                    (root.height - height) / 2
            }
        }

        Item {
            visible: root.alignment === "center" || root.alignment === "left"
            Layout.fillWidth: true
        }
    }

    default property alias children: contentRow.children
}
