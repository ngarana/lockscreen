#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TESTS_DIR="$SCRIPT_DIR/tests"

echo "Running lockscreen tests..."
echo ""

PASS=0
FAIL=0

test_file_exists() {
    local file="$1"
    local description="$2"
    
    if [ -f "$file" ]; then
        echo "✓ PASS: $description"
        PASS=$((PASS + 1))
    else
        echo "✗ FAIL: $description (file not found: $file)"
        FAIL=$((FAIL + 1))
    fi
}

test_dependency() {
    local cmd="$1"
    local description="$2"
    
    if command -v "$cmd" >/dev/null 2>&1; then
        echo "✓ PASS: $description"
        PASS=$((PASS + 1))
    else
        echo "✗ FAIL: $description"
        FAIL=$((FAIL + 1))
    fi
}

echo "=== File Structure Tests ==="
test_file_exists "$SCRIPT_DIR/shell.qml" "Main shell.qml exists"
test_file_exists "$SCRIPT_DIR/src/services/LockController.qml" "LockController service exists"
test_file_exists "$SCRIPT_DIR/src/services/PowerManager.qml" "PowerManager service exists"
test_file_exists "$SCRIPT_DIR/src/services/Theme.qml" "Theme service exists"
test_file_exists "$SCRIPT_DIR/src/components/PasswordField.qml" "PasswordField component exists"
test_file_exists "$SCRIPT_DIR/src/components/Clock.qml" "Clock component exists"
test_file_exists "$SCRIPT_DIR/src/components/ActionButton.qml" "ActionButton component exists"
test_file_exists "$SCRIPT_DIR/src/components/StatusMessage.qml" "StatusMessage component exists"
test_file_exists "$SCRIPT_DIR/src/widgets/LockScreen.qml" "LockScreen widget exists"
test_file_exists "$SCRIPT_DIR/run.sh" "Run script exists"
test_file_exists "$SCRIPT_DIR/lock.sh" "Lock script exists"
test_file_exists "$SCRIPT_DIR/README.md" "README exists"

echo ""
echo "=== Dependency Tests ==="
test_dependency "qs" "qs CLI is installed"
test_dependency "systemctl" "systemctl is available for power management"

echo ""
echo "=== Summary ==="
echo "Passed: $PASS"
echo "Failed: $FAIL"
echo ""

if [ "$FAIL" -eq 0 ]; then
    echo "All tests passed!"
    exit 0
else
    echo "Some tests failed."
    exit 1
fi
