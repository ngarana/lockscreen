// LauncherPopover.cpp - Application launcher implementation.
#include "ui/statusbar/LauncherPopover.hpp"

#include <xkbcommon/xkbcommon-keysyms.h>

#include <algorithm>

#include "render/Painter.hpp"
#include "system/DesktopIndex.hpp"
#include "ui/Theme.hpp"

namespace qypr {

namespace {
constexpr double kW = 380.0;
constexpr double kPad = 8.0;
constexpr double kSearchH = 38.0;
constexpr double kGap = 6.0;     // search box → result list
constexpr double kRowH = 34.0;
constexpr size_t kRows = 8;      // max visible result rows
constexpr const char* kSearchGlyph = "󰍉";  // nf-md-magnify
}  // namespace

LauncherPopover::LauncherPopover(DesktopIndex* index) : index_(index) { refresh(); }

double LauncherPopover::contentWidth() const { return kW; }

double LauncherPopover::contentHeight() const {
    const size_t vis = results_.empty() ? 1 : std::min(results_.size(), kRows);
    return kPad + kSearchH + kGap + static_cast<double>(vis) * kRowH + kPad;
}

void LauncherPopover::refresh() {
    results_ = index_ ? index_->search(query_) : std::vector<const DesktopEntry*>{};
    sel_ = 0;
    scroll_ = 0;
}

void LauncherPopover::ensureVisible() {
    if (sel_ < scroll_) scroll_ = sel_;
    else if (sel_ >= scroll_ + kRows) scroll_ = sel_ - kRows + 1;
}

void LauncherPopover::launch(size_t idx) {
    if (idx >= results_.size()) return;
    const DesktopEntry* e = results_[idx];
    spawnDetached(e->exec, e->terminal);
    closeRequested_ = true;
}

bool LauncherPopover::handleText(const std::string& utf8) {
    query_ += utf8;
    refresh();
    return true;
}

bool LauncherPopover::handleKey(uint32_t keysym) {
    switch (keysym) {
        case XKB_KEY_BackSpace:
            if (!query_.empty()) {
                // UTF-8 aware: drop trailing continuation bytes, then the lead byte.
                size_t i = query_.size();
                do { --i; } while (i > 0 && (static_cast<unsigned char>(query_[i]) & 0xC0) == 0x80);
                query_.erase(i);
                refresh();
            }
            return true;
        case XKB_KEY_Up:
            if (sel_ > 0) { --sel_; ensureVisible(); }
            return true;
        case XKB_KEY_Down:
            if (sel_ + 1 < results_.size()) { ++sel_; ensureVisible(); }
            return true;
        case XKB_KEY_Return:
        case XKB_KEY_KP_Enter:
            if (!results_.empty()) launch(sel_);
            return true;
    }
    return false;
}

bool LauncherPopover::handleClick(double x, double y) {
    const Rect b = getBounds();
    const double listTop = b.y + kPad + kSearchH + kGap;
    const size_t vis = std::min(results_.size() - std::min(scroll_, results_.size()), kRows);
    for (size_t j = 0; j < vis; ++j) {
        const Rect row{b.x + kPad, listTop + static_cast<double>(j) * kRowH, b.w - kPad * 2, kRowH};
        if (row.contains(x, y)) {
            launch(scroll_ + j);
            return true;
        }
    }
    return true;  // swallow clicks inside the popover (never dismiss on a miss)
}

bool LauncherPopover::handleDrag(double x, double y) {
    hoverX_ = x;
    hoverY_ = y;
    return false;
}

bool LauncherPopover::consumeCloseRequest() {
    const bool c = closeRequested_;
    closeRequested_ = false;
    return c;
}

void LauncherPopover::draw(Painter& p, int64_t now) {
    Rect b = getBounds();
    b.y += (growUp ? 1.0 : -1.0) * (1.0 - openProgress_.value(now)) * 6.0;
    if (!drawSharedBackdrop(p, b, theme::statusbar::popoverRadius))
        p.fillRoundedRectSource(b, theme::statusbar::popoverRadius, theme::statusbar::panelSurface());

    // --- Search field ---
    const Rect sr{b.x + kPad, b.y + kPad, b.w - kPad * 2, kSearchH};
    p.fillRoundedRect(sr, 8.0, theme::color::glassHover);
    TextStyle gs{theme::font::iconFamily, 15.0, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
    p.drawText(sr.x + 10.0, sr.y + (kSearchH - 17.0) / 2.0, kSearchGlyph, gs);
    const double textX = sr.x + 34.0;
    if (query_.empty()) {
        TextStyle ph{theme::font::family, 13.0, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
        p.drawText(textX, sr.y + (kSearchH - 16.0) / 2.0, "Search applications…", ph);
    } else {
        TextStyle qs{theme::font::family, 13.0, PANGO_WEIGHT_NORMAL, theme::color::text};
        Size qsz = p.measureText(query_, qs, sr.w - 44.0);
        p.drawText(textX, sr.y + (kSearchH - 16.0) / 2.0, query_, qs, HAlign::Left, sr.w - 44.0);
        // Caret just past the query text.
        p.fillRect({textX + qsz.w + 1.0, sr.y + 9.0, 1.5, kSearchH - 18.0}, theme::color::primary);
    }

    // --- Results ---
    const double listTop = b.y + kPad + kSearchH + kGap;
    if (results_.empty()) {
        TextStyle es{theme::font::family, 12.0, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
        p.drawText(b.x + kPad + 8.0, listTop + (kRowH - 14.0) / 2.0, "No matching applications", es);
        return;
    }

    const size_t vis = std::min(results_.size() - scroll_, kRows);
    for (size_t j = 0; j < vis; ++j) {
        const size_t idx = scroll_ + j;
        const DesktopEntry* e = results_[idx];
        const Rect row{b.x + kPad, listTop + static_cast<double>(j) * kRowH, b.w - kPad * 2, kRowH};
        const bool selected = idx == sel_;
        const bool hot = row.contains(hoverX_, hoverY_);
        if (selected) {
            p.fillRoundedRect(row, 6.0, theme::color::primary.withAlpha(0.22));
        } else if (hot) {
            p.fillRoundedRect(row, 6.0, theme::color::glassHover);
        }

        TextStyle ns{theme::font::family, 13.0,
                     selected ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL, theme::color::text};
        p.drawText(row.x + 10.0, row.y + 4.0, e->name, ns, HAlign::Left, row.w - 20.0);
        TextStyle xs{theme::font::family, 10.0, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
        p.drawText(row.x + 10.0, row.y + 19.0, e->exec, xs, HAlign::Left, row.w - 20.0);
    }
}

}  // namespace qypr
