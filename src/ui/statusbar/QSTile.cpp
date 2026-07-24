// QSTile.cpp - Quick Settings tile implementations
#include "ui/statusbar/QSTile.hpp"
#include "render/Painter.hpp"
#include "ui/Theme.hpp"
#include <xkbcommon/xkbcommon-keysyms.h>
#include <cmath>

namespace qypr {

// ─────────────────────────────────────────────────────────────────────────
// Slider keyboard fine-adjust step. The spec calls out ±5%; kept as a
// constant so callers and tests share the exact value.
// ─────────────────────────────────────────────────────────────────────────
namespace {
constexpr double kSliderArrowStep = 0.05;  // ±5%
}  // namespace

// --- QSToggleTile ---

void QSToggleTile::onClick(double, double) {
    if (onToggle_) {
        onToggle_();
    }
}

void QSToggleTile::draw(Painter& p, int64_t now) {
    bool active = isActive_ && isActive_();
    double hAlpha = hoverAnim_.value(now);

    // Dynamic backgrounds: filled surface tiles, no outline stroke.
    Color bg = active ? theme::color::primary : theme::color::surface;
    if (hAlpha > 0.01) {
        bg = active ? theme::color::primary.withAlpha(0.85)
                    : theme::color::surfaceHover;
    }

    p.fillRoundedRect(bounds, 12.0, bg);

    // Draw content
    Color fgColor = active ? Color::fromHex("#1e1e2e") : theme::color::text;
    Color subColor = active ? Color::fromHex("#313244") : theme::color::textSubtle;

    // Draw icon on the left
    TextStyle iconStyle{theme::font::iconFamily, 20.0, PANGO_WEIGHT_NORMAL, fgColor};
    Size iconSz = p.measureText(icon_, iconStyle);

    double contentPad = 12.0;
    double iconX = bounds.x + contentPad;
    double iconY = bounds.y + (bounds.h - iconSz.h) / 2.0;
    p.drawText(iconX, iconY, icon_, iconStyle);

    // Draw text on the right
    double textX = iconX + iconSz.w + 10.0;
    std::string sub = subtitle_ ? subtitle_() : "";

    TextStyle titleStyle{theme::font::family, 13.0, PANGO_WEIGHT_BOLD, fgColor};
    TextStyle subStyle{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL, subColor};

    if (sub.empty()) {
        Size titleSz = p.measureText(title_, titleStyle);
        p.drawText(textX, bounds.y + (bounds.h - titleSz.h) / 2.0, title_, titleStyle);
    } else {
        Size titleSz = p.measureText(title_, titleStyle);
        Size subSz = p.measureText(sub, subStyle);
        double totalTextH = titleSz.h + subSz.h + 2.0;

        double titleY = bounds.y + (bounds.h - totalTextH) / 2.0;
        p.drawText(textX, titleY, title_, titleStyle);
        p.drawText(textX, titleY + titleSz.h + 2.0, sub, subStyle);
    }
}

// --- QSSliderTile ---

void QSSliderTile::onClick(double x, double y) {
    // The icon is a button when the owner gave us one (mute); everything else
    // on the row adjusts the value.
    if (onIconClick_ && iconBounds_.contains(x, y)) {
        onIconClick_();
        return;
    }
    updateValueFromCoord(x);
}

void QSSliderTile::onDrag(double x, double y) {
    // Never let a drag that began on the icon scrub the value.
    if (onIconClick_ && iconBounds_.contains(x, y)) return;
    updateValueFromCoord(x);
}

bool QSSliderTile::handleKey(uint32_t keysym) {
    if (dimmed_ && dimmed_()) {
        // While muted we still let arrows adjust the level underneath (so
        // the next unmute is already at a comfortable setting), matching the
        // drag-on-track behaviour.
    }
    switch (keysym) {
        case XKB_KEY_Up:
        case XKB_KEY_Right:
            return stepValue(true);
        case XKB_KEY_Down:
        case XKB_KEY_Left:
            return stepValue(false);
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
    double val = (x - sliderTrackBounds_.x) / sliderTrackBounds_.w;
    val = clamp01(val);
    if (onValueChange_) {
        onValueChange_(val);
    }
}

void QSSliderTile::draw(Painter& p, int64_t now) {
    double val = getValue_ ? getValue_() : 0.0;
    // Muted: grey the whole row and flatten the fill, so a glance reads "off"
    // even though the level underneath is preserved.
    const bool dim = dimmed_ && dimmed_();
    const Color iconColor = dim ? theme::color::textSubtle : theme::color::text;
    const Color fillColor = dim ? theme::color::textSubtle : theme::color::primary;

    // Draw icon on the left. When the owner supplies a dynamic glyph (mute) it
    // wins, and the icon doubles as a button — record its hit box for onClick.
    const std::string glyph = currentIcon();
    TextStyle iconStyle{theme::font::iconFamily, 18.0, PANGO_WEIGHT_NORMAL, iconColor};
    Size iconSz = p.measureText(glyph, iconStyle);

    double iconX = bounds.x + 8.0;
    double iconY = bounds.y + (bounds.h - iconSz.h) / 2.0;
    p.drawText(iconX, iconY, glyph, iconStyle);
    // Pad the click target vertically so it is comfortable to hit.
    iconBounds_ = {bounds.x, bounds.y, iconX + iconSz.w + 6.0 - bounds.x, bounds.h};

    // Draw percentage text on the right
    int percent = static_cast<int>(std::round(val * 100.0));
    std::string pctText = std::to_string(percent) + "%";

    TextStyle pctStyle{theme::font::family, 13.0, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
    Size pctSz = p.measureText(pctText, pctStyle);

    double pctX = bounds.x + bounds.w - pctSz.w - 8.0;
    double pctY = bounds.y + (bounds.h - pctSz.h) / 2.0;
    p.drawText(pctX, pctY, pctText, pctStyle);

    // Draw track in the middle
    double trackX = iconX + iconSz.w + 12.0;
    double trackW = pctX - 12.0 - trackX;
    double trackH = 4.0;
    double trackY = bounds.y + (bounds.h - trackH) / 2.0;

    sliderTrackBounds_ = {trackX, trackY, trackW, trackH};

    // Draw track background
    p.fillRoundedRect(sliderTrackBounds_, trackH / 2.0, theme::color::surface);

    // Draw filled track
    Rect filledBounds = sliderTrackBounds_;
    filledBounds.w = trackW * val;
    p.fillRoundedRect(filledBounds, trackH / 2.0, fillColor);

    // Draw thumb dot
    double thumbRadius = 6.0;
    double thumbX = trackX + trackW * val;
    double thumbY = trackY + trackH / 2.0;
    p.fillCircle(thumbX, thumbY, thumbRadius, fillColor);
}

// --- QSInfoTile ---

void QSInfoTile::draw(Painter& p, int64_t now) {
    double progress = getProgress_ ? getProgress_() : 0.0;
    std::string info = getInfo_ ? getInfo_() : "";

    // Background: filled surface tile, no outline.
    p.fillRoundedRect(bounds, 12.0, theme::color::surface);

    double pad = 12.0;

    // Icon
    TextStyle iconStyle{theme::font::iconFamily, 18.0, PANGO_WEIGHT_NORMAL, theme::color::text};
    Size iconSz = p.measureText(icon_, iconStyle);
    double iconX = bounds.x + pad;
    double iconY = bounds.y + pad;
    p.drawText(iconX, iconY, icon_, iconStyle);

    // Title (top-right of icon)
    TextStyle titleStyle{theme::font::family, 13.0, PANGO_WEIGHT_BOLD, theme::color::text};
    p.drawText(iconX + iconSz.w + 10.0, iconY, title_, titleStyle);

    // Progress bar
    double barY = bounds.y + pad + iconSz.h + 8.0;
    double barH = 6.0;
    double barW = bounds.w - 2 * pad;
    Rect barBg{bounds.x + pad, barY, barW, barH};
    p.fillRoundedRect(barBg, barH / 2.0, theme::color::surface);

    Rect barFill{bounds.x + pad, barY, barW * clamp01(progress), barH};
    Color barColor = progress > 0.5 ? theme::color::success
                   : progress > 0.2 ? theme::color::warning
                   : theme::color::error;
    p.fillRoundedRect(barFill, barH / 2.0, barColor);

    // Info text below bar
    if (!info.empty()) {
        TextStyle infoStyle{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
        p.drawText(bounds.x + pad, barY + barH + 6.0, info, infoStyle);
    }
}

}  // namespace qypr
