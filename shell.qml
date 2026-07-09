// shell.qml - Qypr Desktop Shell Entry Point

import Quickshell
import Quickshell.Wayland
import Quickshell.Io
import Quickshell.Hyprland
import QtQuick
import QtQuick.Layouts

import "src/modules/lockscreen"
import "src/modules/statusbar"
import "src/modules/osd"
import "src/services"
import "src/core"

ShellRoot {
    id: root

    readonly property bool autoLock: Quickshell.env("QUICKSHELL_LOCKSCREEN_AUTO_LOCK") === "1"
    readonly property string mode: Quickshell.env("QUICKSHELL_MODE") || "full"
    readonly property bool debugMode: Quickshell.env("QUICKSHELL_DEBUG") === "1"
    readonly property bool lockscreenEnabled: root.mode === "full" || root.mode === "lock"
    readonly property bool statusbarEnabled: root.mode === "full" || root.mode === "bar"

    Scope {
        Component.onCompleted: {
            Logger.developmentMode()
        }
    }

    Loader {
        active: root.statusbarEnabled
        sourceComponent: Variants {
            model: Quickshell.screens

            delegate: Item {
                required property var modelData

                PanelWindow {
                    screen: modelData

                    anchors {
                        top: true
                        left: true
                        right: true
                    }

                    implicitHeight: 70

                    StatusBar {
                        anchors.fill: parent
                    }
                }
            }
        }
    }

    WlSessionLock {
        id: sessionLock
        locked: root.autoLock

        Component.onCompleted: LockController.setLockInstance(sessionLock)

        WlSessionLockSurface {
            color: "transparent"

            LockScreen {
                anchors.fill: parent
                onUnlockRequested: sessionLock.locked = false
            }
        }
    }

    Component.onCompleted: {
        Logger.info("Qypr shell initialized", "shell.qml")
        Logger.info("Mode: " + root.mode, "shell.qml")
        Logger.info("Screens: " + Quickshell.screens.length, "shell.qml")
    }
}