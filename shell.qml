// shell.qml - Quickshell Lockscreen Entry Point
//
// Main configuration file for the lockscreen application.
// Sets up the session lock, UI surface, and IPC interface.
//
// Architecture Overview:
//   shell.qml (Entry Point)
//     ├── WlSessionLock (Wayland Session Lock)
//     │   └── WlSessionLockSurface (Per-screen UI)
//     │       └── LockScreen (Main UI)
//     ├── IpcHandler (External Control)
//     └── LockController (Auth Manager)
//
// Features:
// - Secure session locking via ext_session_lock_v1 protocol
// - Auto-lock support via QUICKSHELL_LOCKSCREEN_AUTO_LOCK env var
// - IPC interface for external control
// - Multi-monitor support (auto-creates surface per screen)
//
// Usage:
//   # Development mode (no auto-lock)
//   ./run.sh
//
//   # Lock mode (auto-lock enabled)
//   QUICKSHELL_LOCKSCREEN_AUTO_LOCK=1 quickshell -p shell.qml
//
//   # IPC control
//   quickshell -c lockscreen --ipc lock
//   quickshell -c lockscreen --ipc status
//
// Environment Variables:
//   QUICKSHELL_LOCKSCREEN_AUTO_LOCK=1 - Enable auto-lock on startup

import Quickshell
import Quickshell.Wayland
import Quickshell.Io
import QtQuick
import "src/services"
import "src/widgets"

ShellRoot {
    id: root

    // ========================================================================
    // Configuration
    // ========================================================================

    // Check for auto-lock environment variable
    // Set QUICKSHELL_LOCKSCREEN_AUTO_LOCK=1 to lock immediately on startup
    readonly property bool autoLock: Quickshell.env("QUICKSHELL_LOCKSCREEN_AUTO_LOCK") === "1"

    // ========================================================================
    // Session Lock (Wayland ext_session_lock_v1)
    // ========================================================================
    // Provides secure session locking at the compositor level.
    // When locked:
    // - All screens show lock surfaces
    // - No other windows can be shown
    // - If quickshell crashes, screen stays locked (solid color)

    WlSessionLock {
        id: sessionLock

        // Initial lock state based on auto-lock setting
        // Can be toggled via IPC or LockController
        locked: root.autoLock

        // ====================================================================
        // Lock Surface (Per-Screen UI)
        // ====================================================================
        // One WlSessionLockSurface is created per connected display.
        // Each surface displays the LockScreen widget.

        WlSessionLockSurface {
            id: lockSurface

            // Main lockscreen UI
            LockScreen {
                anchors.fill: parent

                // Handle unlock request from LockScreen
                onUnlockRequested: {
                    sessionLock.locked = false
                }
            }
        }
    }

    // ========================================================================
    // IPC Handler (External Control Interface)
    // ========================================================================
    // Allows controlling the lockscreen from command line or scripts.
    //
    // Usage:
    //   quickshell -c lockscreen --ipc lock
    //   quickshell -c lockscreen --ipc status

    IpcHandler {
        // Unique identifier for this lockscreen instance
        target: "lockscreen"

        // ----------------------------------------------------------------
        // Lock the session
        // Usage: quickshell -c lockscreen --ipc lock
        // ----------------------------------------------------------------
        function lock(): void {
            LockController.lock()
        }

        // ----------------------------------------------------------------
        // Unlock the session
        // Usage: quickshell -c lockscreen --ipc unlock
        // ----------------------------------------------------------------
        function unlock(): void {
            LockController.unlock()
        }

        // ----------------------------------------------------------------
        // Toggle lock state
        // Usage: quickshell -c lockscreen --ipc toggle
        // ----------------------------------------------------------------
        function toggle(): void {
            if (LockController.isLocked) {
                LockController.unlock()
            } else {
                LockController.lock()
            }
        }

        // ----------------------------------------------------------------
        // Get lock status
        // Usage: quickshell -c lockscreen --ipc status
        // Returns: "locked", "locking", or "unlocked"
        // ----------------------------------------------------------------
        function status(): string {
            if (!LockController.isLocked) {
                return "unlocked"
            }

            // "locking" means compositor hasn't confirmed all screens covered
            return LockController.isSecure ? "locked" : "locking"
        }
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        // Give LockController a reference to the session lock
        // This allows it to control lock/unlock state
        LockController.setLockInstance(sessionLock)
    }
}
