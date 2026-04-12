// LayoutTests.qml - Visual Tests for Layout Components
//
// Tests all layout components with various configurations.
//
// Run with: ./scripts/run.sh and navigate to this test component.

import QtQuick
import QtQuick.Layouts
import "../../src/layouts" as Layouts
import "../../src/atoms" as Atoms
import "../../src/theme" as Theme

Rectangle {
    id: root
    color: Theme.Theme.colors.base

    Column {
        anchors.fill: parent
        anchors.margins: 32
        spacing: 32

        Atoms.Label {
            text: "Layout Component Tests"
            fontSize: Theme.Theme.typography.size2xl
            fontWeight: Theme.Theme.typography.weightBold
            color: Theme.Theme.colors.textPrimary
        }

        // PanelLayout Tests
        TestSection {
            title: "PanelLayout Tests"

            Column {
                spacing: 16

                Atoms.Label { text: "Left aligned:"; color: Theme.Theme.colors.textSecondary }
                Rectangle {
                    width: 400
                    height: 40
                    color: Theme.Theme.colors.surface0
                    radius: Theme.Theme.radius.medium

                    Layouts.PanelLayout {
                        anchors.fill: parent
                        leftMargin: 8
                        rightMargin: 8
                        alignment: "left"

                        Atoms.Label { text: "Left" }
                    }
                }

                Atoms.Label { text: "Center aligned:"; color: Theme.Theme.colors.textSecondary }
                Rectangle {
                    width: 400
                    height: 40
                    color: Theme.Theme.colors.surface0
                    radius: Theme.Theme.radius.medium

                    Layouts.PanelLayout {
                        anchors.fill: parent
                        leftMargin: 8
                        rightMargin: 8
                        alignment: "center"

                        Atoms.Label { text: "Center" }
                    }
                }

                Atoms.Label { text: "Right aligned:"; color: Theme.Theme.colors.textSecondary }
                Rectangle {
                    width: 400
                    height: 40
                    color: Theme.Theme.colors.surface0
                    radius: Theme.Theme.radius.medium

                    Layouts.PanelLayout {
                        anchors.fill: parent
                        leftMargin: 8
                        rightMargin: 8
                        alignment: "right"

                        Atoms.Label { text: "Right" }
                    }
                }

                Atoms.Label { text: "Multiple items:"; color: Theme.Theme.colors.textSecondary }
                Rectangle {
                    width: 400
                    height: 40
                    color: Theme.Theme.colors.surface0
                    radius: Theme.Theme.radius.medium

                    Layouts.PanelLayout {
                        anchors.fill: parent
                        leftMargin: 8
                        rightMargin: 8

                        Atoms.Label { text: "Item 1" }
                        Atoms.Label { text: "Item 2" }
                        Atoms.Label { text: "Item 3"; Layouts.PanelLayout.alignment: "right" }
                    }
                }
            }
        }

        // GridLayout Tests
        TestSection {
            title: "GridLayout Tests"

            Column {
                spacing: 16

                Atoms.Label { text: "4 columns, 8 items:"; color: Theme.Theme.colors.textSecondary }
                Rectangle {
                    width: 340
                    height: 180
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
                                opacity: 0.3 + (index * 0.08)

                                Atoms.Label {
                                    anchors.centerIn: parent
                                    text: index + 1
                                    color: Theme.Theme.colors.crust
                                }
                            }
                        }
                    }
                }

                Atoms.Label { text: "3 columns, 6 items:"; color: Theme.Theme.colors.textSecondary }
                Rectangle {
                    width: 260
                    height: 180
                    color: Theme.Theme.colors.surface0
                    radius: Theme.Theme.radius.medium
                    clip: true

                    Layouts.GridLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        columns: 3
                        spacing: 8

                        Repeater {
                            model: 6
                            Rectangle {
                                color: Theme.Theme.colors.green
                                radius: Theme.Theme.radius.small
                                opacity: 0.3 + (index * 0.1)

                                Atoms.Label {
                                    anchors.centerIn: parent
                                    text: index + 1
                                    color: Theme.Theme.colors.crust
                                }
                            }
                        }
                    }
                }
            }
        }

        // PopupLayout Tests
        TestSection {
            title: "PopupLayout Tests"

            Column {
                spacing: 16

                Row {
                    spacing: 16

                    Atoms.Button {
                        text: "Show Modal Popup"
                        onClicked: modalPopup.visible = true
                    }

                    Atoms.Button {
                        text: "Show Non-Modal Popup"
                        onClicked: nonModalPopup.visible = true
                    }
                }

                Atoms.Label { text: "Click button to see popup"; color: Theme.Theme.colors.textSecondary }
            }
        }

        // DrawerLayout Tests
        TestSection {
            title: "DrawerLayout Tests"

            Column {
                spacing: 16

                Row {
                    spacing: 8

                    Atoms.Button { text: "Left Drawer"; onClicked: leftDrawer.visible = true }
                    Atoms.Button { text: "Right Drawer"; onClicked: rightDrawer.visible = true }
                    Atoms.Button { text: "Top Drawer"; onClicked: topDrawer.visible = true }
                    Atoms.Button { text: "Bottom Drawer"; onClicked: bottomDrawer.visible = true }
                }

                Atoms.Label { text: "Click button to see drawer"; color: Theme.Theme.colors.textSecondary }
            }
        }

        // LayerLayout Tests
        TestSection {
            title: "LayerLayout Tests"

            Column {
                spacing: 16

                Atoms.Label { text: "Layers: background(0) < content(10) < overlay(20) < modal(30) < tooltip(40) < top(50)"; color: Theme.Theme.colors.textSecondary }

                Layouts.LayerLayout {
                    width: 300
                    height: 150

                    Rectangle {
                        anchors.fill: parent
                        color: Theme.Theme.colors.surface1
                        radius: Theme.Theme.radius.medium

                        Atoms.Label {
                            anchors.centerIn: parent
                            text: "LayerLayout demo"
                        }
                    }
                }
            }
        }
    }

    // Modal popup
    Layouts.PopupLayout {
        id: modalPopup
        anchors.fill: parent

        Rectangle {
            width: 300
            height: 200
            color: Theme.Theme.colors.surface0
            radius: Theme.Theme.radius.large
            border.color: Theme.Theme.colors.surface1
            border.width: 1

            Column {
                anchors.centerIn: parent
                spacing: 16

                Atoms.Label {
                    text: "Modal Popup"
                    fontSize: Theme.Theme.typography.sizeLg
                    fontWeight: Theme.Theme.typography.weightBold
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Atoms.Label {
                    text: "Click backdrop to close"
                    color: Theme.Theme.colors.textSecondary
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Atoms.Button {
                    text: "Close"
                    onClicked: modalPopup.visible = false
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    // Non-modal popup
    Layouts.PopupLayout {
        id: nonModalPopup
        anchors.fill: parent
        modal: false

        Rectangle {
            width: 250
            height: 150
            color: Theme.Theme.colors.surface0
            radius: Theme.Theme.radius.large

            Column {
                anchors.centerIn: parent
                spacing: 16

                Atoms.Label {
                    text: "Non-Modal Popup"
                    fontSize: Theme.Theme.typography.sizeLg
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Atoms.Button {
                    text: "Close"
                    onClicked: nonModalPopup.visible = false
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    // Left drawer
    Layouts.DrawerLayout {
        id: leftDrawer
        anchors.fill: parent
        position: "left"
        drawerWidth: 250

        Column {
            anchors.fill: parent
            spacing: 16

            Atoms.Label {
                text: "Left Drawer"
                fontSize: Theme.Theme.typography.sizeLg
                fontWeight: Theme.Theme.typography.weightBold
            }

            Atoms.Button {
                text: "Close"
                onClicked: leftDrawer.visible = false
            }
        }
    }

    // Right drawer
    Layouts.DrawerLayout {
        id: rightDrawer
        anchors.fill: parent
        position: "right"
        drawerWidth: 250

        Column {
            anchors.fill: parent
            spacing: 16

            Atoms.Label {
                text: "Right Drawer"
                fontSize: Theme.Theme.typography.sizeLg
                fontWeight: Theme.Theme.typography.weightBold
            }

            Atoms.Button {
                text: "Close"
                onClicked: rightDrawer.visible = false
            }
        }
    }

    // Top drawer
    Layouts.DrawerLayout {
        id: topDrawer
        anchors.fill: parent
        position: "top"
        drawerHeight: 150

        Column {
            anchors.fill: parent
            spacing: 16

            Atoms.Label {
                text: "Top Drawer"
                fontSize: Theme.Theme.typography.sizeLg
                fontWeight: Theme.Theme.typography.weightBold
            }

            Atoms.Button {
                text: "Close"
                onClicked: topDrawer.visible = false
            }
        }
    }

    // Bottom drawer
    Layouts.DrawerLayout {
        id: bottomDrawer
        anchors.fill: parent
        position: "bottom"
        drawerHeight: 150

        Column {
            anchors.fill: parent
            spacing: 16

            Atoms.Label {
                text: "Bottom Drawer"
                fontSize: Theme.Theme.typography.sizeLg
                fontWeight: Theme.Theme.typography.weightBold
            }

            Atoms.Button {
                text: "Close"
                onClicked: bottomDrawer.visible = false
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
            width: parent.parent.width - 64
            height: 1
            color: Theme.Theme.colors.surface1
        }
    }
}
