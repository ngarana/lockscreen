// Theme.cpp - Runtime theme loading from bar.conf [theme] section.
#include "ui/Theme.hpp"

#include "core/Config.hpp"

namespace qypr::theme {

namespace {
// Override a Color from config if the key exists.
void overrideColor(Color& target, const Config& cfg, const char* section, const char* key) {
    if (cfg.has(section, key)) target = Color::fromHex(cfg.getString(section, key, ""));
}

// Override an int from config if the key exists.
void overrideInt(int& target, const Config& cfg, const char* section, const char* key) {
    if (cfg.has(section, key)) target = cfg.getInt(section, key, target);
}

// Override a double from config if the key exists.
void overrideDouble(double& target, const Config& cfg, const char* section, const char* key) {
    if (cfg.has(section, key)) target = cfg.getDouble(section, key, target);
}
}  // namespace

void loadTheme(const Config& cfg) {
    // Reset to compiled defaults before applying overrides
    color::background = Color::fromHex("#0d0e15");
    color::surface = Color::fromHex("#181a24");
    color::surfaceHover = Color::fromHex("#222534");
    color::glass = Color::fromHex("#181a24").withAlpha(0.65);
    color::glassHover = Color::fromHex("#222534").withAlpha(0.75);
    color::glassBorder = Color::fromHex("#ffffff").withAlpha(0.08);
    color::primary = Color::fromHex("#89b4fa");
    color::primaryGlow = Color::fromHex("#4089b4fa");
    color::text = Color::fromHex("#cdd6f4");
    color::textSubtle = Color::fromHex("#a6adc8");
    color::textMuted = Color::fromHex("#6c7086");
    color::error = Color::fromHex("#f38ba8");
    color::success = Color::fromHex("#a6e3a1");
    color::warning = Color::fromHex("#f9e2af");

    font::family = "Inter";
    font::iconFamily = "CaskaydiaCove Nerd Font";
    font::size = 16;
    font::sizeLarge = 22;
    font::sizeClock = 64;
    font::sizeDate = 18;

    // Menu-bar backdrop resets to compiled-in alpha/enabled defaults; the tint
    // and border *colours* are derived from the resolved palette further below
    // (after the colour overrides), so the strip follows the active theme.
    statusbar::barTintAlpha = 0.80;
    statusbar::barBorderAlpha = 0.08;
    statusbar::barBorderEnabled = true;
    statusbar::panelSurfaceAlpha = 1.0;

    // Statusbar geometry resets to compiled-in defaults.
    statusbar::height = 36.0;
    statusbar::topMargin = spacing::large;
    statusbar::sideMargin = spacing::xlarge;
    statusbar::cornerRadius = 12.0;
    statusbar::iconSize = 16.0;
    statusbar::clockIconSize = 16.0;
    statusbar::symbolicIconSize = 18.0;
    statusbar::iconSpacing = 18.0;
    statusbar::padding = 14.0;
    statusbar::separatorWidth = 1.0;
    statusbar::qsPanelWidth = 380.0;
    statusbar::qsTileSize = 110.0;
    statusbar::qsTileHeight = 64.0;
    statusbar::qsTileGap = 8.0;
    statusbar::qsSliderHeight = 40.0;
    statusbar::qsPadding = 16.0;
    statusbar::qsCornerRadius = 16.0;

    // Icon rendering resets to Auto (themed-when-available, else Nerd Font glyph).
    icons::mode = icons::Mode::Auto;

    constexpr const char* s = "theme";

    // ─── Colors ──────────────────────────────────────────────────────
    overrideColor(color::background, cfg, s, "background");
    overrideColor(color::surface, cfg, s, "surface");
    overrideColor(color::surfaceHover, cfg, s, "surface-hover");
    overrideColor(color::glass, cfg, s, "glass");
    overrideColor(color::glassHover, cfg, s, "glass-hover");
    overrideColor(color::glassBorder, cfg, s, "glass-border");
    overrideColor(color::primary, cfg, s, "primary");
    overrideColor(color::primaryGlow, cfg, s, "primary-glow");
    overrideColor(color::text, cfg, s, "text");
    overrideColor(color::textSubtle, cfg, s, "text-subtle");
    overrideColor(color::textMuted, cfg, s, "text-muted");
    overrideColor(color::error, cfg, s, "error");
    overrideColor(color::success, cfg, s, "success");
    overrideColor(color::warning, cfg, s, "warning");

    // ─── Fonts ───────────────────────────────────────────────────────
    if (cfg.has(s, "font-family")) font::family = cfg.getString(s, "font-family", "");
    if (cfg.has(s, "icon-family")) font::iconFamily = cfg.getString(s, "icon-family", "");
    overrideInt(font::size, cfg, s, "font-size");
    overrideInt(font::sizeLarge, cfg, s, "font-size-large");
    overrideInt(font::sizeClock, cfg, s, "font-size-clock");
    overrideInt(font::sizeDate, cfg, s, "font-size-date");

    // ─── Spacing ─────────────────────────────────────────────────────
    overrideInt(spacing::small, cfg, s, "spacing-small");
    overrideInt(spacing::medium, cfg, s, "spacing-medium");
    overrideInt(spacing::large, cfg, s, "spacing-large");
    overrideInt(spacing::xlarge, cfg, s, "spacing-xlarge");

    // ─── Radius ──────────────────────────────────────────────────────
    overrideInt(radius::small, cfg, s, "radius-small");
    overrideInt(radius::medium, cfg, s, "radius-medium");
    overrideInt(radius::large, cfg, s, "radius-large");

    // ─── Animation ───────────────────────────────────────────────────
    overrideInt(anim::fast, cfg, s, "anim-fast");
    overrideInt(anim::medium, cfg, s, "anim-medium");
    overrideInt(anim::slow, cfg, s, "anim-slow");
    overrideInt(anim::reveal, cfg, s, "anim-reveal");

    // ─── Style preset ───────────────────────────────────────────────
    // `style` selects popover rendering only:
    //   "glass"  — frosted translucent cards (default).
    //   "solid"  — opaque cards, same hue, no translucency.
    // The menu-bar backdrop (tint + hairline) is a separate, always-available
    // feature driven by the bar-tint/bar-border keys below; it is no longer tied
    // to a preset. "macos" is kept as a backward-compatible alias for "glass" so
    // an existing config keeps working.
    if (cfg.has(s, "style")) style::mode = cfg.getString(s, "style", "glass");
    if (style::mode == "macos") style::mode = "glass";

    // ─── Icon style ─────────────────────────────────────────────────
    // auto (default) | symbolic | glyph. See theme::icons::Mode.
    if (cfg.has(s, "icon-style")) {
        const std::string m = cfg.getString(s, "icon-style", "auto");
        if (m == "symbolic") icons::mode = icons::Mode::Symbolic;
        else if (m == "glyph" || m == "nerd" || m == "font") icons::mode = icons::Mode::Glyph;
        else icons::mode = icons::Mode::Auto;
    }

    // ─── Effects ─────────────────────────────────────────────────────
    overrideDouble(effects::shadowOpacity, cfg, s, "shadow-opacity");
    overrideInt(effects::shadowOffset, cfg, s, "shadow-offset");

    // ─── Statusbar ───────────────────────────────────────────────────
    overrideDouble(statusbar::height, cfg, s, "bar-height");
    overrideDouble(statusbar::iconSize, cfg, s, "bar-icon-size");
    statusbar::symbolicIconSize = 18.0;
    overrideDouble(statusbar::symbolicIconSize, cfg, s, "bar-symbolic-icon-size");
    statusbar::clockIconSize = statusbar::iconSize;
    overrideDouble(statusbar::clockIconSize, cfg, s, "bar-clock-icon-size");
    overrideDouble(statusbar::iconSpacing, cfg, s, "bar-icon-spacing");
    overrideDouble(statusbar::padding, cfg, s, "bar-padding");
    overrideDouble(statusbar::cornerRadius, cfg, s, "bar-corner-radius");
    overrideDouble(statusbar::popoverRadius, cfg, s, "popover-radius");

    // Menu-bar backdrop: translucent tint + hairline border. The tint and border
    // colours default to the resolved theme palette (background/text) so the strip
    // follows whatever colours are configured; each can be overridden explicitly.
    statusbar::barTint = color::background;
    statusbar::barBorder = color::text;
    overrideColor(statusbar::barTint, cfg, s, "bar-tint");
    overrideDouble(statusbar::barTintAlpha, cfg, s, "bar-tint-alpha");
    overrideColor(statusbar::barBorder, cfg, s, "bar-border-color");
    overrideDouble(statusbar::barBorderAlpha, cfg, s, "bar-border-alpha");
    statusbar::barBorderEnabled = cfg.getBool(s, "bar-border", statusbar::barBorderEnabled);

    // ─── Notification ────────────────────────────────────────────────
    overrideInt(notification::cardWidth, cfg, s, "notification-card-width");
    overrideInt(notification::iconSize, cfg, s, "notification-icon-size");
    overrideInt(notification::padding, cfg, s, "notification-padding");
    overrideInt(notification::gap, cfg, s, "notification-gap");
    overrideInt(notification::titleSize, cfg, s, "notification-title-size");
    overrideInt(notification::bodySize, cfg, s, "notification-body-size");
    overrideInt(notification::radius, cfg, s, "notification-radius");

    // ─── Audio ───────────────────────────────────────────────────────
    overrideInt(audio::buttonSize, cfg, s, "audio-button-size");
    overrideInt(audio::buttonIconSize, cfg, s, "audio-button-icon-size");
    overrideInt(audio::minWidth, cfg, s, "audio-min-width");
    overrideInt(audio::maxWidth, cfg, s, "audio-max-width");
    overrideInt(audio::progressHeight, cfg, s, "audio-progress-height");
    overrideInt(audio::volumeSliderWidth, cfg, s, "audio-volume-slider-width");
}

}  // namespace qypr::theme
