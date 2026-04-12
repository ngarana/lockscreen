// shell.qml - Qypr Desktop Shell Entry Point
//
// Main configuration file for the Qypr modular desktop shell system.
// Uses Quickshell framework with ShellRoot as the root element.
//
// Architecture Overview:
//   shell.qml (Entry Point)
//     ├── Scope (State Persistence)
//     │   ├── IpcHandler (External Control Interface)
//     │   └── Logger configuration
//     ├── WlSessionLock (Wayland Session Lock)
//     │   └── Variants (Per-screen lock surfaces)
//     │       └── LockScreen (Lockscreen module)
//     └── Variants (Per-screen panels)
//         └── StatusBar (Status bar module - future)
//
// Features:
// - Secure session locking via ext_session_lock_v1 protocol
// - Auto-lock support via QUICKSHELL_LOCKSCREEN_AUTO_LOCK env var
// - IPC interface for external control
// - Multi-monitor support (auto-creates surfaces per screen)
// - Module enable/disable via QUICKSHELL_MODE env var
// - State persistence across hot-reloads via Scope
// - Structured logging with Logger singleton
//
// Usage:
//   # Development mode (no auto-lock)
//   ./run.sh
//
//   # Lock mode (auto-lock enabled)
//   QUICKSHELL_LOCKSCREEN_AUTO_LOCK=1 quickshell -p shell.qml
//
//   # IPC control
//   qs ipc -p . call lockscreen lock
//   qs ipc -p . call lockscreen status
//
// Environment Variables:
//   QUICKSHELL_LOCKSCREEN_AUTO_LOCK=1 - Enable auto-lock on startup
//   QUICKSHELL_MODE=full|bar|lock     - Select which modules to enable
//   QUICKSHELL_DEBUG=1                 - Enable debug logging

import Quickshell
import Quickshell.Wayland
import Quickshell.Io
import QtQuick

// Module imports (using relative paths for LSP support)
// Lockscreen module - provides LockScreen and LockController singleton
import "src/modules/lockscreen"
// Services - provides PowerManager and legacy Theme singleton
import "src/services"
// Core utilities - provides Logger singleton
import "src/core"

ShellRoot {
    id: root

    // ========================================================================
    // Configuration
    // ========================================================================

    // Auto-lock environment variable
    // Set QUICKSHELL_LOCKSCREEN_AUTO_LOCK=1 to lock immediately on startup
    readonly property bool autoLock: Quickshell.env("QUICKSHELL_AUTO_LOCK", "0") === "1"

    // Module selection environment variable
    // "full" = all modules (default)
    // "bar" = status bar only
    // "lock" = lockscreen only
    readonly property string mode: Quickshell.env("QUICKSHELL_MODE", "full")

    // Debug mode
    readonly property bool debugMode: Quickshell.env("QUICKSHELL_DEBUG", "0") === "1"

    // Module enable flags
    readonly property bool lockscreenEnabled: root.mode === "full" || root.mode === "lock"
    readonly property bool statusbarEnabled: root.mode === "full" || root.mode === "bar"

    // ========================================================================
    // State Persistence (survives hot-reloads)
    // ========================================================================
    // Scope prevents these objects from being destroyed and recreated on UI reloads.
    // Do NOT put visual components inside a Scope.

    Scope {
        // IPC Handler for external control
        IpcHandler {
            target: "qypr"

            // ----------------------------------------------------------------
            // Lock the session
            // Usage: qs ipc -p . call lockscreen lock
            // ----------------------------------------------------------------
            function lock(): void {
                LockController.lock()
            }

            // ----------------------------------------------------------------
            // Unlock the session
            // Usage: qs ipc -p . call lockscreen unlock
            // ----------------------------------------------------------------
            function unlock(): void {
                LockController.unlock()
            }

            // ----------------------------------------------------------------
            // Toggle lock state
            // Usage: qs ipc -p . call lockscreen toggle
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
            // Usage: qs ipc -p . call lockscreen status
            // Returns: "locked", "locking", or "unlocked"
            // ----------------------------------------------------------------
            function status(): string {
                if (!LockController.isLocked) {
                    return "unlocked"
                }
                return LockController.isSecure ? "locked" : "locking"
            }
        }

        // Configure logger based on debug mode
        // This runs once on startup and survives hot-reloads
        Component.onCompleted: {
            if (root.debugMode) {
                Logger.developmentMode()
            } else {
                Logger.productionMode()
            }
        }
    }

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

        // Lock surfaces across all screens using Variants
        // One WlSessionLockSurface is created per connected display
        Variants {
            model: root.lockscreenEnabled ? Quickshell.screens : []
            delegate: WlSessionLockSurface {
                // modelData is the screen from Quickshell.screens
                screen: modelData

                LockScreen {
                    anchors.fill: parent

                    // Handle unlock request from LockScreen
                    onUnlockRequested: {
                        sessionLock.locked = false
                    }
                }
            }
        }
    }

    // ========================================================================
    // Status Bar (Per-Screen)
    // ========================================================================
    // Status bar displayed on each screen using PanelWindow.
    // Uses Variants to spawn on all available screens.
    //
    // TODO: Uncomment when StatusBar module is implemented (Phase 4)

    // Variants {
    //     model: root.statusbarEnabled ? Quickshell.screens : []
    //     delegate: PanelWindow {
    //         screen: modelData
    //         // StatusBar {
    //         //     anchors.fill: parent
    //         // }
    //     }
    // }

    // ========================================================================
    // Initialization
    // ========================================================================

    Component.onCompleted: {
        // Give LockController a reference to the session lock
        // This allows it to control lock/unlock state
        LockController.setLockInstance(sessionLock)

        // Log startup information
        Logger.info("Qypr shell initialized", "shell.qml")
        Logger.info("Mode: " + root.mode, "shell.qml")
        Logger.info("Auto-lock: " + root.autoLock, "shell.qml")
        Logger.info("Debug: " + root.debugMode, "shell.qml")
        Logger.info("Screens: " + Quickshell.screens.length, "shell.qml")
    }
}
