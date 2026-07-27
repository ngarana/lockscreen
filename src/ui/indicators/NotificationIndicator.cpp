// NotificationIndicator.cpp - Notification centre implementation.
#include "ui/indicators/NotificationIndicator.hpp"

#include <algorithm>
#include <vector>

#include "notifications/NotificationActions.hpp"
#include "notifications/NotificationMonitor.hpp"
#include "render/Painter.hpp"
#include "system/DndState.hpp"
#include "ui/Notification.hpp"
#include "ui/Theme.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"

namespace qypr {

namespace {

constexpr const char* kBell = "󰂚";     // nf-md-bell
constexpr const char* kBellOff = "󰂛";  // nf-md-bell_off (DND)
constexpr const char* kClose = "󰅖";    // nf-md-close
constexpr double kRowH = 54.0;
constexpr double kPad = 10.0;
constexpr double kMenuW = 380.0;
constexpr double kHeaderH = 26.0;
constexpr size_t kMaxRows = 6;  // one page; scroll reaches the rest

// Relative age, the way a phone shows it. Notifications are recent by nature,
// so minutes/hours carry more meaning than a wall-clock time.
std::string ageLabel(int64_t postedAt, int64_t now) {
    if (postedAt <= 0) return "";
    const int64_t secs = (now - postedAt) / 1000;
    if (secs < 10) return "now";
    if (secs < 60) return std::to_string(secs) + "s";
    if (secs < 3600) return std::to_string(secs / 60) + "m";
    if (secs < 86400) return std::to_string(secs / 3600) + "h";
    return std::to_string(secs / 86400) + "d";
}

class NotificationPopover : public DetailedPopover {
public:
    NotificationPopover(const NotificationMonitor* mon, NotificationActions* actions)
        : mon_(mon), actions_(actions) {}

    double contentWidth() const override { return kMenuW; }
    double contentHeight() const override {
        const auto& notes = list();
        const size_t n = std::min(visibleCount(), kMaxRows);
        double h = kPad * 2.0 + kHeaderH;
        if (n == 0) {
            h += 28.0;
        } else {
            const size_t total = notes.size();
            const size_t first = std::min(scroll_, total > kMaxRows ? total - kMaxRows : size_t{0});
            for (size_t i = first; i < std::min(first + kMaxRows, total); ++i) {
                const Notification& n = notes[total - 1 - i];
                int customCount = 0;
                for (const auto& act : n.actions) {
                    if (act.first != "default") customCount++;
                }
                h += (customCount > 0) ? 78.0 : 54.0;
            }
        }
        if (visibleCount() > kMaxRows) h += 20.0;  // scroll hint
        return h;
    }

