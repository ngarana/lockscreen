// Icon.qml - Icon Component
//
// Icon wrapper with theme support. Handles SVG/PNG loading,
// size presets, and color inheritance.
//
// Usage:
//   Icon {
//       source: "icons/home.svg"
//       size: 24
//       color: Theme.colors.primary
//   }
//
// Properties:
//   - source: string - Icon file path or icon name
//   - size: int - Icon size in pixels (default: 24)
//   - color: color - Icon color (default: inherits from parent)
//   - rotation: int - Rotation in degrees
//   - smooth: bool - Use smooth scaling (default: true)

import QtQuick
import "../theme" as Theme

Image {
    id: root

    // Public API
    property int size: 24
    property color color: Theme.Theme.colors.text
    property int rotation: 0

    // Internal
    property bool _isSvg: source.toString().toLowerCase().endsWith('.svg')

    // Layout
    width: size
    height: size

    // Visual
    sourceSize.width: size
    sourceSize.height: size
    fillMode: Image.PreserveAspectFit
    smooth: true
    mipmap: true
    rotation: root.rotation

    // Color overlay for monochrome icons
    ColorOverlay {
        anchors.fill: parent
        source: parent
        color: root.color
        visible: root._isSvg || root.color !== Theme.Theme.colors.text
    }

    // Handle loading errors
    onStatusChanged: {
        if (status === Image.Error) {
            // Could set a fallback icon here
            console.warn("Failed to load icon: " + source)
        }
    }
}
