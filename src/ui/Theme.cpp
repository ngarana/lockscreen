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

    // ─── Effects ─────────────────────────────────────────────────────
    overrideDouble(effects::shadowOpacity, cfg, s, "shadow-opacity");
    overrideInt(effects::shadowOffset, cfg, s, "shadow-offset");

    // ─── Statusbar ───────────────────────────────────────────────────
    overrideDouble(statusbar::height, cfg, s, "bar-height");
    overrideDouble(statusbar::iconSize, cfg, s, "bar-icon-size");
    overrideDouble(statusbar::iconSpacing, cfg, s, "bar-icon-spacing");
    overrideDouble(statusbar::padding, cfg, s, "bar-padding");
    overrideDouble(statusbar::cornerRadius, cfg, s, "bar-corner-radius");
    overrideDouble(statusbar::popoverRadius, cfg, s, "popover-radius");

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
