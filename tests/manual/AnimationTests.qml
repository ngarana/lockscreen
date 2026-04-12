// AnimationTests.qml - Visual Tests for Animation Components
//
// Tests all animation components with various configurations.
//
// Run with: ./scripts/run.sh and navigate to this test component.

import QtQuick
import QtQuick.Layouts
import "../../src/animations" as Animations
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
            text: "Animation Component Tests"
            fontSize: Theme.Theme.typography.size2xl
            fontWeight: Theme.Theme.typography.weightBold
            color: Theme.Theme.colors.textPrimary
        }

        // FadeAnimation Tests
        TestSection {
            title: "FadeAnimation Tests"

            Column {
                spacing: 16

                Row {
                    spacing: 32

                    Column {
                        spacing: 8
                        Atoms.Label { text: "Fade In"; color: Theme.Theme.colors.textSecondary }

                        Rectangle {
                            id: fadeInTarget
                            width: 100
                            height: 100
                            color: Theme.Theme.colors.blue
                            radius: Theme.Theme.radius.medium
                            opacity: 0

                            Atoms.Label {
                                anchors.centerIn: parent
                                text: "Faded In"
                                color: Theme.Theme.colors.crust
                            }

                            Component.onCompleted: opacity = 0
                        }
                    }

                    Column {
                        spacing: 8
                        Atoms.Label { text: "Fade Out"; color: Theme.Theme.colors.textSecondary }

                        Rectangle {
                            id: fadeOutTarget
                            width: 100
                            height: 100
                            color: Theme.Theme.colors.red
                            radius: Theme.Theme.radius.medium
                            opacity: 1

                            Atoms.Label {
                                anchors.centerIn: parent
                                text: "Faded Out"
                                color: Theme.Theme.colors.crust
                            }
                        }
                    }
                }

                Row {
                    spacing: 16
                    Atoms.Button {
                        text: "Fade In"
                        onClicked: {
                            fadeInAnim.start()
                        }
                    }
                    Atoms.Button {
                        text: "Fade Out"
                        onClicked: {
                            fadeOutAnim.start()
                        }
                    }
                }
            }
        }

        Animations.FadeAnimation {
            id: fadeInAnim
            target: fadeInTarget
            fadeIn: true
            fromOpacity: 0
            toOpacity: 1
        }

        Animations.FadeAnimation {
            id: fadeOutAnim
            target: fadeOutTarget
            fadeIn: false
            fadeOut: true
            fromOpacity: 1
            toOpacity: 0
        }

        // SlideAnimation Tests
        TestSection {
            title: "SlideAnimation Tests"

            Column {
                spacing: 16

                Row {
                    spacing: 32

                    Column {
                        spacing: 8
                        Atoms.Label { text: "Slide Left"; color: Theme.Theme.colors.textSecondary }

                        Rectangle {
                            id: slideLeftTarget
                            width: 100
                            height: 100
                            color: Theme.Theme.colors.green
                            radius: Theme.Theme.radius.medium
                            x: 50

                            Atoms.Label {
                                anchors.centerIn: parent
                                text: "Slide"
                                color: Theme.Theme.colors.crust
                            }
                        }
                    }

                    Column {
                        spacing: 8
                        Atoms.Label { text: "Slide Right"; color: Theme.Theme.colors.textSecondary }

                        Rectangle {
                            id: slideRightTarget
                            width: 100
                            height: 100
                            color: Theme.Theme.colors.mauve
                            radius: Theme.Theme.radius.medium
                            x: 50

                            Atoms.Label {
                                anchors.centerIn: parent
                                text: "Slide"
                                color: Theme.Theme.colors.crust
                            }
                        }
                    }

                    Column {
                        spacing: 8
                        Atoms.Label { text: "Slide Up"; color: Theme.Theme.colors.textSecondary }

                        Rectangle {
                            id: slideUpTarget
                            width: 100
                            height: 100
                            color: Theme.Theme.colors.yellow
                            radius: Theme.Theme.radius.medium
                            y: 50

                            Atoms.Label {
                                anchors.centerIn: parent
                                text: "Slide"
                                color: Theme.Theme.colors.crust
                            }
                        }
                    }

                    Column {
                        spacing: 8
                        Atoms.Label { text: "Slide Down"; color: Theme.Theme.colors.textSecondary }

                        Rectangle {
                            id: slideDownTarget
                            width: 100
                            height: 100
                            color: Theme.Theme.colors.pink
                            radius: Theme.Theme.radius.medium
                            y: 50

                            Atoms.Label {
                                anchors.centerIn: parent
                                text: "Slide"
                                color: Theme.Theme.colors.crust
                            }
                        }
                    }
                }

                Row {
                    spacing: 8
                    Atoms.Button {
                        text: "Slide Left"
                        onClicked: slideLeftAnim.start()
                    }
                    Atoms.Button {
                        text: "Slide Right"
                        onClicked: slideRightAnim.start()
                    }
                    Atoms.Button {
                        text: "Slide Up"
                        onClicked: slideUpAnim.start()
                    }
                    Atoms.Button {
                        text: "Slide Down"
                        onClicked: slideDownAnim.start()
                    }
                    Atoms.Button {
                        text: "Reset"
                        onClicked: {
                            slideLeftTarget.x = 50
                            slideRightTarget.x = 50
                            slideUpTarget.y = 50
                            slideDownTarget.y = 50
                        }
                    }
                }
            }
        }

        Animations.SlideAnimation {
            id: slideLeftAnim
            target: slideLeftTarget
            direction: "left"
            slideIn: false
            distance: 100
        }

        Animations.SlideAnimation {
            id: slideRightAnim
            target: slideRightTarget
            direction: "right"
            slideIn: false
            distance: 100
        }

        Animations.SlideAnimation {
            id: slideUpAnim
            target: slideUpTarget
            direction: "up"
            slideIn: false
            distance: 100
        }

        Animations.SlideAnimation {
            id: slideDownAnim
            target: slideDownTarget
            direction: "down"
            slideIn: false
            distance: 100
        }

        // ScaleAnimation Tests
        TestSection {
            title: "ScaleAnimation Tests"

            Column {
                spacing: 16

                Row {
                    spacing: 32

                    Column {
                        spacing: 8
                        Atoms.Label { text: "Scale In"; color: Theme.Theme.colors.textSecondary }

                        Rectangle {
                            id: scaleInTarget
                            width: 100
                            height: 100
                            color: Theme.Theme.colors.teal
                            radius: Theme.Theme.radius.medium
                            scale: 0.5

                            Atoms.Label {
                                anchors.centerIn: parent
                                text: "Scale"
                                color: Theme.Theme.colors.crust
                            }
                        }
                    }

                    Column {
                        spacing: 8
                        Atoms.Label { text: "Scale Out"; color: Theme.Theme.colors.textSecondary }

                        Rectangle {
                            id: scaleOutTarget
                            width: 100
                            height: 100
                            color: Theme.Theme.colors.peach
                            radius: Theme.Theme.radius.medium
                            scale: 1.0

                            Atoms.Label {
                                anchors.centerIn: parent
                                text: "Scale"
                                color: Theme.Theme.colors.crust
                            }
                        }
                    }

                    Column {
                        spacing: 8
                        Atoms.Label { text: "Pop Effect"; color: Theme.Theme.colors.textSecondary }

                        Rectangle {
                            id: popTarget
                            width: 100
                            height: 100
                            color: Theme.Theme.colors.sapphire
                            radius: Theme.Theme.radius.medium
                            scale: 1.0

                            Atoms.Label {
                                anchors.centerIn: parent
                                text: "Pop!"
                                color: Theme.Theme.colors.crust
                            }
                        }
                    }
                }

                Row {
                    spacing: 16
                    Atoms.Button {
                        text: "Scale In"
                        onClicked: scaleInAnim.start()
                    }
                    Atoms.Button {
                        text: "Scale Out"
                        onClicked: scaleOutAnim.start()
                    }
                    Atoms.Button {
                        text: "Pop"
                        onClicked: popAnim.start()
                    }
                    Atoms.Button {
                        text: "Reset"
                        onClicked: {
                            scaleInTarget.scale = 0.5
                            scaleOutTarget.scale = 1.0
                            popTarget.scale = 1.0
                        }
                    }
                }
            }
        }

        Animations.ScaleAnimation {
            id: scaleInAnim
            target: scaleInTarget
            scaleFrom: 0.5
            scaleTo: 1.0
            scaleIn: true
        }

        Animations.ScaleAnimation {
            id: scaleOutAnim
            target: scaleOutTarget
            scaleFrom: 1.0
            scaleTo: 0.5
            scaleIn: false
            scaleOut: true
        }

        Animations.ScaleAnimation {
            id: popAnim
            target: popTarget
            scaleFrom: 1.0
            scaleTo: 1.2
            duration: Theme.Theme.animation.fast
            scaleIn: true
        }

        // SpringAnimation Tests
        TestSection {
            title: "SpringAnimation Tests"

            Column {
                spacing: 16

                Row {
                    spacing: 32

                    Column {
                        spacing: 8
                        Atoms.Label { text: "Spring Y"; color: Theme.Theme.colors.textSecondary }

                        Rectangle {
                            id: springTarget
                            width: 100
                            height: 100
                            color: Theme.Theme.colors.lavender
                            radius: Theme.Theme.radius.medium
                            y: 0

                            Atoms.Label {
                                anchors.centerIn: parent
                                text: "Spring"
                                color: Theme.Theme.colors.crust
                            }
                        }
                    }

                    Column {
                        spacing: 8
                        Atoms.Label { text: "Spring X"; color: Theme.Theme.colors.textSecondary }

                        Rectangle {
                            id: springXTarget
                            width: 100
                            height: 100
                            color: Theme.Theme.colors.flamingo
                            radius: Theme.Theme.radius.medium
                            x: 0

                            Atoms.Label {
                                anchors.centerIn: parent
                                text: "Spring"
                                color: Theme.Theme.colors.crust
                            }
                        }
                    }
                }

                Row {
                    spacing: 16
                    Atoms.Button {
                        text: "Spring Y"
                        onClicked: {
                            springYAnim.from = 100
                            springYAnim.to = 0
                            springYAnim.start()
                        }
                    }
                    Atoms.Button {
                        text: "Spring X"
                        onClicked: {
                            springXAnim.from = 100
                            springXAnim.to = 0
                            springXAnim.start()
                        }
                    }
                    Atoms.Button {
                        text: "Reset"
                        onClicked: {
                            springTarget.y = 100
                            springXTarget.x = 100
                        }
                    }
                }

                Atoms.Label {
                    text: "spring: 1.5, damping: 0.15, mass: 1.0"
                    color: Theme.Theme.colors.textMuted
                }
            }
        }

        Animations.SpringAnimation {
            id: springYAnim
            target: springTarget
            property: "y"
            from: 100
            to: 0
            spring: 1.5
            damping: 0.15
        }

        Animations.SpringAnimation {
            id: springXAnim
            target: springXTarget
            property: "x"
            from: 100
            to: 0
            spring: 1.5
            damping: 0.15
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
