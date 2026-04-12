// GlassPanel.qml - Glassmorphic Container Component
//
// A reusable glassmorphic panel container with configurable blur,
// borders, and shadow effects. Provides a consistent glass-like
// appearance for overlays, dialogs, and panels.
//
// Properties:
//   - blurIntensity: Blur strength (0.0 - 1.0), affects background opacity
//   - borderColor: Custom border color (defaults to theme glassBorder)
//   - borderWidth: Border thickness
//   - radius: Corner radius (defaults to theme large)
//   - shadowEnabled: Whether to show drop shadow
//   - shadowRadius: Shadow blur radius
//   - shadowColor: Shadow color with alpha
//   - elevation: Elevation level (0-3) for shadow intensity
//
// Usage:
//   GlassPanel {
//       width: 300; height: 200
//       elevation: 2
//       // content here
//   }

import QtQuick
import QtQuick.Effects
import "../services" as Services

Item {
    id: root

    implicitWidth: 200
    implicitHeight: 100

    // ====== Public Properties ======

    // Blur/transparency
    property real blurIntensity: 0.4
    property color backgroundColor: Services.Theme.colors.glass

    // Border
    property color borderColor: Services.Theme.colors.glassBorder
    property int borderWidth: 1

    // Radius
    property int radius: Services.Theme.radius.large

    // Shadow
    property bool shadowEnabled: true
    property int elevation: 1  // 0 = none, 1 = subtle, 2 = medium, 3 = strong

    // Content padding (convenience for consumers)
    property int padding: Services.Theme.spacing.medium

    // Active/highlighted state
    property bool active: false
    property color activeBorderColor: Services.Theme.colors.primary

    // ====== Computed Shadow Properties ======

    readonly property int _shadowRadius: _elevationToRadius(elevation)
    readonly property color _shadowColor: _elevationToColor(elevation)
    readonly property vector2d _shadowOffset: _elevationToOffset(elevation)

    function _elevationToRadius(elev) {
        switch (elev) {
            case 0: return 0
            case 1: return 8
            case 2: return 16
            case 3: return 24
            default: return 8
        }
    }

    function _elevationToColor(elev) {
        var alpha
        switch (elev) {
            case 0: alpha = 0.0; break
            case 1: alpha = 0.15; break
            case 2: alpha = 0.25; break
            case 3: alpha = 0.35; break
            default: alpha = 0.15
        }
        return Qt.rgba(0, 0, 0, alpha)
    }

    function _elevationToOffset(elev) {
        switch (elev) {
            case 0: return Qt.vector2d(0, 0)
            case 1: return Qt.vector2d(0, 2)
            case 2: return Qt.vector2d(0, 4)
            case 3: return Qt.vector2d(0, 8)
            default: return Qt.vector2d(0, 2)
        }
    }

    // ====== Shadow Layer ======

    Rectangle {
        id: shadowRect
        anchors.fill: parent
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: root._shadowOffset.x
        anchors.verticalCenterOffset: root._shadowOffset.y
        visible: root.shadowEnabled && root.elevation > 0
        color: root._shadowColor
        radius: root.radius
        // Expand slightly to create shadow effect
        z: -1
    }

    // ====== Main Panel ======

    Rectangle {
        id: panel
        anchors.fill: parent

        color: root.backgroundColor
        radius: root.radius
        border.width: root.borderWidth
        border.color: root.active ? root.activeBorderColor : root.borderColor

        // Smooth border color transition
        Behavior on border.color {
            ColorAnimation { duration: Services.Theme.animation.fast }
        }

        // Inner highlight gradient (subtle top highlight)
        Rectangle {
            anchors.fill: parent
            anchors.margins: root.borderWidth
            radius: parent.radius - root.borderWidth
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop {
                    position: 0.0
                    color: Qt.rgba(1, 1, 1, 0.05)
                }
                GradientStop {
                    position: 0.5
                    color: Qt.rgba(1, 1, 1, 0.0)
                }
                GradientStop {
                    position: 1.0
                    color: Qt.rgba(0, 0, 0, 0.02)
                }
            }
        }
    }

    // ====== Active Glow Effect ======

    Rectangle {
        anchors.fill: parent
        anchors.margins: -2
        radius: parent.radius + 2
        visible: root.active
        color: "transparent"
        border.width: 2
        border.color: Services.Theme.colors.primaryGlow
        z: -1
    }
}
