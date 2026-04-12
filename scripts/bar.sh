#!/usr/bin/env bash
# bar.sh - Status Bar Launcher for Qypr
#
# Starts Qypr with only the status bar module enabled.
# Useful for testing the status bar independently.
#
# Usage:
#   ./bar.sh              # Start status bar
#   ./bar.sh --reload     # Reload existing instance

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_PATH="$(dirname "$SCRIPT_DIR")"

# Check for reload
if [[ "${1:-}" == "--reload" ]]; then
    echo "Sending reload command..."
    qs ipc -p "$CONFIG_PATH" show 2>/dev/null || true
    exit 0
fi

# Check if Qypr is already running
if qs ipc -p "$CONFIG_PATH" show >/dev/null 2>&1; then
    echo "Qypr is already running."
    echo "Use 'qs ipc -p $CONFIG_PATH call statusbar toggle' to toggle visibility"
    exit 1
fi

export QUICKSHELL_AUTO_LOCK=0
export QUICKSHELL_MODE="bar"

echo "╔══════════════════════════════════════════════════╗"
echo "║           Qypr - Status Bar Mode                 ║"
echo "╠══════════════════════════════════════════════════╣"
echo "║  Config: $CONFIG_PATH"
echo "║  Mode:   bar"
echo "╚══════════════════════════════════════════════════╝"
echo ""

exec qs -p "$CONFIG_PATH" "$@"
