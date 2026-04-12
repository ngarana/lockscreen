// LayerLayout.qml - Z-Index Layering Helper
//
// Manages stacking context and z-index layering for UI elements.
// Provides named layers with consistent z-ordering.
//
// Usage:
// LayerLayout {
//     LayerLayout.layer: "background"
//     Item { ... }
// }
//
// Properties:
// - layer: string - Named layer: "background", "content", "overlay", "modal", "tooltip" (default: "content")
// - layers: var - Object mapping layer names to z-index values
//
// Layers (default z-order):
// - background: 0
// - content: 10
// - overlay: 20
// - modal: 30
// - tooltip: 40
// - top: 50

import QtQuick
import "../theme" as Theme

Item {
    id: root

    // Layer definitions
    readonly property var layers: ({
        "background": 0,
        "content": 10,
        "overlay": 20,
        "modal": 30,
        "tooltip": 40,
        "top": 50
    })

    // Default layer for children
    property string defaultLayer: "content"

    // Get z-index for layer name
    function zIndex(layerName) {
        return layers[layerName] !== undefined ? layers[layerName] : layers.content
    }

    // Layer container component
    component LayerContainer: Item {
        property string layerName: "content"
        z: root.zIndex(layerName)
        anchors.fill: parent
    }

    // Background layer
    Item {
        id: backgroundLayer
        objectName: "internal"
        z: layers.background
        anchors.fill: parent

        default property alias children: backgroundLayer._children
        property var _children: []
    }

    // Content layer
    Item {
        id: contentLayer
        objectName: "internal"
        z: layers.content
        anchors.fill: parent

        default property alias children: contentLayer._children
        property var _children: []
    }

    // Overlay layer
    Item {
        id: overlayLayer
        objectName: "internal"
        z: layers.overlay
        anchors.fill: parent

        default property alias children: overlayLayer._children
        property var _children: []
    }

    // Modal layer
    Item {
        id: modalLayer
        objectName: "internal"
        z: layers.modal
        anchors.fill: parent

        default property alias children: modalLayer._children
        property var _children: []
    }

    // Tooltip layer
    Item {
        id: tooltipLayer
        objectName: "internal"
        z: layers.tooltip
        anchors.fill: parent

        default property alias children: tooltipLayer._children
        property var _children: []
    }

    // Top layer (highest z-index)
    Item {
        id: topLayer
        objectName: "internal"
        z: layers.top
        anchors.fill: parent

        default property alias children: topLayer._children
        property var _children: []
    }

    // Add item to specific layer
    function addToLayer(item, layerName) {
        switch (layerName) {
            case "background": item.parent = backgroundLayer; break
            case "content": item.parent = contentLayer; break
            case "overlay": item.parent = overlayLayer; break
            case "modal": item.parent = modalLayer; break
            case "tooltip": item.parent = tooltipLayer; break
            case "top": item.parent = topLayer; break
            default: item.parent = contentLayer
        }
    }
}
