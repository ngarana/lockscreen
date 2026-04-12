// AtomTests.qml - Visual Tests for Atomic Components
//
// Tests all atom components with various states and configurations.
//
// Run with: ./scripts/run.sh and navigate to this test component.

import QtQuick
import QtQuick.Layouts
import "../../src/atoms" as Atoms
import "../../src/theme" as Theme

Rectangle {
    id: root
    color: Theme.Theme.colors.base

    property var testResults: ({})

    Column {
        anchors.fill: parent
        anchors.margins: 32
        spacing: 32

        Atoms.Label {
            text: "Atom Component Tests"
            fontSize: Theme.Theme.typography.size2xl
            fontWeight: Theme.Theme.typography.weightBold
            color: Theme.Theme.colors.textPrimary
        }

        // Button Tests
        TestSection {
            title: "Button Tests"

            Column {
                spacing: 16

                Row {
                    spacing: 8
                    Atoms.Button { text: "Small"; size: "small" }
                    Atoms.Button { text: "Medium"; size: "medium" }
                    Atoms.Button { text: "Large"; size: "large" }
                }

                Row {
                    spacing: 8
                    Atoms.Button { text: "Filled"; variant: "filled" }
                    Atoms.Button { text: "Outlined"; variant: "outlined" }
                    Atoms.Button { text: "Ghost"; variant: "ghost" }
                }

                Row {
                    spacing: 8
                    Atoms.Button { text: "Enabled"; enabled: true }
                    Atoms.Button { text: "Disabled"; enabled: false }
                    Atoms.Button { text: "Loading"; loading: true }
                }
            }
        }

        // IconButton Tests
        TestSection {
            title: "IconButton Tests"

            Row {
                spacing: 8
                Atoms.IconButton { icon: "search"; size: "small" }
                Atoms.IconButton { icon: "settings"; size: "medium" }
                Atoms.IconButton { icon: "close"; size: "large" }
                Atoms.IconButton { icon: "menu"; rounded: true }
                Atoms.IconButton { icon: "heart"; enabled: false }
            }
        }

        // TextButton Tests
        TestSection {
            title: "TextButton Tests"

            Row {
                spacing: 16
                Atoms.TextButton { text: "Primary" }
                Atoms.TextButton { text: "Secondary"; variant: "secondary" }
                Atoms.TextButton { text: "Tertiary"; variant: "tertiary" }
                Atoms.TextButton { text: "Destructive"; variant: "destructive" }
            }
        }

        // Input Tests
        TestSection {
            title: "Input Tests"

            Column {
                spacing: 8

                Row {
                    spacing: 16
                    Atoms.Input { placeholder: "Default"; width: 180 }
                    Atoms.Input { placeholder: "Password"; passwordMode: true; width: 180 }
                    Atoms.Input { placeholder: "With clear"; showClear: true; text: "Text"; width: 180 }
                }
            }
        }

        // Slider Tests
        TestSection {
            title: "Slider Tests"

            Column {
                spacing: 8

                Atoms.Slider { width: 200; value: 0.0; showValue: true }
                Atoms.Slider { width: 200; value: 0.5; showValue: true }
                Atoms.Slider { width: 200; value: 1.0; showValue: true }
            }
        }

        // ProgressBar Tests
        TestSection {
            title: "ProgressBar Tests"

            Column {
                spacing: 8

                Atoms.ProgressBar { width: 200; value: 0.0 }
                Atoms.ProgressBar { width: 200; value: 0.5 }
                Atoms.ProgressBar { width: 200; value: 1.0 }
                Atoms.ProgressBar { width: 200; indeterminate: true }
            }
        }

        // Badge Tests
        TestSection {
            title: "Badge Tests"

            Row {
                spacing: 8
                Atoms.Badge { count: 0 }
                Atoms.Badge { count: 5 }
                Atoms.Badge { count: 99; variant: "success" }
                Atoms.Badge { count: 150; variant: "warning" }
                Atoms.Badge { variant: "error"; dotOnly: true }
            }
        }

        // Spinner Tests
        TestSection {
            title: "Spinner Tests"

            Row {
                spacing: 16
                Atoms.Spinner { size: 16 }
                Atoms.Spinner { size: 24 }
                Atoms.Spinner { size: 32; color: Theme.Theme.colors.blue }
            }
        }
    }

    component TestSection: Column {
        property string title

        spacing: 8

        Atoms.Label {
            text: parent.title
            fontSize: Theme.Theme.typography.sizeMd
            fontWeight: Theme.Theme.typography.weightSemiBold
            color: Theme.Theme.colors.textPrimary
        }

        Rectangle {
            width: parent.parent.width
            height: 1
            color: Theme.Theme.colors.surface1
        }
    }
}
