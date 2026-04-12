// Typography.qml - Font and Text Style Definitions
//
// Provides typography tokens for consistent text styling across the shell.

import QtQuick

QtObject {
    id: root

    // ========================================================================
    // Font Family
    // ========================================================================

    readonly property string fontFamily: "Inter"
    readonly property string fontFamilyMono: "JetBrains Mono"

    // ========================================================================
    // Font Sizes
    // ========================================================================

    readonly property int sizeXs: 10
    readonly property int sizeSm: 12
    readonly property int sizeMd: 14
    readonly property int sizeLg: 16
    readonly property int sizeXl: 20
    readonly property int size2xl: 24
    readonly property int size3xl: 32
    readonly property int size4xl: 48
    readonly property int size5xl: 64
    readonly property int sizeClock: 96

    // ========================================================================
    // Font Weights
    // ========================================================================

    readonly property int weightLight: Font.Light
    readonly property int weightRegular: Font.Normal
    readonly property int weightNormal: Font.Normal
    readonly property int weightMedium: Font.Medium
    readonly property int weightSemiBold: Font.DemiBold
    readonly property int weightBold: Font.Bold

    // ========================================================================
    // Text Styles (predefined combinations)
    // ========================================================================

    // Heading styles
    readonly property var h1: QtObject {
        readonly property int size: root.size5xl
        readonly property int weight: root.weightBold
        readonly property real lineHeight: 1.1
    }

    readonly property var h2: QtObject {
        readonly property int size: root.size4xl
        readonly property int weight: root.weightBold
        readonly property real lineHeight: 1.2
    }

    readonly property var h3: QtObject {
        readonly property int size: root.size3xl
        readonly property int weight: root.weightSemiBold
        readonly property real lineHeight: 1.2
    }

    readonly property var h4: QtObject {
        readonly property int size: root.size2xl
        readonly property int weight: root.weightSemiBold
        readonly property real lineHeight: 1.3
    }

    // Body styles
    readonly property var bodyLg: QtObject {
        readonly property int size: root.sizeLg
        readonly property int weight: root.weightNormal
        readonly property real lineHeight: 1.6
    }

    readonly property var bodyMd: QtObject {
        readonly property int size: root.sizeMd
        readonly property int weight: root.weightNormal
        readonly property real lineHeight: 1.5
    }

    readonly property var bodySm: QtObject {
        readonly property int size: root.sizeSm
        readonly property int weight: root.weightNormal
        readonly property real lineHeight: 1.5
    }

    // Special styles
    readonly property var clock: QtObject {
        readonly property int size: root.sizeClock
        readonly property int weight: root.weightLight
        readonly property real lineHeight: 1.0
    }

    readonly property var caption: QtObject {
        readonly property int size: root.sizeXs
        readonly property int weight: root.weightMedium
        readonly property real lineHeight: 1.4
    }

    readonly property var button: QtObject {
        readonly property int size: root.sizeMd
        readonly property int weight: root.weightMedium
        readonly property real lineHeight: 1.0
    }
}
