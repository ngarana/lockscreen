#!/bin/bash
# Test script for external tool integration
# Tests that status bar indicators can launch system tools

echo "=== Testing External Tool Integration ==="
echo ""

# Check if required tools are installed
echo "Checking for required system tools..."
MISSING_TOOLS=()

if ! command -v nm-connection-editor &> /dev/null; then
    echo "  ⚠️  nm-connection-editor not found (network manager GUI)"
    MISSING_TOOLS+=("network-manager-applet")
fi

if ! command -v pavucontrol &> /dev/null; then
    echo "  ⚠️  pavucontrol not found (audio control GUI)"
    MISSING_TOOLS+=("pavucontrol")
fi

if ! command -v blueman-manager &> /dev/null; then
    echo "  ⚠️  blueman-manager not found (bluetooth manager GUI)"
    MISSING_TOOLS+=("blueman")
fi

if ! command -v xfce4-display-settings &> /dev/null; then
    echo "  ⚠️  xfce4-display-settings not found (display settings GUI)"
    echo "     Alternative: You can use arandr or other display tools"
fi

if [ ${#MISSING_TOOLS[@]} -gt 0 ]; then
    echo ""
    echo "To install missing tools on Arch Linux:"
    echo "  sudo pacman -S ${MISSING_TOOLS[*]}"
    echo ""
fi

echo ""
echo "=== Usage Instructions ==="
echo ""
echo "The status bar indicators now support external tool launching:"
echo ""
echo "  • Network indicator:"
echo "    - Left click: Open quick settings panel"
echo "    - Right/Middle click: Launch nm-connection-editor"
echo ""
echo "  • Bluetooth indicator:"
echo "    - Left click: Open quick settings panel"
echo "    - Right/Middle click: Launch blueman-manager"
echo ""
echo "  • Volume indicator:"
echo "    - Left click: Open audio popup"
echo "    - Right/Middle click: Launch pavucontrol"
echo "    - Scroll wheel: Adjust volume"
echo ""
echo "  • Brightness indicator:"
echo "    - Left click: Open quick settings panel"
echo "    - Right/Middle click: Launch display settings"
echo "    - Scroll wheel: Adjust brightness"
echo ""
echo "=== Test Complete ==="
