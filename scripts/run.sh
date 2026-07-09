#!/usr/bin/env bash
# run.sh - Development Locker Launcher for Qypr
#
# Starts the locker in development mode with auto-lock disabled.
#
# Usage:
#   ./run.sh              # Start with defaults

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_PATH="$(dirname "$SCRIPT_DIR")"

export QUICKSHELL_LOCKSCREEN_AUTO_LOCK=0

echo "Starting Qypr locker (development)..."
echo "Config: $CONFIG_PATH"
echo ""

exec qs -p "$CONFIG_PATH" "$@"
