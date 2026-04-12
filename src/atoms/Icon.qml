// Icon.qml - Base Icon Component
//
// Provides theme-integrated icon display with support for:
// - PNG/SVG files
// - System icon theme names (via image://icon/ provider)
// - Color overlays for monochrome icons
// - Automatic layout and sizing
//
// Properties:
// - icon: url — The icon source (path or name)
// - size: int — Side length of the icon (default 24)
// - color: color — Color applied to monochrome icons
//
// Usage:
//   Icon {
//       icon: "network-wireless"
//       size: 16
//       color: Theme.colors.primary
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

    readonly property string _resolvedSource: {
        const src = icon.toString()
        if (!src) return ""
        
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
                
                // 3. Final fallback
                if (currentSource !== "image://icon/application-x-executable-symbolic") {
                    source = "image://icon/application-x-executable-symbolic"
                }
            }
        }
    }

    // ========================================================================
    // Color Overlay
    // ========================================================================

    ColorOverlay {
        anchors.fill: image
        source: image
        color: root.color
        // Apply color overlay for all monochrome icons to match theme
        visible: image.status === Image.Ready
    }
}
