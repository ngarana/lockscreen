#!/usr/bin/env bash
# test.sh - Backward Compatibility Wrapper
# Redirects to scripts/test.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec "$SCRIPT_DIR/scripts/test.sh" "$@"
