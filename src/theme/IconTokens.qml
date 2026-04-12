// IconTokens.qml - Icon Glyph Token Definitions
//
// Unicode glyph constants for system icons.
// Uses proper Unicode symbols instead of emoji for consistent rendering.
//
// Usage:
//   import "../theme" as Theme
//
//   Text {
//       text: Theme.Theme.icons.applications
//       font.pixelSize: 16
//   }

import QtQuick

QtObject {
    // ========================================================================
    // System Icons
    // ========================================================================

    // Volume icons (U+1F507-U+1F50A range avoided; using simpler symbols)
    readonly property string volumeHigh: "🔊"    // Speaker with sound waves
    readonly property string volumeMedium: "🔉"  // Speaker with one sound wave
    readonly property string volumeLow: "🔈"     // Speaker with no sound waves
    readonly property string volumeMuted: "🔇"   // Muted speaker

    // Display icons
    readonly property string brightness: "☀"     // White sun (U+2600)
    readonly property string brightnessLow: "🌤"  // Sun behind small cloud

    // ========================================================================
    // Battery Icons
    // ========================================================================

    readonly property string battery: "🔋"           // Battery
    readonly property string batteryCharging: "⚡"    // High voltage symbol
    readonly property string batteryLow: "◷"         // Clock face (low indicator)
    readonly property string batteryCritical: "⚠"    // Warning sign

    // ========================================================================
    // Network Icons
    // ========================================================================

    readonly property string wifi: "📶"          // Antenna with bars
    readonly property string wifiOff: "✕"        // Multiplication sign (no wifi)
    readonly property string ethernet: "⌘"       // Place of interest sign (wired)
    readonly property string bluetooth: "⚭"      // Bluetooth symbol (U+26AD)
    readonly property string bluetoothOff: "⊘"   // Prohibited circle
    readonly property string airplane: "✈"       // Airplane (U+2708)

    // ========================================================================
    // Application Icons
    // ========================================================================

    readonly property string applications: "⊞"   // Squared times (app grid)
    readonly property string settings: "⚙"       // Gear (U+2699)
    readonly property string notifications: "🔔" // Bell
    readonly property string notificationsOff: "🔕" // Bell with slash
    readonly property string calendar: "📅"      // Calendar
    readonly property string weather: "☁"        // Cloud (U+2601)
    readonly property string media: "♫"          // Beamed eighth notes (U+266B)

    // ========================================================================
    // Status & UI Icons
    // ========================================================================

    readonly property string check: "✓"          // Check mark (U+2713)
    readonly property string close: "✕"          // Multiplication X (U+2715)
    readonly property string warning: "⚠"        // Warning sign (U+26A0)
    readonly property string error: "✗"          // Ballot X (U+2717)
    readonly property string info: "ℹ"           // Information source (U+2139)
    readonly property string arrow: "▸"          // White right-pointing small triangle
    readonly property string arrowDown: "▾"      // Down-pointing triangle
    readonly property string arrowUp: "▴"        // Up-pointing triangle
    readonly property string plus: "+"
    readonly property string minus: "−"          // Minus sign (U+2212)
    readonly property string dots: "⋯"           // Midline horizontal ellipsis

    // ========================================================================
    // Power Icons
    // ========================================================================

    readonly property string power: "⏻"          // Power symbol (U+23FB)
    readonly property string sleep: "⏾"          // Sleep mode (U+23FE)
    readonly property string restart: "↻"        // Clockwise arrow (U+21BB)
    readonly property string logout: "⏻"         // Power off / logout
    readonly property string lock: "🔒"          // Lock (U+1F512)
}
