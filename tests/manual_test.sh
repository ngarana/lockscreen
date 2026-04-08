#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Test configuration for manual testing

# Test 1: Check if lockscreen loads without errors
test_load() {
    echo "Testing: Loading lockscreen..."
    timeout 3 env QUICKSHELL_LOCKSCREEN_AUTO_LOCK=0 qs -p "$SCRIPT_DIR" 2>&1 | head -20
}

# Test 2: Test PAM integration (requires password input)
test_pam() {
    echo "Testing: PAM integration..."
    echo "This test requires manual password input"
    echo "Enter your password when prompted"
}

# Test 3: Test power commands
test_power() {
    echo "Testing: Power management commands..."

    for capability in CanSuspend CanReboot CanPowerOff CanHibernate; do
        value="$(systemctl show -p "$capability" --value 2>/dev/null || echo "unknown")"
        echo "$capability: $value"
    done
}

# Test 4: Test IPC interface
test_ipc() {
    echo "Testing: IPC interface..."
    echo "Run: qs ipc -p \"$SCRIPT_DIR\" show"
    echo "Run: qs ipc -p \"$SCRIPT_DIR\" call lockscreen status"
    echo "Run: qs ipc -p \"$SCRIPT_DIR\" call lockscreen lock"
    echo "Run: qs kill -p \"$SCRIPT_DIR\""
}

# Run selected test
case "${1:-all}" in
    load) test_load ;;
    pam) test_pam ;;
    power) test_power ;;
    ipc) test_ipc ;;
    all)
        test_load
        echo "---"
        test_power
        echo "---"
        test_ipc
        ;;
    *)
        echo "Usage: $0 {load|pam|power|ipc|all}"
        exit 1
        ;;
esac
