// NotificationIndicator.cpp - Notification centre implementation.
#include "ui/indicators/NotificationIndicator.hpp"

#include <algorithm>
#include <unordered_map>
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
constexpr const char* kChevronRight = "›";
constexpr const char* kChevronDown = "⌄";
constexpr double kPad = 10.0;
constexpr double kMenuW = 380.0;
constexpr double kHeaderH = 26.0;
constexpr double kGroupHeaderH = 32.0;
constexpr double kPreviewH = 22.0;
constexpr double kSubCardH = 52.0;
constexpr double kSubCardGap = 4.0;
constexpr double kAccentBarW = 3.0;
constexpr size_t kMaxVisibleGroups = 6;

std::string ageLabel(int64_t postedAt, int64_t now) {
    if (postedAt <= 0) return "";
    const int64_t secs = (now - postedAt) / 1000;
    if (secs < 10) return "now";
    if (secs < 60) return std::to_string(secs) + "s";
    if (secs < 3600) return std::to_string(secs / 60) + "m";
    if (secs < 86400) return std::to_string(secs / 3600) + "h";
    return std::to_string(secs / 86400) + "d";
}

// ── Group: notifications from the same app ──────────────────────────────────
struct NotifyGroup {
    std::string app;
    Color accent = theme::color::primary;
    uint8_t urgency = 1;
    // Newest-first (reversed from monitor's oldest-first).
    std::vector<const Notification*> notes;
};

std::vector<NotifyGroup> buildGroups(const std::vector<Notification>& notes) {
    // Bucket by app name, preserving insertion order for first-seen apps.
    std::vector<NotifyGroup> groups;
    std::unordered_map<std::string, size_t> appIndex;

    for (int i = static_cast<int>(notes.size()) - 1; i >= 0; --i) {
        const Notification& n = notes[i];
        const std::string key = n.app.empty() ? "System" : n.app;
        auto it = appIndex.find(key);
        if (it == appIndex.end()) {
            appIndex[key] = groups.size();
            groups.push_back({key, n.accent, n.urgency, {&n}});
        } else {
            auto& g = groups[it->second];
            g.notes.push_back(&n);
            // Critical overrides normal.
            if (n.urgency > g.urgency) g.urgency = n.urgency;
        }
    }
    return groups;
}

// ── Display item: either a group header or a sub-card ───────────────────────
struct DisplayItem {
    enum Type { GroupHeader, SubCard } type;
    size_t groupIdx;
    size_t noteIdx;
    Rect bounds;
    Rect dismissBounds;
    Rect arrowBounds;
    std::vector<Rect> buttonBounds;
    std::vector<std::string> buttonKeys;
};

class NotificationPopover : public DetailedPopover {
public:
    NotificationPopover(const NotificationMonitor* mon, NotificationActions* actions)
        : mon_(mon), actions_(actions) {}

    double contentWidth() const override { return kMenuW; }

    double contentHeight() const override {
        const auto& notes = list();
        if (notes.empty()) return kPad * 2.0 + kHeaderH + 28.0;

        auto groups = buildGroups(notes);
        const int64_t now = nowMs();
        double h = kPad * 2.0 + kHeaderH;
        const size_t n = std::min(groups.size(), kMaxVisibleGroups);

        for (size_t gi = 0; gi < n; ++gi) {
            const auto& g = groups[gi];
            h += kGroupHeaderH;  // group header row

            const double prog = groupProgress(g.app).value(now);

            if (g.notes.size() == 1 && prog < 0.01) {
                // Single notification, collapsed: show preview line.
                h += kPreviewH;
            } else if (g.notes.size() > 1) {
                // Multiple: collapsed shows preview of newest, expanded shows all.
                h += kPreviewH;  // always show at least the preview
                if (prog > 0.01) {
                    const size_t extra = g.notes.size() - 1;
                    const double expandedSubH =
                        extra * (kSubCardH + kSubCardGap);
                    h += expandedSubH * prog;
                }
            }
        }
        if (groups.size() > kMaxVisibleGroups) h += 20.0;
        return h;
    }