    void draw(Painter& p, int64_t now) override {
        Rect b = getBounds();
        b.y += (growUp ? 1.0 : -1.0) * (1.0 - openProgress_.value(now)) * 6.0;

        p.fillRoundedRect(b, theme::statusbar::popoverRadius, theme::color::surface);

        rows_.clear();
        clearAll_ = {0, 0, 0, 0};
        double y = b.y + kPad;

        const auto& notes = list();

        // ── Header: title + "Clear all" ────────────────────────────────────
        TextStyle head{theme::font::family, 12.0, PANGO_WEIGHT_BOLD, theme::color::textSubtle};
        p.drawText(b.x + kPad, y, "Notifications", head);
        if (!notes.empty() && actions_) {
            TextStyle ca{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL,
                         clearAllHot_ ? theme::color::text : theme::color::textSubtle};
            const Size sz = p.measureText("Clear all", ca);
            const double cx = b.x + b.w - kPad - sz.w;
            clearAll_ = {cx - 6.0, y - 3.0, sz.w + 12.0, 20.0};
            if (clearAllHot_) p.fillRoundedRect(clearAll_, 6.0, theme::color::glassHover);
            p.drawText(cx, y, "Clear all", ca);
        }
        y += kHeaderH;

        if (notes.empty()) {
            TextStyle empty{theme::font::family, 13.0, PANGO_WEIGHT_NORMAL,
                            theme::color::textSubtle};
            p.drawText(b.x + kPad, y, "No notifications", empty);
            return;
        }

        // ── Rows: newest first (the monitor stores oldest→newest) ──────────
        const size_t total = notes.size();
        const size_t first = std::min(scroll_, total > kMaxRows ? total - kMaxRows : size_t{0});
        for (size_t i = first; i < std::min(first + kMaxRows, total); ++i) {
            const Notification& n = notes[total - 1 - i];

            std::vector<std::pair<std::string, std::string>> customActions;
            for (const auto& act : n.actions) {
                if (act.first != "default") customActions.push_back(act);
            }

            const double curRowH = customActions.empty() ? 54.0 : 78.0;
            const Rect row{b.x + kPad, y, b.w - kPad * 2.0, curRowH};
            Row rowItem{row, n.daemonId, n.actions, {}, {}};

            const bool hot = row.contains(hoverX_, hoverY_);
            if (hot) p.fillRoundedRect(row, 8.0, theme::color::glassHover.withAlpha(0.35));

            // Critical notifications keep the accent the daemon asked for.
            const Color accent = n.urgency >= 2 ? theme::color::error : n.accent;
            p.fillRoundedRect({row.x, row.y + 4.0, 3.0, row.h - 12.0}, 1.5, accent);

            // App name + age, on one line.
            TextStyle app{theme::font::family, 11.0, PANGO_WEIGHT_BOLD, accent};
            p.drawText(row.x + 12.0, row.y + 4.0, n.app.empty() ? "System" : n.app, app);
            const std::string age = ageLabel(n.postedAt, now);
            if (!age.empty()) {
                TextStyle at{theme::font::family, 10.0, PANGO_WEIGHT_NORMAL,
                             theme::color::textSubtle};
                const Size asz = p.measureText(age, at);
                p.drawText(row.x + row.w - 26.0 - asz.w, row.y + 5.0, age, at);
            }

            // Dismiss (×) — only meaningful once the daemon has assigned an id.
            if (n.daemonId != 0 && actions_) {
                const Rect x{row.x + row.w - 24.0, row.y + 2.0, 20.0, 20.0};
                const bool xhot = x.contains(hoverX_, hoverY_);
                TextStyle g{theme::font::iconFamily, 11.0, PANGO_WEIGHT_NORMAL,
                            xhot ? theme::color::error
                                 : theme::color::textSubtle.withAlpha(hot ? 0.9 : 0.0)};
                const Size gs = p.measureText(kClose, g);
                p.drawText(x.x + (x.w - gs.w) / 2.0, x.y + (x.h - gs.h) / 2.0, kClose, g);
            }

            TextStyle title{theme::font::family, 13.0, PANGO_WEIGHT_NORMAL, theme::color::text};
            p.drawText(row.x + 12.0, row.y + 19.0, n.title.empty() ? n.app : n.title, title,
                       HAlign::Left, row.w - 40.0);

            if (!n.body.empty()) {
                TextStyle body{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL,
                               theme::color::textSubtle};
                p.drawText(row.x + 12.0, row.y + 35.0, n.body, body, HAlign::Left, row.w - 40.0);
            }

            // Render custom action buttons if present
            if (!customActions.empty()) {
                const double availW = row.w - 24.0;
                const double btnW = (availW - (customActions.size() - 1) * 6.0) / customActions.size();
                double bx = row.x + 12.0;
                for (const auto& act : customActions) {
                    Rect btnRect{bx, row.y + 52.0, btnW, 20.0};
                    bool bhot = btnRect.contains(hoverX_, hoverY_);
                    p.fillRoundedRect(btnRect, 4.0, bhot ? theme::color::glassHover : theme::color::surface.withAlpha(0.6));
                    TextStyle btnTxt{theme::font::family, 10.0, PANGO_WEIGHT_BOLD, bhot ? theme::color::text : theme::color::textSubtle};
                    const Size bsz = p.measureText(act.second, btnTxt);
                    p.drawText(btnRect.x + (btnRect.w - bsz.w) / 2.0, btnRect.y + (btnRect.h - bsz.h) / 2.0 + 1.0, act.second, btnTxt);
                    rowItem.buttonBounds.push_back(btnRect);
                    rowItem.buttonKeys.push_back(act.first);
                    bx += btnW + 6.0;
                }
            }

            rows_.push_back(std::move(rowItem));
            y += curRowH;
        }

        // Scroll hint: how many are above/below this page.
        if (total > kMaxRows) {
            TextStyle more{theme::font::family, 10.0, PANGO_WEIGHT_NORMAL,
                           theme::color::textSubtle};
            const std::string s = "showing " + std::to_string(first + 1) + "–" +
                                  std::to_string(std::min(first + kMaxRows, total)) + " of " +
                                  std::to_string(total) + "  ·  scroll for more";
            p.drawText(b.x + kPad, y + 1.0, s, more);
        }
    }

