// MoleculeTests.qml - Visual Tests for Molecule Components
//
// Tests all molecule components with various states and configurations.
//
// Run with: ./scripts/run.sh and navigate to this test component.

import QtQuick
import QtQuick.Layouts
import "../../src/molecules" as Molecules
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
            text: "Molecule Component Tests"
            fontSize: Theme.Theme.typography.size2xl
            fontWeight: Theme.Theme.typography.weightBold
            color: Theme.Theme.colors.textPrimary
        }

        // Clock Tests
        TestSection {
            title: "Clock Tests"

            Column {
                spacing: 16

                Row {
                    spacing: 32
                    Molecules.Clock { showTime: true; showDate: true }
                    Molecules.Clock { showTime: true; showDate: false }
                    Molecules.Clock { showTime: true; showDate: true; format24Hour: false }
                    Molecules.Clock { showTime: true; showDate: true; showSeconds: true }
                }
            }
        }

        // VolumeControl Tests
        TestSection {
            title: "VolumeControl Tests"

            Row {
                spacing: 32
                Molecules.VolumeControl { width: 200 }
                Molecules.VolumeControl { width: 200; orientation: Qt.Vertical; height: 150 }
            }
        }

        // BrightnessControl Tests
        TestSection {
            title: "BrightnessControl Tests"

            Molecules.BrightnessControl { width: 200 }
        }

        // NetworkIndicator Tests
        TestSection {
            title: "NetworkIndicator Tests"

            Row {
                spacing: 16
                Molecules.NetworkIndicator {}
            }
        }

        // BatteryIndicator Tests
        TestSection {
            title: "BatteryIndicator Tests"

            Row {
                spacing: 16
                Molecules.BatteryIndicator {}
            }
        }

        // WorkspaceIndicator Tests
        TestSection {
            title: "WorkspaceIndicator Tests"

            Molecules.WorkspaceIndicator {}
        }

        // MediaWidget Tests
        TestSection {
            title: "MediaWidget Tests"

            Molecules.MediaWidget { width: 280 }
        }

        // SearchInput Tests
        TestSection {
            title: "SearchInput Tests"

            Column {
                spacing: 8
                Molecules.SearchInput { width: 300; placeholder: "Search apps..." }
                Molecules.SearchInput { width: 300; placeholder: "Search files..."; showShortcut: true }
            }
        }

        // ListItem Tests
        TestSection {
            title: "ListItem Tests"

            Column {
                spacing: 4
                Molecules.ListItem { title: "Basic Item"; width: 300 }
                Molecules.ListItem { title: "With Subtitle"; subtitle: "Subtitle text"; width: 300 }
                Molecules.ListItem { title: "With Arrow"; showArrow: true; width: 300 }
                Molecules.ListItem { title: "Selected"; selected: true; width: 300 }
            }
        }

        // AppGridItem Tests
        TestSection {
            title: "AppGridItem Tests"

            Row {
                spacing: 16
                Molecules.AppGridItem { appName: "Files"; width: 80; height: 90 }
                Molecules.AppGridItem { appName: "Terminal"; width: 80; height: 90 }
                Molecules.AppGridItem { appName: "Browser"; width: 80; height: 90 }
            }
        }

        // NotificationItem Tests
        TestSection {
            title: "NotificationItem Tests"

            Column {
                spacing: 8
                Molecules.NotificationItem {
                    appName: "System"
                    title: "Update Available"
                    body: "A new version is ready to install"
                    width: 320
                }
            }
        }

        // PowerMenu Tests
        TestSection {
            title: "PowerMenu Tests"

            Molecules.PowerMenu {}
        }

        // UserMenu Tests
        TestSection {
            title: "UserMenu Tests"

            Molecules.UserMenu {}
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
