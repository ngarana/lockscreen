// NotificationIndicator.cpp - Notification centre implementation.
#include "ui/indicators/NotificationIndicator.hpp"

#include <algorithm>
#include <vector>

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
constexpr double kRowH = 54.0;
constexpr double kPad = 10.0;
constexpr double kMenuW = 320.0;
constexpr size_t kMaxRows = 6;  // cap the popover; the rest are counted in the footer

class NotificationPopover : public DetailedPopover {
public:
    explicit NotificationPopover(const NotificationMonitor* mon) : mon_(mon) {}

    double contentWidth() const override { return kMenuW; }
    double contentHeight() const override {
        const size_t n = std::min(rowCount(), kMaxRows);
        // Header + rows (or an empty-state line) + footer when truncated.
        double h = kPad * 2.0 + 24.0 + (n == 0 ? 28.0 : n * kRowH);
        if (rowCount() > kMaxRows) h += 22.0;
        return h;
    }

    void draw(Painter& p, int64_t now) override {
        Rect b = getBounds();
        b.y += (growUp ? 1.0 : -1.0) * (1.0 - openProgress_.value(now)) * 6.0;

        p.fillRoundedRect(b, theme::statusbar::popoverRadius, theme::color::glass);
        p.strokeRoundedRect(b, theme::statusbar::popoverRadius, theme::color::glassBorder, 1.0);

        double y = b.y + kPad;

        // Header.
        TextStyle head{theme::font::family, 12.0, PANGO_WEIGHT_BOLD, theme::color::textSubtle};
        p.drawText(b.x + kPad, y, "Notifications", head);
        y += 24.0;

        const auto& notes = list();
        if (notes.empty()) {
            TextStyle empty{theme::font::family, 13.0, PANGO_WEIGHT_NORMAL,
                            theme::color::textSubtle};
            p.drawText(b.x + kPad, y, "No notifications", empty);
            return;
        }

        // Newest first — the opposite of the monitor's oldest→newest order.
        for (size_t i = 0; i < std::min(notes.size(), kMaxRows); ++i) {
            const Notification& n = notes[notes.size() - 1 - i];
            const Rect row{b.x + kPad, y, b.w - kPad * 2.0, kRowH};

            // Critical notifications keep the accent the daemon asked for.
            const Color accent = n.urgency >= 2 ? theme::color::error : n.accent;
            p.fillRoundedRect({row.x, row.y + 4.0, 3.0, row.h - 12.0}, 1.5, accent);

            TextStyle app{theme::font::family, 11.0, PANGO_WEIGHT_BOLD, accent};
            p.drawText(row.x + 12.0, row.y + 4.0, n.app.empty() ? "System" : n.app, app);

            TextStyle title{theme::font::family, 13.0, PANGO_WEIGHT_NORMAL, theme::color::text};
            p.drawText(row.x + 12.0, row.y + 19.0, n.title.empty() ? n.app : n.title, title,
                       HAlign::Left, row.w - 24.0);

            if (!n.body.empty()) {
                TextStyle body{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL,
                               theme::color::textSubtle};
                p.drawText(row.x + 12.0, row.y + 35.0, n.body, body, HAlign::Left, row.w - 24.0);
            }
            y += kRowH;
        }

        if (notes.size() > kMaxRows) {
            TextStyle more{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL,
                           theme::color::textSubtle};
            p.drawText(b.x + kPad, y + 2.0,
                       "+" + std::to_string(notes.size() - kMaxRows) + " more", more);
        }
    }

private:
    const std::vector<Notification>& list() const {
        static const std::vector<Notification> kNone;
        return mon_ ? mon_->notifications() : kNone;
    }
    size_t rowCount() const { return list().size(); }

    const NotificationMonitor* mon_ = nullptr;
};

}  // namespace

NotificationIndicator::NotificationIndicator(const SystemBackends& backends)
    : StatusIndicator("notifications", Zone::Right, 650),
      monitor_(backends.notifications),
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
    return std::make_unique<NotificationPopover>(monitor_);
}

REGISTER_INDICATOR("notifications", Zone::Right, 650, NotificationIndicator)

}  // namespace qypr