    void draw(Painter& p, int64_t now) override {
        Rect b = getBounds();
        b.y += (growUp ? 1.0 : -1.0) * (1.0 - openProgress_.value(now)) * 6.0;

        p.fillRoundedRect(b, theme::statusbar::popoverRadius, theme::color::surface);

        items_.clear();
        clearAll_ = {0, 0, 0, 0};
        double y = b.y + kPad;

        const auto& notes = list();

        // ── Header ─────────────────────────────────────────────────────────
        TextStyle head{theme::font::family, 12.0, PANGO_WEIGHT_BOLD,
                       theme::color::textSubtle};
        p.drawText(b.x + kPad, y, "Notifications", head);
        if (!notes.empty() && actions_) {
            TextStyle ca{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL,
                         clearAllHot_ ? theme::color::text : theme::color::textSubtle};
            const Size sz = p.measureText("Clear all", ca);
            const double cx = b.x + b.w - kPad - sz.w;
            clearAll_ = {cx - 6.0, y - 3.0, sz.w + 12.0, 20.0};
            if (clearAllHot_)
                p.fillRoundedRect(clearAll_, 6.0, theme::color::glassHover);
            p.drawText(cx, y, "Clear all", ca);
        }
        y += kHeaderH;

        if (notes.empty()) {
            TextStyle empty{theme::font::family, 13.0, PANGO_WEIGHT_NORMAL,
                            theme::color::textSubtle};
            p.drawText(b.x + kPad, y, "No notifications", empty);
            return;
        }

        // ── Groups ─────────────────────────────────────────────────────────
        groups_ = buildGroups(notes);
        const size_t n = std::min(groups_.size(), kMaxVisibleGroups);

        for (size_t gi = 0; gi < n; ++gi) {
            auto& g = groups_[gi];
            const double prog = clamp01(groupProgress(g.app).value(now));
            const bool gHot = Rect{b.x + kPad, y, b.w - kPad * 2.0,
                                   kGroupHeaderH}
                                  .contains(hoverX_, hoverY_);

            // ── Group header ───────────────────────────────────────────────
            const Rect headerR{b.x + kPad, y, b.w - kPad * 2.0, kGroupHeaderH};
            DisplayItem headerItem{DisplayItem::GroupHeader, gi, 0, headerR};

            if (gHot)
                p.fillRoundedRect(headerR, 8.0,
                                  theme::color::glassHover.withAlpha(0.35));

            // Accent bar.
            const Color accent =
                g.urgency >= 2 ? theme::color::error : g.accent;
            p.fillRoundedRect({headerR.x, headerR.y + 3.0, kAccentBarW,
                               headerR.h - 6.0},
                              1.5, accent);

            // App name + count badge.
            const double textX = headerR.x + kAccentBarW + 8.0;
            TextStyle appStyle{theme::font::family, 11.0, PANGO_WEIGHT_BOLD,
                               accent};
            p.drawText(textX, headerR.y + 4.0, g.app, appStyle);

            if (g.notes.size() > 1) {
                const std::string count = "(" + std::to_string(g.notes.size()) + ")";
                TextStyle cntStyle{theme::font::family, 10.0, PANGO_WEIGHT_NORMAL,
                                   theme::color::textSubtle};
                Size appSz = p.measureText(g.app, appStyle);
                p.drawText(textX + appSz.w + 6.0, headerR.y + 5.0, count,
                           cntStyle);
            }

            // Age of newest notification.
            const std::string age = ageLabel(g.notes[0]->postedAt, now);
            if (!age.empty()) {
                TextStyle at{theme::font::family, 10.0, PANGO_WEIGHT_NORMAL,
                             theme::color::textSubtle};
                const Size asz = p.measureText(age, at);
                p.drawText(headerR.x + headerR.w - 26.0 - asz.w,
                           headerR.y + 5.0, age, at);
            }

            // Dismiss group (×).
            if (actions_) {
                const Rect xBtn{headerR.x + headerR.w - 24.0, headerR.y + 2.0,
                                20.0, 20.0};
                const bool xHot = xBtn.contains(hoverX_, hoverY_);
                TextStyle gStyle{theme::font::iconFamily, 11.0,
                                 PANGO_WEIGHT_NORMAL,
                                 xHot ? theme::color::error
                                      : theme::color::textSubtle.withAlpha(
                                            gHot ? 0.9 : 0.0)};
                const Size gs = p.measureText(kClose, gStyle);
                p.drawText(xBtn.x + (xBtn.w - gs.w) / 2.0,
                           xBtn.y + (xHot ? (xBtn.h - gs.h) / 2.0 - 1.0
                                          : (xBtn.h - gs.h) / 2.0),
                           kClose, gStyle);
                headerItem.dismissBounds = xBtn;
            }

            // Expand / collapse chevron.
            if (g.notes.size() > 1) {
                const Rect arr{headerR.x + headerR.w - 46.0, headerR.y + 2.0,
                               20.0, 20.0};
                const bool arrHot = arr.contains(hoverX_, hoverY_);
                TextStyle arrStyle{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL,
                                   arrHot ? theme::color::primary
                                          : theme::color::textSubtle};
                const char* glyph = prog > 0.5 ? kChevronDown : kChevronRight;
                const Size asz = p.measureText(glyph, arrStyle);
                p.drawText(arr.x + (arr.w - asz.w) / 2.0,
                           arr.y + (arr.h - asz.h) / 2.0, glyph, arrStyle);
                headerItem.arrowBounds = arr;
            }

            items_.push_back(headerItem);
            y += kGroupHeaderH;

            // ── Collapsed preview (newest notification title) ──────────────
            const Notification& newest = *g.notes[0];
            const Rect previewR{b.x + kPad + 8.0, y, b.w - kPad * 2.0 - 8.0,
                                kPreviewH};
            TextStyle previewStyle{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL,
                                   theme::color::textSubtle};
            const std::string previewText =
                newest.title.empty() ? newest.app : newest.title;
            p.drawText(previewR.x, previewR.y + 3.0, previewText, previewStyle,
                       HAlign::Left, previewR.w - 10.0);
            y += kPreviewH;

            // ── Expanded sub-cards ─────────────────────────────────────────
            if (g.notes.size() > 1 && prog > 0.01) {
                p.pushGroup();
                const size_t extra = g.notes.size() - 1;
                double subY = y;
                for (size_t ni = 1; ni < g.notes.size(); ++ni) {
                    const Notification& n = *g.notes[ni];
                    const Rect subR{b.x + kPad + 4.0, subY,
                                    b.w - kPad * 2.0 - 4.0, kSubCardH};
                    DisplayItem subItem{DisplayItem::SubCard, gi, ni, subR};

                    // Sub-card background.
                    const bool subHot = subR.contains(hoverX_, hoverY_);
                    p.fillRoundedRect(subR, 6.0,
                                      subHot ? theme::color::glassHover.withAlpha(0.3)
                                             : theme::color::surface.withAlpha(0.5));
                    p.strokeRoundedRect(subR, 6.0,
                                        theme::color::glassBorder.withAlpha(0.3), 1);

                    // Title line: app source + age + dismiss.
                    const double sx = subR.x + 8.0;
                    const double sw = subR.w - 16.0;

                    // Sender/title.
                    TextStyle stStyle{theme::font::family, 10.0, PANGO_WEIGHT_BOLD,
                                      theme::color::text};
                    const std::string sender =
                        n.title.empty() ? n.app : n.title;
                    p.drawText(sx, subR.y + 4.0, sender, stStyle, HAlign::Left,
                               sw - 40.0);

                    // Age.
                    const std::string subAge = ageLabel(n.postedAt, now);
                    if (!subAge.empty()) {
                        TextStyle sat{theme::font::family, 9.0, PANGO_WEIGHT_NORMAL,
                                      theme::color::textMuted};
                        const Size sasz = p.measureText(subAge, sat);
                        p.drawText(subR.x + subR.w - 24.0 - sasz.w,
                                   subR.y + 4.0, subAge, sat);
                    }

                    // Dismiss sub-card (×).
                    if (n.daemonId != 0 && actions_) {
                        const Rect subX{subR.x + subR.w - 20.0, subR.y + 2.0,
                                        18.0, 18.0};
                        const bool subXHot = subX.contains(hoverX_, hoverY_);
                        TextStyle sxStyle{theme::font::iconFamily, 9.0,
                                          PANGO_WEIGHT_NORMAL,
                                          subXHot ? theme::color::error
                                                  : theme::color::textMuted};
                        const Size sxGs = p.measureText(kClose, sxStyle);
                        p.drawText(subX.x + (subX.w - sxGs.w) / 2.0,
                                   subX.y + (subX.h - sxGs.h) / 2.0, kClose,
                                   sxStyle);
                        subItem.dismissBounds = subX;
                    }

                    // Body (single line, truncated).
                    if (!n.body.empty()) {
                        TextStyle sbStyle{theme::font::family, 10.0,
                                          PANGO_WEIGHT_NORMAL,
                                          theme::color::textSubtle};
                        p.drawText(sx, subR.y + 18.0, n.body, sbStyle,
                                   HAlign::Left, sw - 20.0);
                    }

                    // Action buttons row.
                    std::vector<std::pair<std::string, std::string>> customActs;
                    for (const auto& a : n.actions)
                        if (a.first != "default") customActs.push_back(a);
                    if (!customActs.empty()) {
                        const double btnY = subR.y + 34.0;
                        const double btnGap = 4.0;
                        const double btnW =
                            (sw - (customActs.size() - 1) * btnGap) /
                            customActs.size();
                        double bx = sx;
                        for (const auto& a : customActs) {
                            Rect btnR{bx, btnY, btnW, 14.0};
                            bool bHot = btnR.contains(hoverX_, hoverY_);
                            p.fillRoundedRect(
                                btnR, 3.0,
                                bHot ? theme::color::glassHover
                                     : theme::color::surface.withAlpha(0.4));
                            TextStyle bTxt{theme::font::family, 8.0,
                                           PANGO_WEIGHT_BOLD,
                                           bHot ? theme::color::primary
                                                : theme::color::textMuted};
                            const Size bsz = p.measureText(a.second, bTxt);
                            p.drawText(
                                btnR.x + (btnR.w - bsz.w) / 2.0,
                                btnR.y + (btnR.h - bsz.h) / 2.0, a.second,
                                bTxt);
                            subItem.buttonBounds.push_back(btnR);
                            subItem.buttonKeys.push_back(a.first);
                            bx += btnW + btnGap;
                        }
                    }

                    items_.push_back(subItem);
                    subY += (kSubCardH + kSubCardGap) * prog;
                }
                p.popGroupWithAlpha(prog);
                y += (extra * (kSubCardH + kSubCardGap)) * prog;
            }
        }

        // Scroll hint.
        if (groups_.size() > kMaxVisibleGroups) {
            TextStyle more{theme::font::family, 10.0, PANGO_WEIGHT_NORMAL,
                           theme::color::textSubtle};
            const std::string s =
                "showing " + std::to_string(n) + " of " +
                std::to_string(groups_.size()) + " apps  ·  scroll for more";
            p.drawText(b.x + kPad, y + 1.0, s, more);
        }
    }

