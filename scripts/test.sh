#!/usr/bin/env bash
# test.sh - Qypr Test Suite Runner
#
# Runs comprehensive tests for the Qypr desktop shell system.
# Tests file structure, dependencies, module integrity, and more.
#
# Usage:
#   ./test.sh              # Run all tests
#   ./test.sh --structure  # Run structure tests only
#   ./test.sh --modules    # Run module tests only
#   ./test.sh --themes     # Run theme tests only

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PASS=0
FAIL=0
WARN=0

# ============================================================================
# Test Helpers
# ============================================================================

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

test_file_not_empty() {
    local file="$1"
    local description="$2"

    if [ -f "$file" ] && [ -s "$file" ]; then
        echo "✓ PASS: $description"
        PASS=$((PASS + 1))
    else
        echo "⚠ WARN: $description (file empty or missing: $file)"
        WARN=$((WARN + 1))
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

# ============================================================================
# Test Suites
# ============================================================================

test_structure() {
    echo ""
    echo "=== File Structure Tests ==="
    
    # Root files
    test_file_exists "$SCRIPT_DIR/shell.qml" "Main shell.qml exists"
    test_file_exists "$SCRIPT_DIR/.qmlls.ini" "LSP configuration exists"
    test_file_exists "$SCRIPT_DIR/README.md" "README exists"
    test_file_exists "$SCRIPT_DIR/AGENTS.md" "AGENTS.md exists"
    test_file_not_empty "$SCRIPT_DIR/docs/ROADMAP.md" "ROADMAP.md exists and is not empty"
    
    echo ""
    echo "--- Core Utilities ---"
    test_dir_exists "$SCRIPT_DIR/src/core" "Core directory exists"
    test_file_exists "$SCRIPT_DIR/src/core/qmldir" "Core qmldir exists"
    test_file_exists "$SCRIPT_DIR/src/core/Constants.qml" "Constants.qml exists"
    test_file_exists "$SCRIPT_DIR/src/core/Logger.qml" "Logger.qml exists"
    test_file_exists "$SCRIPT_DIR/src/core/Utils.qml" "Utils.qml exists"
    test_file_exists "$SCRIPT_DIR/src/core/Errors.qml" "Errors.qml exists"
    
    echo ""
    echo "--- Theme System ---"
    test_dir_exists "$SCRIPT_DIR/src/theme" "Theme directory exists"
    test_file_exists "$SCRIPT_DIR/src/theme/qmldir" "Theme qmldir exists"
    test_file_exists "$SCRIPT_DIR/src/theme/ThemeEngine.qml" "ThemeEngine.qml exists"
    test_file_exists "$SCRIPT_DIR/src/theme/ColorPalette.qml" "ColorPalette.qml exists"
    test_file_exists "$SCRIPT_DIR/src/theme/Typography.qml" "Typography.qml exists"
    test_file_exists "$SCRIPT_DIR/src/theme/Spacing.qml" "Spacing.qml exists"
    test_file_exists "$SCRIPT_DIR/src/theme/Effects.qml" "Effects.qml exists"
    test_file_exists "$SCRIPT_DIR/src/theme/AnimationTokens.qml" "AnimationTokens.qml exists"
}

test_modules() {
    echo ""
    echo "=== Module Tests ==="
    
    echo ""
    echo "--- Module Directories ---"
    test_dir_exists "$SCRIPT_DIR/src/modules/lockscreen" "Lockscreen module directory exists"
    test_dir_exists "$SCRIPT_DIR/src/modules/statusbar" "Statusbar module directory exists"
    test_dir_exists "$SCRIPT_DIR/src/modules/launcher" "Launcher module directory exists"
    test_dir_exists "$SCRIPT_DIR/src/modules/notifications" "Notifications module directory exists"
    test_dir_exists "$SCRIPT_DIR/src/modules/controlcenter" "Control center module directory exists"
    
    echo ""
    echo "--- Lockscreen Module ---"
    test_file_exists "$SCRIPT_DIR/src/modules/lockscreen/qmldir" "Lockscreen qmldir exists"
    test_file_exists "$SCRIPT_DIR/src/modules/lockscreen/LockScreen.qml" "LockScreen.qml exists"
    test_file_exists "$SCRIPT_DIR/src/modules/lockscreen/LockController.qml" "LockController.qml exists"
    
    echo ""
    echo "--- Status Bar Module ---"
    test_file_exists "$SCRIPT_DIR/src/modules/statusbar/qmldir" "Statusbar qmldir exists"
    
    echo ""
    echo "--- Launcher Module ---"
    test_file_exists "$SCRIPT_DIR/src/modules/launcher/qmldir" "Launcher qmldir exists"
    
    echo ""
    echo "--- Notifications Module ---"
    test_file_exists "$SCRIPT_DIR/src/modules/notifications/qmldir" "Notifications qmldir exists"
    
    echo ""
    echo "--- Control Center Module ---"
    test_file_exists "$SCRIPT_DIR/src/modules/controlcenter/qmldir" "Control center qmldir exists"
}

test_components() {
    echo ""
    echo "=== Component Tests ==="
    
    echo ""
    echo "--- Components Directory ---"
    test_dir_exists "$SCRIPT_DIR/src/components" "Components directory exists"
    test_file_exists "$SCRIPT_DIR/src/components/qmldir" "Components qmldir exists"
    test_file_exists "$SCRIPT_DIR/src/components/ActionButton.qml" "ActionButton.qml exists"
    test_file_exists "$SCRIPT_DIR/src/components/AudioController.qml" "AudioController.qml exists"
    test_file_exists "$SCRIPT_DIR/src/components/AudioMetadata.qml" "AudioMetadata.qml exists"
    test_file_exists "$SCRIPT_DIR/src/components/AudioPlayerButton.qml" "AudioPlayerButton.qml exists"
    test_file_exists "$SCRIPT_DIR/src/components/Clock.qml" "Clock.qml exists"
    test_file_exists "$SCRIPT_DIR/src/components/PasswordField.qml" "PasswordField.qml exists"
    test_file_exists "$SCRIPT_DIR/src/components/StatusMessage.qml" "StatusMessage.qml exists"
    test_file_exists "$SCRIPT_DIR/src/components/VideoBackground.qml" "VideoBackground.qml exists"
    
    echo ""
    echo "--- Atomic Components ---"
    test_dir_exists "$SCRIPT_DIR/src/atoms" "Atoms directory exists"
    test_file_exists "$SCRIPT_DIR/src/atoms/qmldir" "Atoms qmldir exists"
    
    echo ""
    echo "--- Molecular Components ---"
    test_dir_exists "$SCRIPT_DIR/src/molecules" "Molecules directory exists"
    test_file_exists "$SCRIPT_DIR/src/molecules/qmldir" "Molecules qmldir exists"
}

test_services() {
    echo ""
    echo "=== Service Tests ==="
    
    test_dir_exists "$SCRIPT_DIR/src/services" "Services directory exists"
    test_file_exists "$SCRIPT_DIR/src/services/qmldir" "Services qmldir exists"
    test_file_exists "$SCRIPT_DIR/src/services/LockController.qml" "LockController service exists"
    test_file_exists "$SCRIPT_DIR/src/services/PowerManager.qml" "PowerManager service exists"
    test_file_exists "$SCRIPT_DIR/src/services/Theme.qml" "Theme service exists (legacy)"
}

test_models() {
    echo ""
    echo "=== Model Tests ==="
    
    test_dir_exists "$SCRIPT_DIR/src/models" "Models directory exists"
    test_file_exists "$SCRIPT_DIR/src/models/qmldir" "Models qmldir exists"
}

test_themes() {
    echo ""
    echo "=== Theme Tests ==="
    
    test_dir_exists "$SCRIPT_DIR/themes" "Themes directory exists"
    test_dir_exists "$SCRIPT_DIR/themes/default" "Default theme directory exists"
    test_file_exists "$SCRIPT_DIR/themes/default/theme.json" "Default theme.json exists"
    test_dir_exists "$SCRIPT_DIR/themes/catppuccin-latte" "Catppuccin Latte theme directory exists"
    test_file_exists "$SCRIPT_DIR/themes/catppuccin-latte/theme.json" "Catppuccin Latte theme.json exists"
}

test_scripts() {
    echo ""
    echo "=== Script Tests ==="
    
    test_dir_exists "$SCRIPT_DIR/scripts" "Scripts directory exists"
    test_file_exists "$SCRIPT_DIR/scripts/run.sh" "Run script exists"
    test_file_exists "$SCRIPT_DIR/scripts/lock.sh" "Lock script exists"
    test_file_exists "$SCRIPT_DIR/scripts/bar.sh" "Bar script exists"
    test_file_exists "$SCRIPT_DIR/scripts/launcher.sh" "Launcher script exists"
    
    # Check backward compatibility wrappers
    test_file_exists "$SCRIPT_DIR/run.sh" "Root run.sh wrapper exists"
    test_file_exists "$SCRIPT_DIR/lock.sh" "Root lock.sh wrapper exists"
    
    # Check scripts are executable
    if [ -x "$SCRIPT_DIR/scripts/run.sh" ]; then
        echo "✓ PASS: Scripts are executable"
        PASS=$((PASS + 1))
    else
        echo "✗ FAIL: Scripts are not executable"
        FAIL=$((FAIL + 1))
    fi
}

test_assets() {
    echo ""
    echo "=== Asset Tests ==="
    
    test_dir_exists "$SCRIPT_DIR/assets" "Assets directory exists"
    test_dir_exists "$SCRIPT_DIR/assets/icons" "Icons directory exists"
    test_dir_exists "$SCRIPT_DIR/assets/icons/actions" "Action icons directory exists"
    test_dir_exists "$SCRIPT_DIR/assets/icons/apps" "App icons directory exists"
    test_dir_exists "$SCRIPT_DIR/assets/icons/categories" "Category icons directory exists"
    test_dir_exists "$SCRIPT_DIR/assets/icons/status" "Status icons directory exists"
    test_dir_exists "$SCRIPT_DIR/assets/icons/system" "System icons directory exists"
    test_dir_exists "$SCRIPT_DIR/assets/videos" "Videos directory exists"
    test_dir_exists "$SCRIPT_DIR/assets/sounds" "Sounds directory exists"
    test_dir_exists "$SCRIPT_DIR/assets/fonts" "Fonts directory exists"
    test_dir_exists "$SCRIPT_DIR/assets/wallpapers" "Wallpapers directory exists"
    
    echo ""
    echo "--- Playlists ---"
    test_file_exists "$SCRIPT_DIR/playlists/day.m3u" "Day playlist exists"
    test_file_exists "$SCRIPT_DIR/playlists/night.m3u" "Night playlist exists"
}

test_layouts() {
    echo ""
    echo "=== Layout Tests ==="
    
    test_dir_exists "$SCRIPT_DIR/src/layouts" "Layouts directory exists"
    test_file_exists "$SCRIPT_DIR/src/layouts/qmldir" "Layouts qmldir exists"
}

test_popups() {
    echo ""
    echo "=== Popup Tests ==="
    
    test_dir_exists "$SCRIPT_DIR/src/popups" "Popups directory exists"
    test_file_exists "$SCRIPT_DIR/src/popups/qmldir" "Popups qmldir exists"
}

test_animations() {
    echo ""
    echo "=== Animation Tests ==="
    
    test_dir_exists "$SCRIPT_DIR/src/animations" "Animations directory exists"
    test_file_exists "$SCRIPT_DIR/src/animations/qmldir" "Animations qmldir exists"
}

test_dependencies() {
    echo ""
    echo "=== Dependency Tests ==="
    
    test_dependency "qs" "qs CLI is installed"
    test_dependency "systemctl" "systemctl is available for power management"
}

# ============================================================================
# Main Test Runner
# ============================================================================

echo "╔══════════════════════════════════════════════════╗"
echo "║              Qypr Test Suite                     ║"
echo "╚══════════════════════════════════════════════════╝"

# Check for specific test suite
TEST_SUITE="${1:-all}"

case "$TEST_SUITE" in
    --structure|-s)
        test_structure
        ;;
    --modules|-m)
        test_modules
        test_services
        test_models
        ;;
    --components|-c)
        test_components
        test_layouts
        test_popups
        test_animations
        ;;
    --themes|-t)
        test_themes
        ;;
    --scripts|-sc)
        test_scripts
        ;;
    --assets|-a)
        test_assets
        ;;
    --all|all|"")
        test_structure
        test_modules
        test_components
        test_services
        test_models
        test_themes
        test_scripts
        test_assets
        test_layouts
        test_popups
        test_animations
        ;;
    --help|-h)
        echo "Usage: $0 [--structure|--modules|--components|--themes|--scripts|--assets|--all]"
        exit 0
        ;;
    *)
        echo "Unknown test suite: $TEST_SUITE"
        echo "Use --help for usage information"
        exit 1
        ;;
esac

test_dependencies

echo ""
echo "╔══════════════════════════════════════════════════╗"
echo "║                  Test Summary                    ║"
echo "╠══════════════════════════════════════════════════╣"
echo "║  Passed: $PASS"
echo "║  Failed: $FAIL"
echo "║  Warnings: $WARN"
echo "╚══════════════════════════════════════════════════╝"
echo ""

if [ "$FAIL" -eq 0 ]; then
    echo "✓ All tests passed!"
    exit 0
else
    echo "✗ Some tests failed."
    exit 1
fi
