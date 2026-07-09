#!/usr/bin/env bash
# test.sh - Qypr Locker Test Suite Runner
#
# Tests file structure, dependencies, and module integrity
# for the standalone lockscreen module.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PASS=0
FAIL=0

# ============================================================================
# Test Helpers
# ============================================================================

test_file_exists() {
    local file="$1"
    local description="$2"
    if [ -f "$file" ]; then
        echo "PASS: $description"
        PASS=$((PASS + 1))
    else
        echo "FAIL: $description (file not found: $file)"
        FAIL=$((FAIL + 1))
    fi
}

test_dir_exists() {
    local dir="$1"
    local description="$2"
    if [ -d "$dir" ]; then
        echo "PASS: $description"
        PASS=$((PASS + 1))
    else
        echo "FAIL: $description (directory not found: $dir)"
        FAIL=$((FAIL + 1))
    fi
}

test_file_not_empty() {
    local file="$1"
    local description="$2"
    if [ -s "$file" ]; then
        echo "PASS: $description"
        PASS=$((PASS + 1))
    else
        echo "FAIL: $description (file is empty)"
        FAIL=$((FAIL + 1))
    fi
}

test_grep() {
    local pattern="$1"
    local file="$2"
    local description="$3"
    if grep -q "$pattern" "$file"; then
        echo "PASS: $description"
        PASS=$((PASS + 1))
    else
        echo "FAIL: $description (pattern not found: $pattern)"
        FAIL=$((FAIL + 1))
    fi
}

# ============================================================================
# Structure Tests
# ============================================================================

echo "=== Structure Tests ==="
test_dir_exists "$SCRIPT_DIR/src" "Source directory exists"
test_dir_exists "$SCRIPT_DIR/src/modules/lockscreen" "Lockscreen module directory exists"
test_file_exists "$SCRIPT_DIR/src/modules/lockscreen/qmldir" "Lockscreen qmldir exists"
test_file_exists "$SCRIPT_DIR/src/modules/lockscreen/LockScreen.qml" "LockScreen.qml exists"
test_file_exists "$SCRIPT_DIR/src/modules/lockscreen/LockController.qml" "LockController.qml exists"

echo ""
echo "--- Components ---"
test_dir_exists "$SCRIPT_DIR/src/components" "Components directory exists"
test_file_exists "$SCRIPT_DIR/src/components/qmldir" "Components qmldir exists"
test_file_exists "$SCRIPT_DIR/src/components/VideoBackground.qml" "VideoBackground.qml exists"
test_file_exists "$SCRIPT_DIR/src/components/Clock.qml" "Clock.qml exists"
test_file_exists "$SCRIPT_DIR/src/components/PasswordField.qml" "PasswordField.qml exists"
test_file_exists "$SCRIPT_DIR/src/components/StatusMessage.qml" "StatusMessage.qml exists"
test_file_exists "$SCRIPT_DIR/src/components/ActionButton.qml" "ActionButton.qml exists"
test_file_exists "$SCRIPT_DIR/src/components/AudioController.qml" "AudioController.qml exists"
test_file_exists "$SCRIPT_DIR/src/components/AudioMetadata.qml" "AudioMetadata.qml exists"
test_file_exists "$SCRIPT_DIR/src/components/AudioPlayerButton.qml" "AudioPlayerButton.qml exists"

echo ""
echo "--- Services ---"
test_dir_exists "$SCRIPT_DIR/src/services" "Services directory exists"
test_file_exists "$SCRIPT_DIR/src/services/qmldir" "Services qmldir exists"
test_file_exists "$SCRIPT_DIR/src/services/Theme.qml" "Theme service exists"
test_file_exists "$SCRIPT_DIR/src/services/VideoConfig.qml" "VideoConfig exists"
test_file_exists "$SCRIPT_DIR/src/services/PowerManager.qml" "PowerManager exists"
test_grep "singleton LockController 1.0 ../modules/lockscreen/LockController.qml" \
    "$SCRIPT_DIR/src/services/qmldir" "LockController re-export in qmldir"

echo ""
echo "--- Audio Module ---"
test_dir_exists "$SCRIPT_DIR/src/audio" "Audio module exists"
test_file_exists "$SCRIPT_DIR/src/audio/qmldir" "Audio qmldir exists"
test_file_exists "$SCRIPT_DIR/src/audio/AudioService.qml" "AudioService.qml exists"

echo ""
echo "--- Theme Module ---"
test_dir_exists "$SCRIPT_DIR/src/theme" "Theme module exists"
test_file_exists "$SCRIPT_DIR/src/theme/qmldir" "Theme qmldir exists"
test_file_exists "$SCRIPT_DIR/src/theme/ThemeEngine.qml" "ThemeEngine.qml exists"
test_file_exists "$SCRIPT_DIR/src/theme/ColorPalette.qml" "ColorPalette.qml exists"
test_file_exists "$SCRIPT_DIR/src/theme/ColorPaletteGruvbox.qml" "ColorPaletteGruvbox.qml exists"
test_file_exists "$SCRIPT_DIR/src/theme/Typography.qml" "Typography.qml exists"
test_file_exists "$SCRIPT_DIR/src/theme/Spacing.qml" "Spacing.qml exists"
test_file_exists "$SCRIPT_DIR/src/theme/Effects.qml" "Effects.qml exists"
test_file_exists "$SCRIPT_DIR/src/theme/AnimationTokens.qml" "AnimationTokens.qml exists"
test_file_exists "$SCRIPT_DIR/src/theme/IconTokens.qml" "IconTokens.qml exists"

echo ""
echo "--- Shell Entry Point ---"
test_file_exists "$SCRIPT_DIR/shell.qml" "Main shell.qml exists"

# ============================================================================
# Video Assets Tests
# ============================================================================

echo ""
echo "=== Video Assets Tests ==="
test_dir_exists "$SCRIPT_DIR/playlists" "Playlists directory exists"
test_file_exists "$SCRIPT_DIR/playlists/day.m3u" "Day playlist exists"
test_file_exists "$SCRIPT_DIR/playlists/night.m3u" "Night playlist exists"
if [ -d "$SCRIPT_DIR/videos" ]; then
    VIDEO_COUNT=$(find "$SCRIPT_DIR/videos" -maxdepth 1 -type f,l | wc -l)
    if [ "$VIDEO_COUNT" -gt 0 ]; then
        echo "PASS: Videos directory contains $VIDEO_COUNT video files"
        PASS=$((PASS + 1))
    else
        echo "FAIL: Videos directory is empty"
        FAIL=$((FAIL + 1))
    fi
else
    echo "FAIL: Videos directory not found"
    FAIL=$((FAIL + 1))
fi

# ============================================================================
# QML Syntax Tests
# ============================================================================

echo ""
echo "=== QML Lint Tests ==="
if command -v qmllint &> /dev/null; then
    find "$SCRIPT_DIR/src" -name "*.qml" | while read -r file; do
        if qmllint "$file" &> /dev/null; then
            echo "PASS: $file passes qmllint"
            PASS=$((PASS + 1))
        else
            echo "FAIL: $file fails qmllint"
            FAIL=$((FAIL + 1))
        fi
    done
else
    echo "SKIP: qmllint not available"
fi

# ============================================================================
# Summary
# ============================================================================

echo ""
echo "========================================"
echo " Test Summary"
echo "========================================"
echo " Passed: $PASS"
echo " Failed: $FAIL"
echo "========================================"

if [ "$FAIL" -eq 0 ]; then
    echo "All tests passed!"
    exit 0
else
    echo "Some tests failed."
    exit 1
fi
