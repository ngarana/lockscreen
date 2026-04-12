// IconTokens.qml - Icon Glyph Token Definitions
//
// Unicode glyph constants for system icons.
// Uses Nerd Font icons for consistent rendering across all systems.
//
// Usage:
//   import "../theme" as Theme
//
//   Text {
//       text: Theme.ThemeEngine.icons.applications
//       font.pixelSize: 16
//       font.family: Theme.fonts.iconFontFamily
//   }

import QtQuick

QtObject {
    // ========================================================================
    // System Icons
    // ========================================================================

    // Volume icons (Nerd Font)
    readonly property string volumeHigh: "󰕾"     // nf-md-volume_high
    readonly property string volumeMedium: "󰖀"   // nf-md-volume_medium
    readonly property string volumeLow: "󰕿"      // nf-md_volume_low
    readonly property string volumeMuted: "󰖁"    // nf-md_volume_mute

    // Display icons
    readonly property string brightness: "󰃠"     // nf-md-brightness_5
    readonly property string brightnessLow: "󰃞"  // nf-md-brightness_low

    // ========================================================================
    // Battery Icons
    // ========================================================================

    readonly property string battery: "󰁹"           // nf-md-battery
    readonly property string batteryCharging: "󰂄"    // nf-md-battery_charging
    readonly property string batteryLow: "󰂎"         // nf-md-battery_alert
    readonly property string batteryCritical: "󰂃"    // nf-md-battery_charging_10

    // ========================================================================
    // Network Icons
    // ========================================================================

    readonly property string wifi: "󰤨"          // nf-md-wifi
    readonly property string wifiOff: "󰤮"        // nf-md-wifi_off
    readonly property string ethernet: "󰈀"       // nf-md-ethernet
    readonly property string bluetooth: "󰂯"      // nf-md-bluetooth
    readonly property string bluetoothOff: "󰂲"   // nf-md-bluetooth_off
    readonly property string airplane: "󰀝"       // nf-md-airplane

    // ========================================================================
    // Display & Screen Icons
    // ========================================================================

    readonly property string screenMirroring: "󰍺"   // nf-md-mirror
    readonly property string display: "󰍹"            // nf-md-monitor
    readonly property string displayExternal: "󰍹"    // nf-md-monitor
    readonly property string nightLight: "󰖔"         // nf-md-nightlight
    readonly property string focus: "󰋲"              // nf-md-target
    readonly property string creativeCloud: "󰖐"      // nf-md-cloud

    // ========================================================================
    // User & Profile Icons
    // ========================================================================

    readonly property string user: "󰈠"           // nf-md-account
    readonly property string userGroup: "󰈂"      // nf-md-account_group
    readonly property string avatar: "󰈣"          // nf-md-account_circle

    // ========================================================================
    // Weather Icons
    // ========================================================================

    readonly property string weatherSun: "󰖙"         // nf-md-white_balance_sunny
    readonly property string weatherPartlyCloudy: "󰖕" // nf-md-weather_partly_cloudy
    readonly property string weatherCloudy: "󰖐"      // nf-md-cloud
    readonly property string weatherRain: "󰼳"       // nf-md-weather_rainy
    readonly property string weatherSnow: "󰼴"       // nf-md-weather_snowy
    readonly property string weatherStorm: "󰙾"       // nf-md-weather_lightning
    readonly property string weatherFog: "󰖑"        // nf-md-weather_fog

    // ========================================================================
    // Status & Utility Icons
    // ========================================================================

    readonly property string dice: "󰎴"           // nf-md-dice
    readonly property string target: "󰋲"          // nf-md-target
    readonly property string grid: "󰏘"            // nf-md-view_grid
    readonly property string folder: "󰉋"         // nf-md-folder
    readonly property string timer: "󰥔"           // nf-md-timer
    readonly property string mic: "󰍬"            // nf-md-microphone
    readonly property string search: "󰍉"          // nf-md-magnify
    readonly property string closeCircle: "󰨥"     // nf-md-close_circle
    readonly property string window: "󰏘"          // nf-md-window

    // ========================================================================
    // Application Icons
    // ========================================================================

    readonly property string applications: "󰏘"   // nf-md-view_grid
    readonly property string settings: "󰒓"       // nf-md-cog
    readonly property string notifications: "󰂚"  // nf-md-bell
    readonly property string notificationsOff: "󰂛" // nf-md-bell_off
    readonly property string calendar: "󰃭"      // nf-md-calendar
    readonly property string weather: "󰖐"        // nf-md-cloud
    readonly property string media: "󰎈"          // nf-md-music

    // ========================================================================
    // Status & UI Icons
    // ========================================================================

    readonly property string check: "✓"          // Check mark
    readonly property string close: "✕"          // Multiplication X
    readonly property string warning: "⚠"        // Warning sign
    readonly property string error: "✗"          // Ballot X
    readonly property string info: "ℹ"           // Information source
    readonly property string arrow: "▸"          // Right-pointing triangle
    readonly property string arrowDown: "▾"      // Down-pointing triangle
    readonly property string arrowUp: "▴"        // Up-pointing triangle
    readonly property string plus: "+"
    readonly property string minus: "−"          // Minus sign
    readonly property string dots: "⋯"           // Midline horizontal ellipsis

    // ========================================================================
    // Power Icons
    // ========================================================================

    readonly property string power: "󰐥"          // nf-md-power
    readonly property string sleep: "󰤄"          // nf-md-sleep
    readonly property string restart: "󰑐"        // nf-md-restart
    readonly property string logout: "󰍃"         // nf-md-logout
    readonly property string lock: "󰌾"           // nf-md-lock
}
