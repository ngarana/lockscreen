#!/usr/bin/env bash
# launcher.sh - Application Launcher Trigger for Qypr
#
# Toggles the application launcher visibility via IPC.
# If Qypr is running, sends IPC command to show/hide launcher.
# Otherwise, starts Qypr in full mode.
#
# Usage:
#   ./launcher.sh         # Toggle launcher
#   ./launcher.sh show    # Show launcher
#   ./launcher.sh hide    # Hide launcher

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_PATH="$(dirname "$SCRIPT_DIR")"

ACTION="${1:-toggle}"

# Check if Qypr is running
if qs ipc -p "$CONFIG_PATH" show >/dev/null 2>&1; then
    case "$ACTION" in
        show|hide)
            echo "Sending $ACTION command to launcher..."
            qs ipc -p "$CONFIG_PATH" call launcher "$ACTION" 2>/dev/null || {
                echo "Warning: Launcher module not available"
                exit 1
            }
            ;;
        toggle)
            echo "Toggling launcher..."
            qs ipc -p "$CONFIG_PATH" call launcher toggle 2>/dev/null || {
                echo "Warning: Launcher module not available"
                exit 1
            }
            ;;
        *)
            echo "Unknown action: $ACTION"
            echo "Usage: $0 [show|hide|toggle]"
            exit 1
            ;;
    esac
else
    echo "Qypr is not running. Starting in full mode..."
    export QUICKSHELL_AUTO_LOCK=0
    export QUICKSHELL_MODE="full"
    exec qs -p "$CONFIG_PATH" "$@"
fi
