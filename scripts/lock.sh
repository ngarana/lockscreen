#!/usr/bin/env bash
# lock.sh - Production Lock Screen Launcher for Qypr
#
# Activates the lock screen in production mode with:
# - Auto-lock enabled
# - Daemon mode (-d) for background operation
#
# Usage:
#   ./lock.sh           # Lock the screen

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_PATH="$(dirname "$SCRIPT_DIR")"

export QUICKSHELL_LOCKSCREEN_AUTO_LOCK=1

echo "Starting Qypr lock screen..."
echo "Config: $CONFIG_PATH"
echo ""

exec qs -d -p "$CONFIG_PATH" "$@"
