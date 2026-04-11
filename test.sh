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

test_dir_exists() {
    local dir="$1"
    local description="$2"
    
    if [ -d "$dir" ]; then
        echo "✓ PASS: $description"
        PASS=$((PASS + 1))
    else
        echo "✗ FAIL: $description (directory not found: $dir)"
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

echo ""
echo "--- Services ---"
test_file_exists "$SCRIPT_DIR/src/services/qmldir" "Services qmldir exists"
test_file_exists "$SCRIPT_DIR/src/services/LockController.qml" "LockController service exists"
test_file_exists "$SCRIPT_DIR/src/services/PowerManager.qml" "PowerManager service exists"
test_file_exists "$SCRIPT_DIR/src/services/Theme.qml" "Theme service exists"
test_file_exists "$SCRIPT_DIR/src/services/VideoConfig.qml" "VideoConfig service exists"
test_file_exists "$SCRIPT_DIR/src/services/AudioService.qml" "AudioService service exists"

echo ""
echo "--- Components ---"
test_file_exists "$SCRIPT_DIR/src/components/qmldir" "Components qmldir exists"
test_file_exists "$SCRIPT_DIR/src/components/PasswordField.qml" "PasswordField component exists"
test_file_exists "$SCRIPT_DIR/src/components/Clock.qml" "Clock component exists"
test_file_exists "$SCRIPT_DIR/src/components/ActionButton.qml" "ActionButton component exists"
test_file_exists "$SCRIPT_DIR/src/components/StatusMessage.qml" "StatusMessage component exists"
test_file_exists "$SCRIPT_DIR/src/components/VideoBackground.qml" "VideoBackground component exists"
test_file_exists "$SCRIPT_DIR/src/components/AudioController.qml" "AudioController component exists"
test_file_exists "$SCRIPT_DIR/src/components/AudioMetadata.qml" "AudioMetadata component exists"
test_file_exists "$SCRIPT_DIR/src/components/AudioPlayerButton.qml" "AudioPlayerButton component exists"

echo ""
echo "--- Widgets ---"
test_file_exists "$SCRIPT_DIR/src/widgets/qmldir" "Widgets qmldir exists"
test_file_exists "$SCRIPT_DIR/src/widgets/LockScreen.qml" "LockScreen widget exists"

echo ""
echo "--- Scripts ---"
test_file_exists "$SCRIPT_DIR/run.sh" "Run script exists"
test_file_exists "$SCRIPT_DIR/lock.sh" "Lock script exists"
test_file_exists "$SCRIPT_DIR/README.md" "README exists"

echo ""
echo "--- Video Assets ---"
test_dir_exists "$SCRIPT_DIR/videos" "Videos directory/symlink exists"
test_file_exists "$SCRIPT_DIR/playlists/day.m3u" "Day playlist exists"
test_file_exists "$SCRIPT_DIR/playlists/night.m3u" "Night playlist exists"

echo ""
echo "--- Tests ---"
test_file_exists "$SCRIPT_DIR/tests/manual_test.sh" "Manual test script exists"
test_file_exists "$SCRIPT_DIR/tests/manual_audio_test.sh" "Manual audio test script exists"

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