    bool handleClick(double x, double y) override {
        if (!actions_) return false;
        if (clearAll_.contains(x, y)) {
            // Copy the ids first: each close triggers a NotificationClosed that
            // mutates the monitor's vector as we iterate it.
            std::vector<uint32_t> ids;
            for (const auto& n : list()) {
                if (n.daemonId != 0) ids.push_back(n.daemonId);
            }
            for (uint32_t id : ids) actions_->close(id);
            scroll_ = 0;
            return true;
        }
        for (const auto& r : rows_) {
            const Rect close{r.bounds.x + r.bounds.w - 24.0, r.bounds.y + 2.0, 20.0, 20.0};
            if (close.contains(x, y) && r.daemonId != 0) {
                actions_->close(r.daemonId);
                return true;
            }
            for (size_t k = 0; k < r.buttonBounds.size(); ++k) {
                if (r.buttonBounds[k].contains(x, y) && r.daemonId != 0) {
                    actions_->invoke(r.daemonId, r.buttonKeys[k]);
                    return true;
                }
            }
            if (r.bounds.contains(x, y) && r.daemonId != 0) {
                bool hasDefault = false;
                for (const auto& act : r.actions) {
                    if (act.first == "default") {
                        hasDefault = true;
                        break;
                    }
                }
                if (hasDefault) {
                    actions_->invoke(r.daemonId, "default");
                    return true;
                }
            }
        }
        return false;
    }

    bool handleDrag(double x, double y) override {
        hoverX_ = x;
        hoverY_ = y;
        clearAllHot_ = clearAll_.contains(x, y);
        return false;
    }

    bool handleScroll(double, double dy) override {
        const size_t total = visibleCount();
        if (total <= kMaxRows) return false;
        const size_t maxScroll = total - kMaxRows;
        if (dy > 0 && scroll_ > 0) --scroll_;
        else if (dy < 0 && scroll_ < maxScroll) ++scroll_;
        return true;
    }

private:
    struct Row {
        Rect bounds;
        uint32_t daemonId;
        std::vector<std::pair<std::string, std::string>> actions;
        std::vector<Rect> buttonBounds;
        std::vector<std::string> buttonKeys;
    };

    const std::vector<Notification>& list() const {
        static const std::vector<Notification> kNone;
        return mon_ ? mon_->notifications() : kNone;
    }
    size_t visibleCount() const { return list().size(); }

    const NotificationMonitor* mon_ = nullptr;
    NotificationActions* actions_ = nullptr;
    std::vector<Row> rows_;  // rebuilt each draw; hit-tested on click
    Rect clearAll_{0, 0, 0, 0};
    bool clearAllHot_ = false;
    size_t scroll_ = 0;  // index of the newest row on this page
    double hoverX_ = -1, hoverY_ = -1;
};

}  // namespace

NotificationIndicator::NotificationIndicator(const SystemBackends& backends)
    : StatusIndicator("notifications", Zone::Right, 650),
      monitor_(backends.notifications),
      actions_(backends.notificationActions),
      dnd_(backends.dnd) {
    // No monitor (qypr-lock) → the applet does not exist at all.
    visible = monitor_ != nullptr;
}

size_t NotificationIndicator::count() const {
    return monitor_ ? monitor_->notifications().size() : 0;
}

std::string NotificationIndicator::icon() const {
    // While DND is on the bell reads as muted — the count still shows, because
    // suppressed notifications are still waiting for you.
    return (dnd_ && dnd_->enabled()) ? kBellOff : kBell;
}

std::string NotificationIndicator::themedIcon() const {
    // notification-* symbolic from the active icon theme.
    return (dnd_ && dnd_->enabled()) ? "notification-alert-symbolic"
                                      : "notification-new-symbolic";
}

std::string NotificationIndicator::label() const {
    const size_t n = count();
    return n == 0 ? "" : std::to_string(n);
}

std::string NotificationIndicator::tooltip() const {
    const size_t n = count();
    if (n == 0) return "No notifications";
    return std::to_string(n) + (n == 1 ? " notification" : " notifications");
}

Color NotificationIndicator::iconColor() const {
    if (dnd_ && dnd_->enabled()) return theme::color::textSubtle;
    return count() > 0 ? theme::color::primary : theme::color::textSubtle;
}

void NotificationIndicator::onBackendUpdate() {
    if (monitor_) visible = true;  // count/label re-read on draw
}

std::unique_ptr<DetailedPopover> NotificationIndicator::createDetailedView() {
    return std::make_unique<NotificationPopover>(monitor_, actions_);
}

REGISTER_INDICATOR("notifications", Zone::Right, 650, NotificationIndicator)

}  // namespace qypr
