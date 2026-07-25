// QSTile.cpp - Quick Settings tile implementations
#include "ui/statusbar/QSTile.hpp"
#include "render/Painter.hpp"
#include "ui/Theme.hpp"
#include "mpris/MprisController.hpp"
#include <xkbcommon/xkbcommon-keysyms.h>
#include <cmath>

namespace qypr {

namespace {
constexpr double kSliderArrowStep = 0.05;  // ±5%
}  // namespace

// ─── Toggle tile ──────────────────────────────────────────────────────────

void QSToggleTile::onClick(double, double) {
    if (onToggle_) onToggle_();
}

void QSToggleTile::draw(Painter& p, int64_t now) {
    bool active = isActive_ && isActive_();
    double hAlpha = hoverAnim_.value(now);

    Color bg = Color::fromHex("#252336");
    if (hAlpha > 0.01) bg = Color::fromHex("#312e47");

    p.fillRoundedRect(bounds, 12.0, bg);
    p.strokeRoundedRect(bounds, 12.0, Color::fromHex("#383450"), 1.0);

    bool isHorizontal = (title_ == "Do Not Disturb" || bounds.w > 140.0);

    if (isHorizontal) {
        // Horizontal Layout: Left circular badge, Right text
        double badgeR = 16.0;
        double badgeCx = bounds.x + 22.0;
        double badgeCy = bounds.y + bounds.h / 2.0;

        Color badgeBg = active ? Color::fromHex("#38bdf8") : Color::fromHex("#35314a");
        p.fillCircle(badgeCx, badgeCy, badgeR, badgeBg);

        Color iconCol = active ? Color::fromHex("#1e1e2e") : Color::fromHex("#ffffff");
        TextStyle iconStyle{theme::font::iconFamily, 14.0, PANGO_WEIGHT_NORMAL, iconCol};
        Size iconSz = p.measureText(icon_, iconStyle);
        p.drawText(badgeCx - iconSz.w / 2.0, badgeCy - iconSz.h / 2.0, icon_, iconStyle);

        TextStyle titleStyle{theme::font::family, 12.0, PANGO_WEIGHT_BOLD, Color::fromHex("#ffffff")};
        p.drawText(bounds.x + 46.0, bounds.y + (bounds.h - 14.0) / 2.0, title_, titleStyle);
    } else {
        // Vertical Layout: Top circular badge, Bottom text
        double badgeR = 18.0;
        double badgeCx = bounds.x + bounds.w / 2.0;
        double badgeCy = bounds.y + 26.0;

        if (active) {
            if (title_.find("Wired") != std::string::npos || title_.find("WiFi") != std::string::npos || title_.find("Network") != std::string::npos) {
                cairo_t* cr = p.cr();
                cairo_pattern_t* pat = cairo_pattern_create_linear(badgeCx - badgeR, badgeCy - badgeR, badgeCx + badgeR, badgeCy + badgeR);
                cairo_pattern_add_color_stop_rgba(pat, 0.0, 0.0, 0.82, 1.0, 1.0);  // #00d2ff
                cairo_pattern_add_color_stop_rgba(pat, 1.0, 1.0, 0.0, 0.50, 1.0);  // #ff007f
                cairo_arc(cr, badgeCx, badgeCy, badgeR, 0, 2 * M_PI);
                cairo_set_source(cr, pat);
                cairo_fill(cr);
                cairo_pattern_destroy(pat);
            } else if (title_.find("Bluetooth") != std::string::npos || title_.find("BT") != std::string::npos) {
                cairo_t* cr = p.cr();
                cairo_pattern_t* pat = cairo_pattern_create_linear(badgeCx - badgeR, badgeCy - badgeR, badgeCx + badgeR, badgeCy + badgeR);
                cairo_pattern_add_color_stop_rgba(pat, 0.0, 0.0, 0.78, 1.0, 1.0);  // #00c6ff
                cairo_pattern_add_color_stop_rgba(pat, 1.0, 0.0, 0.45, 1.0, 1.0);  // #0072ff
                cairo_arc(cr, badgeCx, badgeCy, badgeR, 0, 2 * M_PI);
                cairo_set_source(cr, pat);
                cairo_fill(cr);
                cairo_pattern_destroy(pat);
            } else if (title_.find("Night") != std::string::npos || title_.find("Dark") != std::string::npos) {
                cairo_t* cr = p.cr();
                cairo_pattern_t* pat = cairo_pattern_create_linear(badgeCx - badgeR, badgeCy - badgeR, badgeCx + badgeR, badgeCy + badgeR);
                cairo_pattern_add_color_stop_rgba(pat, 0.0, 0.96, 0.82, 0.40, 1.0);  // #f6d365
                cairo_pattern_add_color_stop_rgba(pat, 1.0, 0.99, 0.63, 0.52, 1.0);  // #fda085
                cairo_arc(cr, badgeCx, badgeCy, badgeR, 0, 2 * M_PI);
                cairo_set_source(cr, pat);
                cairo_fill(cr);
                cairo_pattern_destroy(pat);
            } else if (title_.find("Keep") != std::string::npos || title_.find("Idle") != std::string::npos || title_.find("Awake") != std::string::npos) {
                cairo_t* cr = p.cr();
                cairo_pattern_t* pat = cairo_pattern_create_linear(badgeCx - badgeR, badgeCy - badgeR, badgeCx + badgeR, badgeCy + badgeR);
                cairo_pattern_add_color_stop_rgba(pat, 0.0, 0.26, 0.91, 0.48, 1.0);  // #43e97b
                cairo_pattern_add_color_stop_rgba(pat, 1.0, 0.22, 0.98, 0.84, 1.0);  // #38f9d7
                cairo_arc(cr, badgeCx, badgeCy, badgeR, 0, 2 * M_PI);
                cairo_set_source(cr, pat);
                cairo_fill(cr);
                cairo_pattern_destroy(pat);
            } else if (title_.find("Screenshot") != std::string::npos) {
                cairo_t* cr = p.cr();
                cairo_pattern_t* pat = cairo_pattern_create_linear(badgeCx - badgeR, badgeCy - badgeR, badgeCx + badgeR, badgeCy + badgeR);
                cairo_pattern_add_color_stop_rgba(pat, 0.0, 1.0, 0.49, 0.37, 1.0);  // #ff7e5f
                cairo_pattern_add_color_stop_rgba(pat, 1.0, 1.0, 0.71, 0.48, 1.0);  // #feb47b
                cairo_arc(cr, badgeCx, badgeCy, badgeR, 0, 2 * M_PI);
                cairo_set_source(cr, pat);
                cairo_fill(cr);
                cairo_pattern_destroy(pat);
            } else {
                p.fillCircle(badgeCx, badgeCy, badgeR, Color::fromHex("#38bdf8"));
            }
        } else {
            p.fillCircle(badgeCx, badgeCy, badgeR, Color::fromHex("#35314a"));
        }

        Color iconCol = active ? Color::fromHex("#ffffff") : Color::fromHex("#a6accd");
        TextStyle iconStyle{theme::font::iconFamily, 16.0, PANGO_WEIGHT_NORMAL, iconCol};
        Size iconSz = p.measureText(icon_, iconStyle);
        p.drawText(badgeCx - iconSz.w / 2.0, badgeCy - iconSz.h / 2.0, icon_, iconStyle);

        std::string sub = subtitle_ ? subtitle_() : "";
        std::string labelText = sub.empty() ? title_ : sub;

        TextStyle labelStyle{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL,
                             active ? Color::fromHex("#ffffff") : Color::fromHex("#a6accd")};
        p.drawText(badgeCx, bounds.y + 50.0, labelText, labelStyle, HAlign::Center, bounds.w - 8.0);
    }
}

// ─── Slider tile ──────────────────────────────────────────────────────────

void QSSliderTile::onClick(double x, double y) {
    if (onIconClick_ && iconBounds_.contains(x, y)) {
        onIconClick_();
        return;
    }
    updateValueFromCoord(x);
}

void QSSliderTile::onDrag(double x, double y) {
    if (onIconClick_ && iconBounds_.contains(x, y)) return;
    updateValueFromCoord(x);
}

bool QSSliderTile::handleKey(uint32_t keysym) {
    switch (keysym) {
        case XKB_KEY_Up: case XKB_KEY_Right: return stepValue(true);
        case XKB_KEY_Down: case XKB_KEY_Left: return stepValue(false);
    }
    return false;
}

bool QSSliderTile::stepValue(bool up) {
    if (!getValue_ || !onValueChange_) return false;
    const double cur = getValue_();
    const double next = clamp01(cur + (up ? kSliderArrowStep : -kSliderArrowStep));
    if (std::abs(next - cur) < 1e-9) return false;
    onValueChange_(next);
    return true;
}

void QSSliderTile::updateValueFromCoord(double x) {
    if (sliderTrackBounds_.w <= 0) return;
    double val = clamp01((x - sliderTrackBounds_.x) / sliderTrackBounds_.w);
    if (onValueChange_) onValueChange_(val);
}

void QSSliderTile::draw(Painter& p, int64_t now) {
    double val = getValue_ ? getValue_() : 0.8;

    p.fillRoundedRect(bounds, 12.0, Color::fromHex("#252336"));
    p.strokeRoundedRect(bounds, 12.0, Color::fromHex("#383450"), 1.0);

    if (bounds.h <= 80.0) {
        // Grid slot card: Monitor/Control Name top, Icon + Slider bottom
        TextStyle titleStyle{theme::font::family, 12.0, PANGO_WEIGHT_BOLD, Color::fromHex("#ffffff")};
        std::string label = title_.empty() ? "Q27G41ZDF" : title_;
        p.drawText(bounds.x + 10.0, bounds.y + 10.0, label, titleStyle);

        // Icon
        std::string glyph = currentIcon();
        if (glyph.empty()) glyph = "󰃟";
        TextStyle iconStyle{theme::font::iconFamily, 16.0, PANGO_WEIGHT_NORMAL, Color::fromHex("#80f5a7")};
        Size iconSz = p.measureText(glyph, iconStyle);
        double iconX = bounds.x + 10.0;
        double iconY = bounds.y + 42.0;
        p.drawText(iconX, iconY, glyph, iconStyle);
        iconBounds_ = {bounds.x, bounds.y + 36.0, 30.0, 36.0};

        // Slider track
        double trackX = iconX + iconSz.w + 8.0;
        double trackW = bounds.w - (trackX - bounds.x) - 10.0;
        double trackH = 8.0;
        double trackY = iconY + (iconSz.h - trackH) / 2.0;
        sliderTrackBounds_ = {trackX, trackY, trackW, trackH};

        p.fillRoundedRect(sliderTrackBounds_, trackH / 2.0, Color::fromHex("#35314a"));
        Rect filled{trackX, trackY, trackW * val, trackH};
        p.fillRoundedRect(filled, trackH / 2.0, Color::fromHex("#38bdf8"));
        p.fillCircle(trackX + trackW * val, trackY + trackH / 2.0, 5.0, Color::fromHex("#ffffff"));
    } else {
        // Full width slider row
        double pad = 12.0;
        const std::string glyph = currentIcon();
        TextStyle iconStyle{theme::font::iconFamily, 16.0, PANGO_WEIGHT_NORMAL, Color::fromHex("#38bdf8")};
        Size iconSz = p.measureText(glyph, iconStyle);

        double iconX = bounds.x + pad;
        double iconY = bounds.y + (bounds.h - iconSz.h) / 2.0;
        p.drawText(iconX, iconY, glyph, iconStyle);
        iconBounds_ = {bounds.x, bounds.y, iconX + iconSz.w + 6.0 - bounds.x, bounds.h};

        double trackX = iconX + iconSz.w + 12.0;
        double trackW = bounds.w - pad - trackX;
        double trackH = 8.0;
        double trackY = bounds.y + (bounds.h - trackH) / 2.0;
        sliderTrackBounds_ = {trackX, trackY, trackW, trackH};

        p.fillRoundedRect(sliderTrackBounds_, trackH / 2.0, Color::fromHex("#35314a"));
        Rect filled{trackX, trackY, trackW * val, trackH};
        p.fillRoundedRect(filled, trackH / 2.0, Color::fromHex("#38bdf8"));
        p.fillCircle(trackX + trackW * val, trackY + trackH / 2.0, 6.0, Color::fromHex("#ffffff"));
    }
}

// ─── Info tile ────────────────────────────────────────────────────────────

void QSInfoTile::draw(Painter& p, int64_t now) {
    double progress = getProgress_ ? getProgress_() : 0.0;
    std::string info = getInfo_ ? getInfo_() : "";

    p.fillRoundedRect(bounds, 12.0, Color::fromHex("#252336"));
    p.strokeRoundedRect(bounds, 12.0, Color::fromHex("#383450"), 1.0);

    double pad = 12.0;

    TextStyle iconStyle{theme::font::iconFamily, 18.0, PANGO_WEIGHT_NORMAL, Color::fromHex("#38bdf8")};
    Size iconSz = p.measureText(icon_, iconStyle);
    double iconX = bounds.x + pad;
    double iconY = bounds.y + pad;
    p.drawText(iconX, iconY, icon_, iconStyle);

    TextStyle titleStyle{theme::font::family, 13.0, PANGO_WEIGHT_BOLD, Color::fromHex("#ffffff")};
    p.drawText(iconX + iconSz.w + 10.0, iconY, title_, titleStyle);

    double barY = bounds.y + pad + iconSz.h + 8.0;
    double barH = 6.0;
    double barW = bounds.w - 2 * pad;
    p.fillRoundedRect({bounds.x + pad, barY, barW, barH}, barH / 2.0, Color::fromHex("#35314a"));

    Rect barFill{bounds.x + pad, barY, barW * clamp01(progress), barH};
    Color barCol = Color::fromHex("#38bdf8");
    p.fillRoundedRect(barFill, barH / 2.0, barCol);

    if (!info.empty()) {
        TextStyle infoStyle{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL, Color::fromHex("#a6accd")};
        p.drawText(bounds.x + pad, barY + barH + 6.0, info, infoStyle);
    }
}

// ─── Header tile ──────────────────────────────────────────────────────────

void QSHeaderTile::draw(Painter& p, int64_t) {
    p.fillRoundedRect(bounds, 12.0, Color::fromHex("#252336"));
    p.strokeRoundedRect(bounds, 12.0, Color::fromHex("#383450"), 1.0);

    // Avatar circle
    double avatarR = 15.0;
    double avatarCX = bounds.x + 24.0;
    double avatarCY = bounds.y + bounds.h / 2.0;

    // Outer ring outline
    p.strokeCircle(avatarCX, avatarCY, avatarR, Color::fromHex("#fabd2f"), 2.0);
    p.fillCircle(avatarCX, avatarCY, avatarR - 1.0, Color::fromHex("#1e1e2e"));

    // User icon inside circle
    TextStyle initStyle{theme::font::iconFamily, 14.0, PANGO_WEIGHT_NORMAL, Color::fromHex("#fabd2f")};
    Size initSz = p.measureText("󰀉", initStyle);
    if (initSz.w == 0 || initSz.h == 0) {
        std::string initial = title_.empty() ? "B" : title_.substr(0, 1);
        initStyle = {theme::font::family, 14.0, PANGO_WEIGHT_BOLD, Color::fromHex("#fabd2f")};
        initSz = p.measureText(initial, initStyle);
        p.drawText(avatarCX - initSz.w / 2.0, avatarCY - initSz.h / 2.0, initial, initStyle);
    } else {
        p.drawText(avatarCX - initSz.w / 2.0, avatarCY - initSz.h / 2.0, "󰀉", initStyle);
    }

    // Name + subtitle
    double textX = avatarCX + avatarR + 12.0;
    TextStyle nameStyle{theme::font::family, 13.0, PANGO_WEIGHT_BOLD, Color::fromHex("#ffffff")};
    TextStyle subStyle{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL, Color::fromHex("#a6accd")};

    Size nameSz = p.measureText(title_, nameStyle);
    Size subSz = p.measureText(subtitle_, subStyle);
    double startY = bounds.y + (bounds.h - (nameSz.h + subSz.h + 2.0)) / 2.0;
    p.drawText(textX, startY, title_, nameStyle);
    p.drawText(textX, startY + nameSz.h + 2.0, subtitle_, subStyle);
}

// ─── Power button ─────────────────────────────────────────────────────────

void QSPowerTile::onClick(double, double) {
    if (onClick_) onClick_();
}

void QSPowerTile::draw(Painter& p, int64_t now) {
    p.fillRoundedRect(bounds, 12.0, Color::fromHex("#252336"));
    p.strokeRoundedRect(bounds, 12.0, Color::fromHex("#383450"), 1.0);

    double cx = bounds.x + bounds.w / 2.0;
    double cy = bounds.y + bounds.h / 2.0;

    const char* glyph = "⏻";
    TextStyle iconStyle{theme::font::iconFamily, 18.0, PANGO_WEIGHT_NORMAL, Color::fromHex("#ffffff")};
    Size iconSz = p.measureText(glyph, iconStyle);
    p.drawText(cx - iconSz.w / 2.0, cy - iconSz.h / 2.0, glyph, iconStyle);
}

// ─── Wi-Fi combo tile ─────────────────────────────────────────────────────

void QSWifiComboTile::draw(Painter& p, int64_t now) {
    double hAlpha = hoverAnim_.value(now);
    Color bg = Color::fromHex("#252336");
    if (hAlpha > 0.01) bg = Color::fromHex("#312e47");

    p.fillRoundedRect(bounds, 12.0, bg);
    p.strokeRoundedRect(bounds, 12.0, Color::fromHex("#383450"), 1.0);

    // Top center circular icon badge
    double badgeR = 18.0;
    double badgeCx = bounds.x + bounds.w / 2.0;
    double badgeCy = bounds.y + 26.0;

    if (enabled_) {
        cairo_t* cr = p.cr();
        cairo_pattern_t* pat = cairo_pattern_create_linear(badgeCx - badgeR, badgeCy - badgeR, badgeCx + badgeR, badgeCy + badgeR);
        cairo_pattern_add_color_stop_rgba(pat, 0.0, 0.0, 0.82, 1.0, 1.0);  // #00d2ff
        cairo_pattern_add_color_stop_rgba(pat, 1.0, 1.0, 0.0, 0.50, 1.0);  // #ff007f
        cairo_arc(cr, badgeCx, badgeCy, badgeR, 0, 2 * M_PI);
        cairo_set_source(cr, pat);
        cairo_fill(cr);
        cairo_pattern_destroy(pat);
    } else {
        p.fillCircle(badgeCx, badgeCy, badgeR, Color::fromHex("#35314a"));
    }

    const char* wifiGlyph = enabled_ ? "󰈀" : "󰤯";
    TextStyle iconStyle{theme::font::iconFamily, 16.0, PANGO_WEIGHT_NORMAL, Color::fromHex("#ffffff")};
    Size iconSz = p.measureText(wifiGlyph, iconStyle);
    p.drawText(badgeCx - iconSz.w / 2.0, badgeCy - iconSz.h / 2.0, wifiGlyph, iconStyle);

    std::string text = ssid_.empty() ? (enabled_ ? "Wired connection" : "Off") : ssid_;
    TextStyle ssidStyle{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL,
                        enabled_ ? Color::fromHex("#ffffff") : Color::fromHex("#a6accd")};
    p.drawText(badgeCx, bounds.y + 50.0, text, ssidStyle, HAlign::Center, bounds.w - 8.0);
}

// ─── Volume section tile ──────────────────────────────────────────────────

QSVolumeTile::QSVolumeTile(std::function<double()> getValue,
                           std::function<void(double)> onChange,
                           std::function<std::string()> dynamicIcon,
                           std::function<void()> onIconClick,
                           std::function<bool()> dimmed)
    : getValue_(std::move(getValue)), onValueChange_(std::move(onChange)),
      dynamicIcon_(std::move(dynamicIcon)), onIconClick_(std::move(onIconClick)),
      dimmed_(std::move(dimmed)) {}

void QSVolumeTile::onClick(double x, double y) {
    if (onIconClick_ && iconBounds_.contains(x, y)) {
        onIconClick_();
        return;
    }
    updateValueFromCoord(x);
}

void QSVolumeTile::onDrag(double x, double y) {
    if (onIconClick_ && iconBounds_.contains(x, y)) return;
    updateValueFromCoord(x);
}

bool QSVolumeTile::handleKey(uint32_t keysym) {
    switch (keysym) {
        case XKB_KEY_Up: case XKB_KEY_Right: return stepValue(true);
        case XKB_KEY_Down: case XKB_KEY_Left: return stepValue(false);
    }
    return false;
}

bool QSVolumeTile::stepValue(bool up) {
    if (!getValue_ || !onValueChange_) return false;
    const double cur = getValue_();
    const double next = clamp01(cur + (up ? kSliderArrowStep : -kSliderArrowStep));
    if (std::abs(next - cur) < 1e-9) return false;
    onValueChange_(next);
    return true;
}

void QSVolumeTile::updateValueFromCoord(double x) {
    if (trackBounds_.w <= 0) return;
    double val = clamp01((x - trackBounds_.x) / trackBounds_.w);
    if (onValueChange_) onValueChange_(val);
}

void QSVolumeTile::draw(Painter& p, int64_t) {
    double val = getValue_ ? getValue_() : 0.7;

    p.fillRoundedRect(bounds, 14.0, Color::fromHex("#252336"));
    p.strokeRoundedRect(bounds, 14.0, Color::fromHex("#383450"), 1.0);

    double pad = 14.0;

    // Header title
    TextStyle titleStyle{theme::font::family, 13.0, PANGO_WEIGHT_BOLD, Color::fromHex("#ffffff")};
    p.drawText(bounds.x + pad, bounds.y + 10.0, "Volume", titleStyle);

    // Speaker icon
    const std::string glyph = currentIcon();
    TextStyle iconStyle{theme::font::iconFamily, 16.0, PANGO_WEIGHT_NORMAL, Color::fromHex("#38bdf8")};
    Size iconSz = p.measureText(glyph, iconStyle);
    double iconX = bounds.x + pad;
    double iconY = bounds.y + 36.0;
    p.drawText(iconX, iconY, glyph, iconStyle);
    iconBounds_ = {bounds.x, iconY - 4.0, iconX + iconSz.w + 6.0 - bounds.x, iconSz.h + 8.0};

    // Right chevron arrow button `>`
    TextStyle arrStyle{theme::font::family, 16.0, PANGO_WEIGHT_BOLD, Color::fromHex("#ff5e62")};
    Size arrSz = p.measureText(">", arrStyle);
    double arrX = bounds.x + bounds.w - pad - arrSz.w;
    double arrY = iconY + (iconSz.h - arrSz.h) / 2.0;
    p.drawText(arrX, arrY, ">", arrStyle);

    // Volume track
    double trackX = iconX + iconSz.w + 12.0;
    double trackW = arrX - 14.0 - trackX;
    double trackH = 10.0;
    double trackY = iconY + (iconSz.h - trackH) / 2.0;
    trackBounds_ = {trackX, trackY, trackW, trackH};

    p.fillRoundedRect(trackBounds_, trackH / 2.0, Color::fromHex("#35314a"));
    Rect filled{trackX, trackY, trackW * val, trackH};
    p.fillRoundedRect(filled, trackH / 2.0, Color::fromHex("#38bdf8"));
    p.fillCircle(trackX + trackW * val, trackY + trackH / 2.0, 6.0, Color::fromHex("#ffffff"));
}

// ─── Media card ───────────────────────────────────────────────────────────

void QSMediaTile::onClick(double x, double y) {
    if (!mpris_ || !mpris_->active()) return;
    if (prevBounds_.contains(x, y))   mpris_->previous();
    else if (playBounds_.contains(x, y)) mpris_->togglePlaying();
    else if (nextBounds_.contains(x, y)) mpris_->next();
}

void QSMediaTile::draw(Painter& p, int64_t) {
    p.fillRoundedRect(bounds, 14.0, Color::fromHex("#252336"));
    p.strokeRoundedRect(bounds, 14.0, Color::fromHex("#383450"), 1.0);

    double pad = 14.0;

    // Left musical note icon
    TextStyle noteStyle{theme::font::iconFamily, 22.0, PANGO_WEIGHT_NORMAL, Color::fromHex("#38bdf8")};
    Size noteSz = p.measureText("󰎈", noteStyle);
    double noteX = bounds.x + pad;
    double noteY = bounds.y + (bounds.h - noteSz.h) / 2.0;
    p.drawText(noteX, noteY, "󰎈", noteStyle);

    // Track title / state
    std::string titleStr = "No Media Playing";
    if (mpris_ && mpris_->active()) {
        titleStr = mpris_->title();
    }

    TextStyle titleStyle{theme::font::family, 12.0, PANGO_WEIGHT_BOLD, Color::fromHex("#ffffff")};
    double textX = noteX + noteSz.w + 14.0;
    double textW = bounds.w - textX - 85.0;
    p.drawText(textX, bounds.y + (bounds.h - 16.0) / 2.0, titleStr, titleStyle, HAlign::Left, textW);

    // Transport buttons (right-aligned)
    double btnY = bounds.y + (bounds.h - 20.0) / 2.0;
    double btnX = bounds.x + bounds.w - pad - 65.0;
    TextStyle ctrlStyle{theme::font::iconFamily, 14.0, PANGO_WEIGHT_NORMAL, Color::fromHex("#6c6f93")};
    prevBounds_ = {btnX, btnY, 18.0, 20.0};
    playBounds_ = {btnX + 22.0, btnY, 18.0, 20.0};
    nextBounds_ = {btnX + 44.0, btnY, 18.0, 20.0};
    p.drawText(btnX, btnY, "󰒮", ctrlStyle);
    p.drawText(btnX + 22.0, btnY, "󰐊", ctrlStyle);
    p.drawText(btnX + 44.0, btnY, "󰒭", ctrlStyle);
}

}  // namespace qypr
