// Label.qml - Text Label Component
//
// Text label with theme typography support. Includes text truncation,
// color variants, and style presets.
//
// Usage:
//   Label {
//       text: "Hello World"
//       variant: "body"
//       color: Theme.colors.textSecondary
//   }
//
// Properties:
//   - text: string - Label text
//   - variant: string - Typography variant: "h1", "h2", "h3", "h4", "body", "caption", "button"
//   - fontSize: int - Override font size
//   - fontWeight: int - Override font weight
//   - color: color - Text color
//   - truncate: bool - Enable text truncation with ellipsis
//   - wrap: bool - Enable text wrapping
//   - maxLines: int - Maximum lines for wrapping

import QtQuick
import "../theme" as Theme

Text {
    id: root

    // Public API
    property string variant: "body" // h1, h2, h3, h4, body, caption, button, clock
    property int fontSize: -1 // -1 means use variant default
    property int fontWeight: -1 // -1 means use variant default
    property bool truncate: false
    property bool wrap: false
    property int maxLines: 0 // 0 means unlimited

    // Variant configuration
    readonly property var _variants: ({
        h1: { size: Theme.Theme.typography.h1.size, weight: Theme.Theme.typography.h1.weight },
        h2: { size: Theme.Theme.typography.h2.size, weight: Theme.Theme.typography.h2.weight },
        h3: { size: Theme.Theme.typography.h3.size, weight: Theme.Theme.typography.h3.weight },
        h4: { size: Theme.Theme.typography.h4.size, weight: Theme.Theme.typography.h4.weight },
        body: { size: Theme.Theme.typography.bodyLg.size, weight: Theme.Theme.typography.bodyLg.weight },
        caption: { size: Theme.Theme.typography.caption.size, weight: Theme.Theme.typography.caption.weight },
        button: { size: Theme.Theme.typography.button.size, weight: Theme.Theme.typography.button.weight },
        clock: { size: Theme.Theme.typography.clock.size, weight: Theme.Theme.typography.clock.weight }
    })

    readonly property var _currentVariant: _variants[variant] || _variants.body

    // Typography
    font.pixelSize: fontSize > 0 ? fontSize : _currentVariant.size
    font.weight: fontWeight >= 0 ? fontWeight : _currentVariant.weight
    font.family: Theme.Theme.typography.fontFamily

    // Color
    color: Theme.Theme.colors.textPrimary

    // Layout
    elide: truncate ? Text.ElideRight : Text.ElideNone
    wrapMode: wrap ? (maxLines > 0 ? Text.Wrap : Text.WordWrap) : Text.NoWrap
    maximumLineCount: maxLines > 0 ? maxLines : 999999
    lineHeightMode: Text.FixedHeight
    lineHeight: {
        if (variant === "h1") return Theme.Theme.typography.h1.lineHeight * font.pixelSize
        if (variant === "h2") return Theme.Theme.typography.h2.lineHeight * font.pixelSize
        if (variant === "h3") return Theme.Theme.typography.h3.lineHeight * font.pixelSize
        if (variant === "h4") return Theme.Theme.typography.h4.lineHeight * font.pixelSize
        if (variant === "body") return Theme.Theme.typography.bodyLg.lineHeight * font.pixelSize
        return font.pixelSize * 1.5
    }

    // Rendering
    renderType: Text.NativeRendering
    antialiasing: true
}
