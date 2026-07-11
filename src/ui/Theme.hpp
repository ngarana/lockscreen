// Theme.hpp - Single source of truth for the visual design.
//
// Direct port of the QML Theme singleton (Catppuccin Mocha + glassmorphism).
// Every widget reads from here so colours/spacing/timing stay consistent (DRY).

#pragma once

#include "core/Types.hpp"

namespace qypr::theme {

namespace color {
inline const Color background = Color::fromHex("#1e1e2e");
inline const Color surface = Color::fromHex("#313244");
inline const Color surfaceHover = Color::fromHex("#45475a");

inline const Color glass = Color::fromHex("#40313244");
inline const Color glassHover = Color::fromHex("#60454a5a");
inline const Color glassBorder = Color::fromHex("#30cdd6f4");

inline const Color primary = Color::fromHex("#89b4fa");
inline const Color primaryGlow = Color::fromHex("#4089b4fa");

inline const Color text = Color::fromHex("#cdd6f4");
inline const Color textSubtle = Color::fromHex("#a6adc8");
inline const Color textMuted = Color::fromHex("#6c7086");

inline const Color error = Color::fromHex("#f38ba8");
inline const Color success = Color::fromHex("#a6e3a1");
inline const Color warning = Color::fromHex("#f9e2af");

// Catppuccin Mocha accents (app tiles, status dots, etc.).
inline const Color blue = Color::fromHex("#89b4fa");
inline const Color lavender = Color::fromHex("#b4befe");
inline const Color mauve = Color::fromHex("#cba6f7");
inline const Color pink = Color::fromHex("#f5c2e7");
inline const Color red = Color::fromHex("#f38ba8");
inline const Color peach = Color::fromHex("#fab387");
inline const Color yellow = Color::fromHex("#f9e2af");
inline const Color green = Color::fromHex("#a6e3a1");
inline const Color teal = Color::fromHex("#94e2d5");
inline const Color sky = Color::fromHex("#89dceb");
inline const Color maroon = Color::fromHex("#eba0ac");
}  // namespace color

namespace font {
inline constexpr int size = 16;
inline constexpr int sizeLarge = 24;
inline constexpr int sizeClock = 96;
inline constexpr int sizeDate = 28;
inline constexpr const char* family = "Inter";
inline constexpr const char* iconFamily = "CaskaydiaCove Nerd Font";
}  // namespace font

namespace spacing {
inline constexpr int small = 8;
inline constexpr int medium = 16;
inline constexpr int large = 24;
inline constexpr int xlarge = 48;
inline constexpr int xxlarge = 80;
}  // namespace spacing

namespace radius {
inline constexpr int small = 4;
inline constexpr int medium = 8;
inline constexpr int large = 16;
inline constexpr int xlarge = 24;
inline constexpr int round = 9999;
}  // namespace radius

namespace anim {
inline constexpr int fast = 150;
inline constexpr int medium = 300;
inline constexpr int slow = 500;
inline constexpr int reveal = 600;
}  // namespace anim

namespace effects {
inline constexpr double shadowOpacity = 0.6;
inline constexpr int shadowOffset = 2;
}  // namespace effects

namespace audio {
inline constexpr int buttonSize = 48;
inline constexpr int buttonIconSize = 20;
inline constexpr int minWidth = 320;
inline constexpr int maxWidth = 420;
inline constexpr int progressHeight = 4;
inline constexpr int volumeSliderWidth = 150;
inline constexpr int spacing = 12;
inline constexpr int panelPadding = 16;
}  // namespace audio

namespace notification {
inline constexpr int cardWidth = 360;
inline constexpr int iconSize = 36;
inline constexpr int padding = 16;
inline constexpr int gap = 12;       // vertical space between stacked cards
inline constexpr int titleSize = 14;
inline constexpr int bodySize = 13;
inline constexpr int maxVisible = 4;  // most recent cards shown
inline constexpr int radius = 12;
}  // namespace notification

}  // namespace qypr::theme
