// StatusIndicator.cpp - Base status bar applet implementation
#include "ui/statusbar/StatusIndicator.hpp"
#include "render/Painter.hpp"
#include "ui/Theme.hpp"

namespace qypr {

Color StatusIndicator::iconColor() const {
    return theme::color::text;
}

double StatusIndicator::measureWidth(Painter& p) {
    TextStyle iconStyle{theme::font::iconFamily, theme::statusbar::iconSize, PANGO_WEIGHT_NORMAL, iconColor()};
    Size iconSize = p.measureText(icon(), iconStyle);

    double w = iconSize.w;

    std::string lbl = label();
    if (!lbl.empty()) {
        TextStyle labelStyle{theme::font::family, 14.0, PANGO_WEIGHT_NORMAL, iconColor()};
        Size labelSize = p.measureText(lbl, labelStyle);
        w += 8.0 + labelSize.w; // 8px gap
    }

    return w + 16.0; // 8px padding on each side for hover zone
}

void StatusIndicator::draw(Painter& p, int64_t now) {
    if (!visible) return;

    double alpha = hoverAlpha_.value(now);
    double scale = hoverScale_.value(now);

    // 1. Draw hover/focus background pill
    if (alpha > 0.01) {
        Color bg = theme::color::glassHover.withAlpha(alpha * theme::color::glassHover.a);
        Rect hoverRect = bounds;
        // Shrink slightly vertically for a cleaner look
        hoverRect.y += 2.0;
        hoverRect.h -= 4.0;
        p.fillRoundedRect(hoverRect, 8.0, bg);
    }

    // 2. Draw focus ring
    if (focused) {
        Rect focusRect = bounds;
        focusRect.y += 1.0;
        focusRect.h -= 2.0;
        p.strokeRoundedRect(focusRect, 8.0, theme::color::primary, 1.5);
    }

    // 3. Draw content (icon + label)
    TextStyle iconStyle{theme::font::iconFamily, theme::statusbar::iconSize, PANGO_WEIGHT_NORMAL, iconColor()};
    Size iconSz = p.measureText(icon(), iconStyle);

    double contentW = iconSz.w;
    std::string lbl = label();
    Size labelSz;
    TextStyle labelStyle{theme::font::family, 14.0, PANGO_WEIGHT_NORMAL, iconColor()};

    if (!lbl.empty()) {
        labelSz = p.measureText(lbl, labelStyle);
        contentW += 8.0 + labelSz.w;
    }

    // Center content inside bounds
    double startX = bounds.x + (bounds.w - contentW) / 2.0;
    double centerY = bounds.y + (bounds.h - iconSz.h) / 2.0;

    // Apply hover scale (subtle text shift or size animation)
    if (scale > 1.001) {
        iconStyle.size *= scale;
        labelStyle.size *= scale;
        // Re-measure centered positions with scaled sizes
        iconSz = p.measureText(icon(), iconStyle);
        contentW = iconSz.w;
        if (!lbl.empty()) {
            labelSz = p.measureText(lbl, labelStyle);
            contentW += 8.0 + labelSz.w;
        }
        startX = bounds.x + (bounds.w - contentW) / 2.0;
        centerY = bounds.y + (bounds.h - iconSz.h) / 2.0;
    }

    p.drawText(startX, centerY, icon(), iconStyle);

    if (!lbl.empty()) {
        double labelY = bounds.y + (bounds.h - labelSz.h) / 2.0;
        p.drawText(startX + iconSz.w + 8.0, labelY, lbl, labelStyle);
    }
}

}  // namespace qypr
