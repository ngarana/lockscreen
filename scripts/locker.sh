#!/usr/bin/env bash
# locker.sh - Standalone Locker Launcher for Qypr
#
# Launches the locker as a standalone module without the full shell.
# Uses the dedicated locker/ entry point for minimal dependency.
#
# Usage:
#   ./locker.sh                    # Lock the screen (standalone)
#
# Environment Variables:
#   QUICKSHELL_DEBUG=1             - Enable debug logging

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_PATH="$(dirname "$SCRIPT_DIR")/locker"

export QUICKSHELL_LOCKSCREEN_AUTO_LOCK=1

echo "Starting Qypr locker (standalone)..."
echo "Config: $CONFIG_PATH"
echo ""

exec qs -d -p "$CONFIG_PATH" "$@"
