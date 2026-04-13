// Icon.qml - Base Icon Component
//
// Provides theme-integrated icon display with support for:
// - PNG/SVG files
// - System icon theme names (via image://icon/ provider)
// - Nerd Font unicode glyphs (rendered as text)
// - Color overlays for monochrome icons
// - Automatic layout and sizing
//
// Properties:
// - icon: url — The icon source (path, name, or unicode glyph)
// - size: int — Side length of the icon (default 24)
// - color: color — Color applied to monochrome icons
//
// Usage:
//   Icon {
//       icon: "network-wireless"
//       size: 16
//       color: Theme.colors.primary
//   }
//
//   // Or with Nerd Font glyph:
//   Icon {
//       icon: "󰕾"  // nf-md-volume_high
//       size: 16
//   }

import QtQuick
import Qt5Compat.GraphicalEffects
import "../theme" as Theme

Item {
    id: root

    // ========================================================================
    // Public API
    // ========================================================================

    property url icon: ""
    // Enforce minimum size of 14px - system icon themes don't have sizes below this
    property int size: 24
    property color color: Theme.ThemeEngine.colors.textPrimary

    // Compatibility alias for existing code
    property alias source: root.icon

    // ========================================================================
    // Layout
    // ========================================================================

    width: size
    height: size

    // ========================================================================
    // Internal Logic
    // ========================================================================

    // Detect if the icon is a unicode glyph (Nerd Font character or symbol)
    // These are typically single or multi-byte unicode characters
    readonly property bool _isUnicodeGlyph: {
        let src = icon.toString()
        if (!src || src.length === 0) return false
        
        // If it's a resolved file URL, extract the filename (the potential glyph)
        if (src.startsWith("file://")) {
            const parts = src.split("/")
            src = parts[parts.length - 1]
        }
        
        // Check if it's a single unicode character or a short unicode string
        if (src.length <= 2) {
            const code = src.codePointAt(0)
            // Check for common unicode symbol ranges:
            // - U+2190-U+21FF (Arrows: ↻ ↺ etc.)
            // - U+2300-U+23FF (Misc Technical: ⏻ ⏼ ⏾ ⏻ etc.)
            // - U+2700-U+27BF (Dingbats)
            // - U+E000-U+F8FF (BMP Private Use Area - Nerd Fonts)
            // - U+F000-U+FFFF (Supplementary Private Use Area / Nerd Fonts)
            // - U+F0000-U+FFFFD (Supplementary Private Use Area-A)
            // - U+100000-U+10FFFD (Supplementary Private Use Area-B)
            return (code >= 0x2190 && code <= 0x21FF) ||  // Arrows
                   (code >= 0x2300 && code <= 0x23FF) ||  // Misc Technical (power symbols)
                   (code >= 0x2700 && code <= 0x27BF) ||  // Dingbats
                   (code >= 0xE000 && code <= 0xF8FF) ||  // BMP PUA (Nerd Fonts)
                   (code >= 0xF000 && code <= 0xFFFF) ||  // Nerd Font range
                   (code >= 0xF0000 && code <= 0xFFFFD) || // PUA-A
                   (code >= 0x100000 && code <= 0x10FFFD)  // PUA-B
        }
        return false
    }

    readonly property string _resolvedSource: {
        const src = icon.toString()
        if (!src) return ""
        
        // If it's a unicode glyph, don't resolve as image path
        if (root._isUnicodeGlyph) return ""
        
        // If it's already an explicit provider or absolute path, use as is
        if (src.startsWith("image://") || src.startsWith("/") || src.startsWith("qrc:/")) return src
        
        // If it's a local file URL resolved by QML
        if (src.startsWith("file://")) {
            const parts = src.split("/")
            const fileName = parts[parts.length - 1]
            // If the filename has no extension, it's likely a system icon name
            if (!fileName.includes(".")) {
                return "image://icon/" + fileName
            }
        }
        
        // Fallback for simple names
        if (!src.includes("/") && !src.includes(".")) {
            return "image://icon/" + src
        }
        
        return src
    }

    readonly property bool _isSvg: icon.toString().toLowerCase().endsWith('.svg')

    // ========================================================================
    // Text-based Icon (for Nerd Font glyphs)
    // ========================================================================

    readonly property string _glyph: {
        let src = icon.toString()
        if (src.startsWith("file://")) {
            const parts = src.split("/")
            return parts[parts.length - 1]
        }
        return src
    }

    Text {
        id: textIcon
        anchors.centerIn: parent
        text: root._glyph
        font.family: Theme.ThemeEngine.fonts.iconFontFamily
        font.pixelSize: root.size
        color: root.color
        visible: root._isUnicodeGlyph
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    // ========================================================================
    // Image Component
    // ========================================================================

    Image {
        id: image
        anchors.centerIn: parent
        source: root._resolvedSource
        // Correct square icon rendering with proper source size
        sourceSize.width: root.size
        sourceSize.height: root.size
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: true
        asynchronous: true
        cache: true
        // Hide image when rendering as unicode glyph
        visible: !root._isUnicodeGlyph && root._resolvedSource !== ""

        // Internal state to prevent loops
        property bool _triedSymbolic: false
        property bool _triedGeneric: false

        // Reset state when source changes
        onSourceChanged: {
            _triedSymbolic = false
            _triedGeneric = false
        }

        // Handle loading errors with smart fallbacks
        onStatusChanged: {
            if (status === Image.Error) {
                const currentSource = source.toString()
                
                // If it's a system icon request that failed
                if (currentSource.startsWith("image://icon/")) {
                    const iconName = currentSource.replace("image://icon/", "")
                    
                    // 1. Try appending -symbolic
                    if (!_triedSymbolic && !iconName.endsWith("-symbolic")) {
                        _triedSymbolic = true
                        source = "image://icon/" + iconName + "-symbolic"
                        return
                    }
                    
                    // 2. Try generic category icons
                    if (!_triedGeneric) {
                        _triedGeneric = true
                        if (iconName.includes("network") || iconName.includes("wifi")) {
                            source = "image://icon/network-workgroup-symbolic"
                        } else if (iconName.includes("bluetooth")) {
                            source = "image://icon/bluetooth-symbolic"
                        } else if (iconName.includes("volume") || iconName.includes("audio")) {
                            source = "image://icon/audio-volume-high-symbolic"
                        } else {
                            source = "image://icon/application-x-executable-symbolic"
                        }
                        return
                    }
                }
                
                // 3. Final fallback (empty to suppress warnings if theme is incomplete)
                if (currentSource !== "") {
                    source = ""
                }
            }
        }
    }

    // ========================================================================
    // Color Overlay (for image-based icons)
    // ========================================================================

    ColorOverlay {
        anchors.fill: image
        source: image
        color: root.color
        // Apply color overlay for all monochrome icons to match theme
        visible: image.status === Image.Ready && !root._isUnicodeGlyph
    }
}
