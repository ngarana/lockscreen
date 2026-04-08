#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_PATH="$SCRIPT_DIR"

if qs ipc -p "$CONFIG_PATH" show >/dev/null 2>&1; then
    exec qs ipc -p "$CONFIG_PATH" call lockscreen lock
fi

echo "Starting lockscreen..."
echo "Config: $CONFIG_PATH"
echo ""

export QUICKSHELL_LOCKSCREEN_AUTO_LOCK=1
exec qs -d -p "$CONFIG_PATH" "$@"
