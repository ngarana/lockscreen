#include "ui/Notification.hpp"

#include <cctype>
#include <numeric>

#include "render/Painter.hpp"
#include "ui/IconResolver.hpp"
#include "ui/Theme.hpp"

namespace qypr {

namespace {
TextStyle titleStyle() {
    return {theme::font::family, theme::notification::titleSize, PANGO_WEIGHT_BOLD,
            theme::color::text};
}
TextStyle bodyStyle() {
    return {theme::font::family, theme::notification::bodySize, PANGO_WEIGHT_NORMAL,
            theme::color::textSubtle};
}
TextStyle iconStyle(double tile) {
    return {theme::font::family, tile * 0.5, PANGO_WEIGHT_BOLD, theme::color::text};
}

// First printable character of `s`, uppercased — the tile fallback glyph.
std::string tileGlyph(const std::string& s) {
    for (unsigned char c : s) {
        if (!std::isspace(c)) return std::string(1, static_cast<char>(std::toupper(c)));
    }
    return "?";
}
}  // namespace

void NotificationView::update(std::vector<Notification> notes) {
    std::vector<Card> next;
    next.reserve(notes.size());
    for (auto& n : notes) {
        auto it = std::find_if(cards_.begin(), cards_.end(),
                               [&](const Card& c) { return c.note.id == n.id; });
        if (it != cards_.end()) {
            Card c = *it;          // keep its animation state (no re-fade)
            c.note = std::move(n); // refresh text/icon/accent
            next.push_back(std::move(c));
        } else {
            Card c{std::move(n), Animated{0}, Animated{0.0}, false, false, false};
            c.appear.animateTo(1.0, theme::anim::reveal, ease::inOutQuad);
            next.push_back(std::move(c));
        }
    }
    cards_ = std::move(next);
    layout_.clear();
}

double NotificationView::cardHeight(const Card& c, Painter& p, double w, int64_t now) const {
    const double pad = theme::notification::padding;
    const double icon = theme::notification::iconSize;
    const double textW = w - 2 * pad - icon - pad;
    const std::string head = c.note.sensitive ? "New Notification" : (c.note.title.empty() ? c.note.app : c.note.title);
    const std::string body = c.note.sensitive ? "Contents hidden" : c.note.body;

    const double titleW = textW - 24; // reserve space for close button
    const double titleH = p.measureText(head, titleStyle(), titleW).h;

    const double collapsedTextH = titleH;
    const double bodyH = p.measureText(body, bodyStyle(), textW).h;
    const double expandedTextH = titleH + theme::spacing::small + bodyH;

    double progress = clamp01(c.expandProgress.value(now));
    double textH = collapsedTextH + (expandedTextH - collapsedTextH) * progress;

    return 2 * pad + std::max(icon, textH);
}

void NotificationView::drawCard(Card& c, Painter& p, int64_t now, const Rect& r) {
    const double a = clamp01(c.appear.value(now));
    if (a <= 0.01) return;

    const double pad = theme::notification::padding;
    const double icon = theme::notification::iconSize;
    const double textW = r.w - 2 * pad - icon - pad;
    const std::string head = c.note.sensitive ? "New Notification" : (c.note.title.empty() ? c.note.app : c.note.title);
    const std::string body = c.note.sensitive ? "Contents hidden" : c.note.body;

    auto paint = [&] {
        const Color fill = c.hovered ? theme::color::glassHover : theme::color::glass;
        const Color border = c.hovered ? theme::color::primary : theme::color::glassBorder;
        p.fillRoundedRect(r, theme::notification::radius, fill);
        p.strokeRoundedRect(r, theme::notification::radius, border, 1);

        // App tile (coloured square with a glyph).
        Rect tile{r.x + pad, r.y + pad, icon, icon};
        p.fillRoundedRect(tile, theme::radius::medium, c.note.accent.withAlpha(0.9));
        cairo_surface_t* iconSurf = IconResolver::instance().get(c.note.icon);
        if (iconSurf) {
            Rect iconDest{tile.x + 2, tile.y + 2, tile.w - 4, tile.h - 4};
            p.drawSurface(iconSurf, iconDest);
        } else {
            const std::string glyph = tileGlyph(c.note.app);
            const TextStyle is = iconStyle(icon);
            const Size gs = p.measureText(glyph, is);
            p.drawText(tile.cx() - gs.w / 2.0, tile.cy() - gs.h / 2.0, glyph, is, HAlign::Left);
        }

        // Text block, vertically centred against the tile dynamically based on expandProgress.
        const double titleW = textW - 24; // reserve space for close button
        const Size ts = p.measureText(head, titleStyle(), titleW);
        const Size bs = p.measureText(body, bodyStyle(), textW);

        double progress = clamp01(c.expandProgress.value(now));
        const double textH = ts.h + (theme::spacing::small + bs.h) * progress;
        double ty = r.y + pad + std::max(0.0, (icon - textH) / 2.0);
        const double tx = r.x + pad + icon + pad;

        p.drawText(tx, ty, head, titleStyle(), HAlign::Left, titleW);
        if (progress > 0.01) {
            p.pushGroup();
            p.drawText(tx, ty + ts.h + theme::spacing::small, body, bodyStyle(),
                       HAlign::Left, textW);
            p.popGroupWithAlpha(progress);
        }

        // Close button (×) on hover.
        if (c.hovered) {
            Rect closeRect{r.x + r.w - pad - 24, r.y + pad, 24, 24};
            if (c.closeHovered) {
                p.fillRoundedRect(closeRect, theme::radius::small, theme::color::surfaceHover.withAlpha(0.5));
            }
            TextStyle cs = {theme::font::family, 12, PANGO_WEIGHT_BOLD,
                            c.closeHovered ? theme::color::error : theme::color::textMuted};
            const std::string closeGlyph = "×";
            const Size gs = p.measureText(closeGlyph, cs);
            p.drawText(closeRect.cx() - gs.w / 2.0, closeRect.cy() - gs.h / 2.0 - 1.0, closeGlyph, cs, HAlign::Left);
        }
    };

    if (a >= 0.999) {
        paint();
        return;
    }
    p.pushGroup();
    paint();
    p.popGroupWithAlpha(a);
}

void NotificationView::draw(Painter& p, int64_t now, double left, double bottom,
                            double maxWidth) {
    layout_.clear();
    if (cards_.empty()) return;

    const double w = std::min(maxWidth, static_cast<double>(theme::notification::cardWidth));
    const double gap = theme::notification::gap;
    const size_t start = cards_.size() > theme::notification::maxVisible
                             ? cards_.size() - theme::notification::maxVisible
                             : 0;

    // Visible slice, oldest card at the top, newest at the bottom.
    std::vector<double> heights;
    for (size_t i = start; i < cards_.size(); ++i)
        heights.push_back(cardHeight(cards_[i], p, w, now));
    double total = std::accumulate(heights.begin(), heights.end(), 0.0);
    if (heights.size() > 1) total += gap * (heights.size() - 1);

    double y = bottom - total;
    for (size_t k = 0; k < heights.size(); ++k) {
        const size_t idx = start + k;
        const double h = heights[k];
        const Rect r{left, y, w, h};
        layout_.emplace_back(idx, r);
        drawCard(cards_[idx], p, now, r);
        y += h + gap;
    }
}

bool NotificationView::handlePress(double x, double y, int64_t now) {
    for (const auto& [idx, r] : layout_) {
        if (r.contains(x, y) && idx < cards_.size()) {
            const double pad = theme::notification::padding;
            Rect closeRect{r.x + r.w - pad - 24, r.y + pad, 24, 24};
            if (closeRect.contains(x, y)) {
                cards_.erase(cards_.begin() + idx);
                layout_.clear();
                return true;
            } else {
                Card& c = cards_[idx];
                c.expanded = !c.expanded;
                double target = c.expanded ? 1.0 : 0.0;
                c.expandProgress.animateTo(target, theme::anim::medium, ease::inOutQuad);
                return true;
            }
        }
    }
    return false;
}

void NotificationView::updateHover(double x, double y, int64_t) {
    for (const auto& [idx, r] : layout_) {
        if (idx < cards_.size()) {
            cards_[idx].hovered = r.contains(x, y);
            if (cards_[idx].hovered) {
                const double pad = theme::notification::padding;
                Rect closeRect{r.x + r.w - pad - 24, r.y + pad, 24, 24};
                cards_[idx].closeHovered = closeRect.contains(x, y);
            } else {
                cards_[idx].closeHovered = false;
            }
        }
    }
}

void NotificationView::clearHover(int64_t) {
    for (auto& c : cards_) {
        c.hovered = false;
        c.closeHovered = false;
    }
}

bool NotificationView::animating(int64_t now) const {
    for (const auto& c : cards_)
        if (c.appear.active(now) || c.expandProgress.active(now)) return true;
    return false;
}

std::vector<Notification> demoNotifications() {
    return {
        {1, 0, "Calendar", "Team Standup", "10:30 AM — Daily sync in Meeting Room B", "",
         theme::color::blue},
        {2, 0, "Mail", "New message from Priya",
         "Re: Q3 roadmap — please review the attached draft", "", theme::color::green, 0, 1, true},
        {3, 0, "System", "Update available", "Hyprland 0.41.0 can be installed", "",
         theme::color::mauve},
        {4, 0, "Weather", "Rain expected", "Showers this afternoon, high of 18°C", "",
         theme::color::yellow},
    };
}

}  // namespace qypr
