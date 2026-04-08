#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_PATH="$SCRIPT_DIR"

echo "Starting lockscreen in development mode..."
echo "Config: $CONFIG_PATH"
echo ""

export QUICKSHELL_LOCKSCREEN_AUTO_LOCK=0
exec qs -p "$CONFIG_PATH" "$@"
