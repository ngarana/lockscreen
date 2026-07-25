// StatusIndicator.cpp - Base status bar applet implementation
#include "ui/statusbar/StatusIndicator.hpp"
#include "render/Painter.hpp"
#include "ui/Theme.hpp"

namespace qypr {

namespace {
constexpr double kContentGap = 8.0;   // between icon and label
constexpr double kSidePad = 8.0;      // hover-zone padding each side
}  // namespace

Color StatusIndicator::iconColor() const {
    return theme::color::text;
}

double StatusIndicator::measureWidth(Painter& p) {
    const std::string ic = icon();
    const std::string lbl = label();
    double w = 0;

    if (!ic.empty()) {
        TextStyle iconStyle{theme::font::iconFamily, theme::statusbar::iconSize,
                            PANGO_WEIGHT_NORMAL, iconColor()};
        w += p.measureText(ic, iconStyle).w;
    }
    if (!lbl.empty()) {
        TextStyle labelStyle{theme::font::family, labelFontSize(), PANGO_WEIGHT_NORMAL,
                             iconColor()};
        if (w > 0) w += kContentGap;
        w += p.measureText(lbl, labelStyle).w;
    }
    return w + 2 * kSidePad;
}

void StatusIndicator::draw(Painter& p, int64_t now) {
    if (!visible) return;

    double alpha = hoverAlpha_.value(now);
    double scale = hoverScale_.value(now);

    // 1. Hover background pill (subtle surface highlight inside the chip)
    if (alpha > 0.01) {
        Color bg = theme::color::surfaceHover.withAlpha(alpha * 0.5);
        Rect hoverRect = bounds;
        hoverRect.y += 2.0;
        hoverRect.h -= 4.0;
        p.fillRoundedRect(hoverRect, 8.0, bg);
    }

    // 2. Focus ring
    if (focused) {
        Rect focusRect = bounds;
        focusRect.y += 1.0;
        focusRect.h -= 2.0;
        p.strokeRoundedRect(focusRect, 8.0, theme::color::primary, 1.5);
    }

    // 3. Content: optional icon + optional label, centered in bounds
    const std::string ic = icon();
    const std::string lbl = label();

    // Phase 6 polish: if the icon glyph has changed since the last frame, kick
    // off a 300ms ease-in-out crossfade. The outgoing glyph rerenders under the
    // incoming one at shrinking alpha; both share the new footprint. Battery
    // level changes and WiFi signal tiers go through this path for free.
    if (!ic.empty() && ic != lastDrawnIcon_) {
        if (!lastDrawnIcon_.empty() && prevDrawnIcon_.empty()) {
            prevDrawnIcon_ = lastDrawnIcon_;
            crossfadeStartMs_ = now;
        }
        lastDrawnIcon_ = ic;
    }
    double crossfadeT = 1.0;
    if (!prevDrawnIcon_.empty()) {
        crossfadeT = clamp01(static_cast<double>(now - crossfadeStartMs_) /
                             static_cast<double>(kCrossfadeMs));
        if (crossfadeT >= 1.0) prevDrawnIcon_.clear();
    }

    TextStyle iconStyle{theme::font::iconFamily, theme::statusbar::iconSize,
                        PANGO_WEIGHT_NORMAL, iconColor()};
    TextStyle labelStyle{theme::font::family, labelFontSize(), PANGO_WEIGHT_NORMAL,
                         iconColor()};
    if (scale > 1.001) {
        iconStyle.size *= scale;
        labelStyle.size *= scale;
    }

    Size iconSz{}, labelSz{};
    double contentW = 0;
    if (!ic.empty()) {
        iconSz = p.measureText(ic, iconStyle);
        contentW += iconSz.w;
    }
    if (!lbl.empty()) {
        labelSz = p.measureText(lbl, labelStyle);
        if (contentW > 0) contentW += kContentGap;
        contentW += labelSz.w;
    }

    // Shadowed like the lockscreen clock: the bar has no background of its
    // own, so text must stay readable straight over the video.
    const double shadowA = theme::effects::shadowOpacity;
    const double shadowOff = theme::effects::shadowOffset;
    double x = bounds.x + (bounds.w - contentW) / 2.0;
    if (!ic.empty()) {
        p.drawTextShadowed(x, bounds.y + (bounds.h - iconSz.h) / 2.0, ic, iconStyle,
                           HAlign::Left, shadowA, shadowOff);
        //during the swap, layer the outgoing glyph at shrinking alpha so the
        // two dissolve rather than blink.
        if (!prevDrawnIcon_.empty() && crossfadeT < 1.0) {
            double t = ease::inOutQuad(crossfadeT);
            TextStyle prevStyle = iconStyle;
            prevStyle.color = iconColor().withAlpha((1.0 - t) * iconColor().a);
            p.drawText(x, bounds.y + (bounds.h - iconSz.h) / 2.0, prevDrawnIcon_, prevStyle);
        }
        x += iconSz.w + (lbl.empty() ? 0.0 : kContentGap);
    }
    if (!lbl.empty()) {
        p.drawTextShadowed(x, bounds.y + (bounds.h - labelSz.h) / 2.0, lbl, labelStyle,
                           HAlign::Left, shadowA, shadowOff);
    }
}

bool StatusIndicator::animating(int64_t now) const {
    return hoverAlpha_.active(now) || hoverScale_.active(now) || !prevDrawnIcon_.empty();
}

}  // namespace qypr
