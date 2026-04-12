#!/usr/bin/env bash
# run.sh - Development Launcher for Qypr
#
# Starts Qypr in development mode with:
# - Auto-lock disabled
# - Hot-reload enabled
# - Full module suite (lockscreen, statusbar, launcher, etc.)
#
# Usage:
#   ./run.sh              # Start with defaults
#   ./run.sh --mode bar   # Start with status bar only
#   ./run.sh --mode lock  # Start with lockscreen only
#
# Environment Variables:
#   QUICKSHELL_MODE=full|bar|lock  - Override mode
#   QUICKSHELL_DEBUG=1             - Enable debug logging

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_PATH="$(dirname "$SCRIPT_DIR")"

# Parse arguments
MODE="${QUICKSHELL_MODE:-full}"
while [[ $# -gt 0 ]]; do
    case "$1" in
        --mode)
            MODE="$2"
            shift 2
            ;;
        --help|-h)
            echo "Usage: $0 [--mode full|bar|lock]"
            echo ""
            echo "Modes:"
            echo "  full  - All modules (default)"
            echo "  bar   - Status bar only"
            echo "  lock  - Lockscreen only"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

export QUICKSHELL_LOCKSCREEN_AUTO_LOCK=0
export QUICKSHELL_MODE="$MODE"

echo "╔══════════════════════════════════════════════════╗"
echo "║           Qypr - Development Mode                ║"
echo "╠══════════════════════════════════════════════════╣"
echo "║  Config: $CONFIG_PATH"
echo "║  Mode:   $MODE"
echo "║  Debug:  ${QUICKSHELL_DEBUG:-0}"
echo "╚══════════════════════════════════════════════════╝"
echo ""

exec qs -p "$CONFIG_PATH" "$@"
