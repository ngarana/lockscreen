#!/usr/bin/env bash
# lock.sh - Production Lock Screen Launcher for Qypr
#
# Activates the lock screen in production mode with:
# - Auto-lock enabled
# - Daemon mode (-d) for background operation
# - IPC check to avoid duplicate instances
#
# Usage:
#   ./lock.sh           # Lock the screen
#
# If Qypr is already running, sends IPC command to lock.
# Otherwise, starts a new instance with auto-lock enabled.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_PATH="$(dirname "$SCRIPT_DIR")"

# Check if Qypr is already running via IPC
if qs ipc -p "$CONFIG_PATH" show >/dev/null 2>&1; then
    echo "Qypr is running. Sending lock command via IPC..."
    exec qs ipc -p "$CONFIG_PATH" call qypr lock
fi

# Start new instance with auto-lock enabled
export QUICKSHELL_LOCKSCREEN_AUTO_LOCK=1
export QUICKSHELL_MODE="lock"

echo "Starting Qypr lock screen..."
echo "Config: $CONFIG_PATH"
echo ""

exec qs -d -p "$CONFIG_PATH" "$@"