    bool handleClick(double x, double y) override {
        if (!actions_) return false;

        // ── "Clear all" ───────────────────────────────────────────────────
        if (clearAll_.contains(x, y)) {
            std::vector<uint32_t> ids;
            for (const auto& n : list())
                if (n.daemonId != 0) ids.push_back(n.daemonId);
            for (uint32_t id : ids) actions_->close(id);
            groupExpanded_.clear();
            scroll_ = 0;
            return true;
        }

        // ── Items: newest first (sub-cards before headers) ─────────────────
        for (int i = static_cast<int>(items_.size()) - 1; i >= 0; --i) {
            const auto& item = items_[i];
            if (!item.bounds.contains(x, y)) continue;

            if (item.type == DisplayItem::SubCard) {
                const auto& g = groups_.at(item.groupIdx);
                const Notification& n = *g.notes[item.noteIdx];

                // Dismiss sub-card.
                if (item.dismissBounds.valid() &&
                    item.dismissBounds.contains(x, y) && n.daemonId != 0) {
                    actions_->close(n.daemonId);
                    return true;
                }
                // Action buttons.
                for (size_t k = 0; k < item.buttonBounds.size(); ++k) {
                    if (item.buttonBounds[k].contains(x, y) &&
                        n.daemonId != 0) {
                        actions_->invoke(n.daemonId, item.buttonKeys[k]);
                        return true;
                    }
                }
                // Click sub-card body → invoke default action.
                if (n.daemonId != 0) {
                    for (const auto& a : n.actions) {
                        if (a.first == "default") {
                            actions_->invoke(n.daemonId, "default");
                            return true;
                        }
                    }
                }
            } else {
                // Group header.
                const auto& g = groups_[item.groupIdx];

                // Dismiss group (all notifications in it).
                if (item.dismissBounds.valid() &&
                    item.dismissBounds.contains(x, y)) {
                    for (const auto* n : g.notes)
                        if (n->daemonId != 0) actions_->close(n->daemonId);
                    groupExpanded_.erase(g.app);
                    return true;
                }
                // Expand/collapse chevron.
                if (item.arrowBounds.valid() &&
                    item.arrowBounds.contains(x, y)) {
                    toggleGroupExpanded(g.app);
                    return true;
                }
                // Click group header → expand/collapse.
                if (g.notes.size() > 1) {
                    toggleGroupExpanded(g.app);
                    return true;
                }
                // Single notification: click to open.
                if (g.notes[0]->daemonId != 0) {
                    for (const auto& a : g.notes[0]->actions) {
                        if (a.first == "default") {
                            actions_->invoke(g.notes[0]->daemonId, "default");
                            return true;
                        }
                    }
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
        auto groups = buildGroups(list());
        if (groups.size() <= kMaxVisibleGroups) return false;
        const size_t maxScroll = groups.size() - kMaxVisibleGroups;
        if (dy > 0 && scroll_ > 0) --scroll_;
        else if (dy < 0 && scroll_ < maxScroll) ++scroll_;
        return true;
    }

private:
    const std::vector<Notification>& list() const {
        static const std::vector<Notification> kNone;
        return mon_ ? mon_->notifications() : kNone;
    }

    bool isGroupExpanded(const std::string& app) const {
        auto it = groupExpanded_.find(app);
        return it != groupExpanded_.end() && it->second;
    }

    Animated& groupProgress(const std::string& app) {
        return groupProgress_[app];
    }
    const Animated& groupProgress(const std::string& app) const {
        auto it = groupProgress_.find(app);
        if (it != groupProgress_.end()) return it->second;
        static const Animated kDefault{0.0};
        return kDefault;
    }

    void toggleGroupExpanded(const std::string& app) {
        bool& exp = groupExpanded_[app];
        exp = !exp;
        groupProgress_[app].animateTo(exp ? 1.0 : 0.0, theme::anim::medium,
                                      ease::inOutQuad);
    }

    const NotificationMonitor* mon_ = nullptr;
    NotificationActions* actions_ = nullptr;

    // Display items rebuilt every draw.
    std::vector<DisplayItem> items_;
    // Groups rebuilt every draw (for click hit-testing).
    std::vector<NotifyGroup> groups_;
    Rect clearAll_{0, 0, 0, 0};
    bool clearAllHot_ = false;
    size_t scroll_ = 0;
    double hoverX_ = -1, hoverY_ = -1;

    // Persistent state across redraws.
    std::unordered_map<std::string, bool> groupExpanded_;
    std::unordered_map<std::string, Animated> groupProgress_;
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
