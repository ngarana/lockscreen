// ComponentShowcase.qml - Visual Component Testing and Showcase
//
// A comprehensive visual test page that displays all atomic components,
// molecules, layouts, and animations for visual verification.
//
// Run with: ./scripts/run.sh --mode full
// Navigate to ComponentShowcase via a test entry point.
//
// Sections:
// - Atoms: Button, IconButton, TextButton, Icon, Label, Input, Slider, etc.
// - Molecules: Clock, VolumeControl, BrightnessControl, etc.
// - Layouts: PanelLayout, PopupLayout, DrawerLayout, GridLayout, LayerLayout
// - Animations: Fade, Slide, Scale, Spring

import QtQuick
import QtQuick.Layouts
import "../atoms" as Atoms
import "../molecules" as Molecules
import "../layouts" as Layouts
import "../animations" as Animations
import "../theme" as Theme

Rectangle {
    id: root
    color: Theme.Theme.colors.base

    width: 1200
    height: 800

    Flickable {
        id: flickable
        anchors.fill: parent
        contentWidth: contentColumn.width
        contentHeight: contentColumn.height
        clip: true

        ScrollBar.vertical: Atoms.ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        Column {
            id: contentColumn
            spacing: 32
            padding: 32

            // Header
            Column {
                spacing: 8
                Atoms.Label {
                    text: "Qypr Component Showcase"
                    fontSize: Theme.Theme.typography.size3xl
                    fontWeight: Theme.Theme.typography.weightBold
                    color: Theme.Theme.colors.textPrimary
                }
                Atoms.Label {
                    text: "Visual testing and component documentation"
                    fontSize: Theme.Theme.typography.sizeMd
                    color: Theme.Theme.colors.textSecondary
                }
            }

            // ========== ATOMS SECTION ==========
            SectionHeader { text: "Atoms" }

            // Buttons
            SubSectionHeader { text: "Button" }
            Row {
                spacing: 16
                Atoms.Button { text: "Filled"; size: "small" }
                Atoms.Button { text: "Filled"; size: "medium" }
                Atoms.Button { text: "Filled"; size: "large" }
                Atoms.Button { text: "Outlined"; variant: "outlined" }
                Atoms.Button { text: "Ghost"; variant: "ghost" }
                Atoms.Button { text: "Disabled"; enabled: false }
                Atoms.Button { text: "Loading"; loading: true }
            }

            // IconButtons
            SubSectionHeader { text: "IconButton" }
            Row {
                spacing: 16
                Atoms.IconButton { icon: "search"; size: "small" }
                Atoms.IconButton { icon: "settings"; size: "medium" }
                Atoms.IconButton { icon: "close"; size: "large" }
                Atoms.IconButton { icon: "menu"; rounded: true }
            }

            // TextButtons
            SubSectionHeader { text: "TextButton" }
            Row {
                spacing: 16
                Atoms.TextButton { text: "Primary" }
                Atoms.TextButton { text: "Secondary"; variant: "secondary" }
                Atoms.TextButton { text: "Tertiary"; variant: "tertiary" }
                Atoms.TextButton { text: "Delete"; variant: "destructive" }
            }

            // Icons
            SubSectionHeader { text: "Icon" }
            Row {
                spacing: 16
                Atoms.Icon { name: "search"; size: 16; color: Theme.Theme.colors.textPrimary }
                Atoms.Icon { name: "settings"; size: 24; color: Theme.Theme.colors.blue }
                Atoms.Icon { name: "heart"; size: 32; color: Theme.Theme.colors.red }
                Atoms.Icon { name: "star"; size: 48; color: Theme.Theme.colors.yellow }
            }

            // Labels
            SubSectionHeader { text: "Label" }
            Column {
                spacing: 8
                Atoms.Label { text: "Heading 1"; variant: "h1" }
                Atoms.Label { text: "Heading 2"; variant: "h2" }
                Atoms.Label { text: "Heading 3"; variant: "h3" }
                Atoms.Label { text: "Body text"; variant: "body" }
                Atoms.Label { text: "Caption text"; variant: "caption" }
            }

            // Inputs
            SubSectionHeader { text: "Input" }
            Row {
                spacing: 16
                Atoms.Input { placeholder: "Default input"; width: 200 }
                Atoms.Input { placeholder: "Password"; passwordMode: true; width: 200 }
                Atoms.Input { placeholder: "With clear"; showClear: true; text: "Clear me"; width: 200 }
            }

            // Sliders
            SubSectionHeader { text: "Slider" }
            Row {
                spacing: 32
                Atoms.Slider { width: 200; value: 0.5; showValue: true }
                Atoms.Slider { width: 200; value: 0.75; orientation: Qt.Horizontal }
            }

            // ProgressBars
            SubSectionHeader { text: "ProgressBar" }
            Column {
                spacing: 8
                Atoms.ProgressBar { width: 300; value: 0.3 }
                Atoms.ProgressBar { width: 300; value: 0.6 }
                Atoms.ProgressBar { width: 300; indeterminate: true }
            }

            // Cards
            SubSectionHeader { text: "Card" }
            Row {
                spacing: 16
                Atoms.Card { width: 150; height: 100 }
                Atoms.Card { width: 150; height: 100; glassmorphic: true }
                Atoms.Card { width: 150; height: 100; hoverable: true }
            }

            // Badges
            SubSectionHeader { text: "Badge" }
            Row {
                spacing: 16
                Atoms.Badge { count: 5 }
                Atoms.Badge { count: 99; variant: "success" }
                Atoms.Badge { count: 150; variant: "warning" }
                Atoms.Badge { variant: "error"; dotOnly: true }
            }

            // Spinners
            SubSectionHeader { text: "Spinner" }
            Row {
                spacing: 16
                Atoms.Spinner { size: 16 }
                Atoms.Spinner { size: 24 }
                Atoms.Spinner { size: 32; color: Theme.Theme.colors.blue }
            }

            // Dividers
            SubSectionHeader { text: "Divider" }
            Column {
                spacing: 16
                width: 300
                Atoms.Divider { width: parent.width }
                Atoms.Divider { width: parent.width; label: "Section" }
            }

            // Tooltips
            SubSectionHeader { text: "Tooltip" }
            Row {
                spacing: 16
                Atoms.Button {
                    text: "Hover me"
                    Atoms.Tooltip {
                        visible: parent.hovered
                        text: "This is a tooltip"
                    }
                }
            }

            // ========== MOLECULES SECTION ==========
            SectionHeader { text: "Molecules" }

            // Clock
            SubSectionHeader { text: "Clock" }
            Row {
                spacing: 32
                Molecules.Clock { showTime: true; showDate: true }
                Molecules.Clock { showTime: true; showDate: false; format24Hour: false }
            }

            // VolumeControl
            SubSectionHeader { text: "VolumeControl" }
            Molecules.VolumeControl { width: 200 }

            // BrightnessControl
            SubSectionHeader { text: "BrightnessControl" }
            Molecules.BrightnessControl { width: 200 }

            // BatteryIndicator
            SubSectionHeader { text: "BatteryIndicator" }
            Row {
                spacing: 16
                Molecules.BatteryIndicator {}
            }

            // NetworkIndicator
            SubSectionHeader { text: "NetworkIndicator" }
            Row {
                spacing: 16
                Molecules.NetworkIndicator {}
            }

            // WorkspaceIndicator
            SubSectionHeader { text: "WorkspaceIndicator" }
            Molecules.WorkspaceIndicator {}

            // MediaWidget
            SubSectionHeader { text: "MediaWidget" }
            Molecules.MediaWidget { width: 280 }

            // SearchInput
            SubSectionHeader { text: "SearchInput" }
            Molecules.SearchInput { width: 300; placeholder: "Search apps..." }

            // ListItem
            SubSectionHeader { text: "ListItem" }
            Column {
                spacing: 4
                Molecules.ListItem { title: "List Item 1"; subtitle: "Subtitle here"; width: 300 }
                Molecules.ListItem { title: "List Item 2"; subtitle: "With arrow"; showArrow: true; width: 300 }
            }

            // AppGridItem
            SubSectionHeader { text: "AppGridItem" }
            Molecules.AppGridItem { appName: "Files"; width: 80; height: 90 }

            // PowerMenu
            SubSectionHeader { text: "PowerMenu" }
            Molecules.PowerMenu { visible: true }

            // UserMenu
            SubSectionHeader { text: "UserMenu" }
            Molecules.UserMenu {}

            // ========== LAYOUTS SECTION ==========
            SectionHeader { text: "Layouts" }

            // PanelLayout demo
            SubSectionHeader { text: "PanelLayout" }
            Rectangle {
                width: 400
                height: 40
                color: Theme.Theme.colors.surface0
                radius: Theme.Theme.radius.medium

                Layouts.PanelLayout {
                    anchors.fill: parent
                    leftMargin: 8
                    rightMargin: 8
                    Atoms.Label { text: "Left" }
                    Atoms.Label { text: "Right"; Layouts.PanelLayout.alignment: "right" }
                }
            }

            // GridLayout demo
            SubSectionHeader { text: "GridLayout" }
            Rectangle {
                width: 320
                height: 160
                color: Theme.Theme.colors.surface0
                radius: Theme.Theme.radius.medium
                clip: true

                Layouts.GridLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    columns: 4
                    spacing: 8

                    Repeater {
                        model: 8
                        Rectangle {
                            color: Theme.Theme.colors.blue
                            radius: Theme.Theme.radius.small
                            opacity: 0.5 + (index * 0.05)
                        }
                    }
                }
            }

            // ========== ANIMATIONS SECTION ==========
            SectionHeader { text: "Animations" }

            SubSectionHeader { text: "Animation Preview" }
            Row {
                spacing: 32

                // Fade demo
                Column {
                    spacing: 8
                    Atoms.Label { text: "Fade" }
                    Rectangle {
                        id: fadeTarget
                        width: 80
                        height: 80
                        color: Theme.Theme.colors.blue
                        radius: Theme.Theme.radius.medium
                        opacity: 0
                        visible: true

                        Atoms.Label {
                            anchors.centerIn: parent
                            text: "Faded"
                            color: Theme.Theme.colors.crust
                        }

                        Component.onCompleted: {
                            fadeAnim.start()
                        }

                        Animations.FadeAnimation {
                            id: fadeAnim
                            target: fadeTarget
                            fadeIn: true
                        }
                    }
                }

                // Scale demo
                Column {
                    spacing: 8
                    Atoms.Label { text: "Scale" }
                    Rectangle {
                        id: scaleTarget
                        width: 80
                        height: 80
                        color: Theme.Theme.colors.green
                        radius: Theme.Theme.radius.medium
                        scale: 0.8

                        Atoms.Label {
                            anchors.centerIn: parent
                            text: "Scaled"
                            color: Theme.Theme.colors.crust
                        }

                        Component.onCompleted: {
                            scaleAnim.start()
                        }

                        Animations.ScaleAnimation {
                            id: scaleAnim
                            target: scaleTarget
                            scaleIn: true
                        }
                    }
                }

                // Spring demo
                Column {
                    spacing: 8
                    Atoms.Label { text: "Spring" }
                    Rectangle {
                        id: springTarget
                        width: 80
                        height: 80
                        color: Theme.Theme.colors.mauve
                        radius: Theme.Theme.radius.medium
                        y: 20

                        Atoms.Label {
                            anchors.centerIn: parent
                            text: "Spring"
                            color: Theme.Theme.colors.crust
                        }

                        Component.onCompleted: {
                            springAnim.start()
                        }

                        Animations.SpringAnimation {
                            id: springAnim
                            target: springTarget
                            property: "y"
                            from: 20
                            to: 0
                            spring: 2.0
                            damping: 0.2
                        }
                    }
                }
            }
        }
    }

    // Section header component
    component SectionHeader: Atoms.Label {
        fontSize: Theme.Theme.typography.size2xl
        fontWeight: Theme.Theme.typography.weightBold
        color: Theme.Theme.colors.textPrimary
        topPadding: 16
    }

    // Subsection header component
    component SubSectionHeader: Atoms.Label {
        fontSize: Theme.Theme.typography.sizeMd
        fontWeight: Theme.Theme.typography.weightMedium
        color: Theme.Theme.colors.textSecondary
        topPadding: 8
    }
}
