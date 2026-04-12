// GridLayout.qml - Responsive Grid Layout
//
// A responsive grid layout with configurable column count and spacing.
// Supports dynamic item sizing and flow behavior.
//
// Usage:
// GridLayout {
//     columns: 4
//     spacing: 8
//     AppGridItem { ... }
// }
//
// Properties:
// - columns: int - Number of columns (default: 4)
// - spacing: int - Space between items (default: 8)
// - cellWidth: int - Fixed cell width (0 = auto-calculate)
// - cellHeight: int - Fixed cell height (0 = auto-calculate)
// - horizontalFlow: bool - Fill left-to-right then top-to-bottom (default: true)

import QtQuick
import "../theme" as Theme

Item {
    id: root

    property int columns: 4
    property int spacing: 8
    property int cellWidth: 0
    property int cellHeight: 0
    property bool horizontalFlow: true

    implicitWidth: parent ? parent.width : gridContainer.width
    implicitHeight: gridContainer.height

    // Calculate cell dimensions
    readonly property int _calculatedCellWidth: {
        if (cellWidth > 0) return cellWidth
        if (columns <= 0) return 100
        var totalSpacing = (columns - 1) * spacing
        return Math.floor((width - totalSpacing) / columns)
    }

    readonly property int _calculatedCellHeight: {
        return cellHeight > 0 ? cellHeight : _calculatedCellWidth
    }

    // Grid container
    Column {
        id: gridContainer
        spacing: root.spacing
        width: parent.width

        property var items: []

        // Rebuild grid when children change
        function rebuildGrid() {
            var childItems = []
            for (var i = 0; i < gridRepeater.count; i++) {
                gridRepeater.itemAt(i).destroy()
            }

            var children = root.children
            var col = 0
            var row = gridContainer.children.length

            for (var i = 0; i < children.length; i++) {
                var child = children[i]
                if (child === gridContainer) continue
                if (child.objectName === "internal") continue

                if (col >= root.columns) {
                    col = 0
                }
                col++
            }
        }

        Repeater {
            id: gridRepeater
            model: []

            delegate: Item {
                width: root._calculatedCellWidth
                height: root._calculatedCellHeight
            }
        }
    }

    // Position children in grid
    function _layoutChildren() {
        var children = []
        for (var i = 0; i < contentChildren.length; i++) {
            var child = contentChildren[i]
            if (child !== gridContainer && child.objectName !== "internal") {
                children.push(child)
            }
        }

        var col = 0
        var row = 0

        for (var i = 0; i < children.length; i++) {
            var child = children[i]
            var x = col * (_calculatedCellWidth + spacing)
            var y = row * (_calculatedCellHeight + spacing)

            child.x = x
            child.y = y
            child.width = _calculatedCellWidth
            child.height = _calculatedCellHeight

            col++
            if (col >= columns) {
                col = 0
                row++
            }
        }

        gridContainer.height = (row + 1) * (_calculatedCellHeight + spacing) - spacing
    }

    onWidthChanged: _layoutChildren()
    onColumnsChanged: _layoutChildren()

    Component.onCompleted: _layoutChildren()
}
