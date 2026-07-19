// BatteryIndicator.cpp - Status bar battery indicator implementation.
#include "ui/indicators/BatteryIndicator.hpp"

#include <vector>

#include "render/Painter.hpp"
#include "system/PowerProfilesBackend.hpp"
#include "ui/Theme.hpp"
#include "ui/statusbar/DetailedPopover.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"
#include "ui/statusbar/QSTile.hpp"

namespace qypr {

namespace {

// Short pill label for a power-profiles-daemon profile name.
std::string profileLabel(const std::string& name) {
    if (name == "power-saver") return "Saver";
    if (name == "balanced") return "Balanced";
    if (name == "performance") return "Perf";
    return name;
}
constexpr const char* kDischargingIcons[] = {
    "󰂎", "󰁺", "󰁻", "󰁼", "󰁽", "󰁾", "󰁿", "󰂀", "󰂁", "󰂂", "󰁹"
};
constexpr const char* kChargingIcons[] = {
    "󰢟", "󰢜", "󰂆", "󰂇", "󰂈", "󰢝", "󰂉", "󰂊", "󰂋", "󰂅", "󰂅"
};

const char* batteryIcon(int percentage, bool charging) {
    int idx = percentage / 10;
    if (idx > 10) idx = 10;
    if (idx < 0) idx = 0;
    return charging ? kChargingIcons[idx] : kDischargingIcons[idx];
}

std::string stateString(BatterySnapshot::State state) {
    switch (state) {
        case BatterySnapshot::Charging: return "Charging";
        case BatterySnapshot::Discharging: return "Discharging";
        case BatterySnapshot::Full: return "Full";
        case BatterySnapshot::PendingCharge: return "Pending Charge";
        default: return "Unknown";
    }
}

std::string formatTime(int64_t seconds) {
    if (seconds <= 0) return "";
    int64_t h = seconds / 3600;
    int64_t m = (seconds % 3600) / 60;
    if (h > 0) return std::to_string(h) + "h " + std::to_string(m) + "min";
    return std::to_string(m) + "min";
}

// Detailed popover per docs/STATUS_BAR.md: percentage bar, time remaining,
// power draw, state. Reads the indicator's snapshot live (safe: PopoverManager
// is destroyed before the indicator vectors in StatusBar).
class BatteryPopover : public DetailedPopover {
public:
    // `profiles` is null on the lock screen (bar-only) — the profile selector is
    // then omitted and only the battery detail shows.
    BatteryPopover(const BatterySnapshot* snap, PowerProfilesBackend* profiles)
        : snap_(snap), profiles_(profiles) {}

    bool hasProfiles() const { return profiles_ && profiles_->snapshot().available; }
    double contentHeight() const override { return hasProfiles() ? 224.0 : 148.0; }

    void draw(Painter& p, int64_t now) override {
        Rect b = getBounds();
        // Slide down 6px while opening (fade is applied by PopoverManager).
        b.y -= (1.0 - openProgress_.value(now)) * 6.0;

        p.fillGlass(b, theme::statusbar::popoverRadius, theme::color::glass, theme::color::glassBorder);
        drawProfiles(p, b);

        const double pad = theme::statusbar::popoverPadding;
        double x = b.x + pad;
        double y = b.y + pad;
        const double innerW = b.w - 2 * pad;

        TextStyle title{theme::font::family, 14.0, PANGO_WEIGHT_BOLD, theme::color::text};
        p.drawText(x, y, "Battery", title);
        p.drawText(b.x + b.w - pad, y, std::to_string(snap_->percentage) + "%", title,
                   HAlign::Right);
        y += 30.0;

        // Charge bar
        const double barH = 8.0;
        p.fillRoundedRect({x, y, innerW, barH}, barH / 2, theme::color::surface);
        double frac = snap_->percentage / 100.0;
        if (frac > 0.01) {
            Color fill = snap_->percentage > 50   ? theme::color::success
                         : snap_->percentage > 20 ? theme::color::warning
                                                  : theme::color::error;
            p.fillRoundedRect({x, y, innerW * frac, barH}, barH / 2, fill);
        }
        y += barH + 16.0;

        TextStyle line{theme::font::family, 12.5, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
        if (snap_->state == BatterySnapshot::Discharging && snap_->timeToEmpty > 0) {
            p.drawText(x, y, formatTime(snap_->timeToEmpty) + " remaining", line);
            y += 20.0;
        } else if (snap_->state == BatterySnapshot::Charging && snap_->timeToFull > 0) {
            p.drawText(x, y, formatTime(snap_->timeToFull) + " until full", line);
            y += 20.0;
        }
        if (snap_->energyRate > 0.05) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%.1f W power draw", snap_->energyRate);
            p.drawText(x, y, buf, line);
            y += 20.0;
        }
        p.drawText(x, y, "State: " + stateString(snap_->state), line);
    }

    bool handleClick(double x, double y) override {
        for (const auto& btn : profileButtons_) {
            if (btn.rect.contains(x, y)) {
                profiles_->setActiveProfile(btn.name);
                return true;
            }
        }
        return false;
    }

private:
    struct ProfileBtn {
        Rect rect;
        std::string name;
    };

    // A segmented control of the daemon's profiles, anchored to the popover's
    // bottom so it never collides with the (variable-height) battery detail.
    void drawProfiles(Painter& p, const Rect& b) {
        profileButtons_.clear();
        if (!hasProfiles()) return;
        const auto& snap = profiles_->snapshot();
        const double pad = theme::statusbar::popoverPadding;
        const double innerW = b.w - 2 * pad;
        const double x = b.x + pad;

        double hy = b.y + b.h - 62.0;
        p.fillRect({x, hy - 12.0, innerW, 1.0}, theme::color::glassBorder);
        TextStyle hdr{theme::font::family, 11.0, PANGO_WEIGHT_BOLD, theme::color::textSubtle};
        p.drawText(x, hy, "POWER PROFILE", hdr);
        hy += 20.0;

        const size_t n = snap.profiles.size();
        if (n == 0) return;
        const double gap = 8.0;
        const double pw = (innerW - (n - 1) * gap) / n;
        const double ph = 30.0;
        for (size_t i = 0; i < n; ++i) {
            const std::string& name = snap.profiles[i];
            const Rect r{x + i * (pw + gap), hy, pw, ph};
            const bool active = name == snap.active;
            p.fillRoundedRect(r, 8.0, active ? theme::color::primary : theme::color::glass);
            p.strokeRoundedRect(r, 8.0, theme::color::glassBorder, 1.0);
            TextStyle ts{theme::font::family, 12.0, active ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL,
                         active ? Color::fromHex("#1e1e2e") : theme::color::text};
            p.drawText(r.x + r.w / 2.0, r.y + (ph - 14.0) / 2.0, profileLabel(name), ts,
                       HAlign::Center);
            profileButtons_.push_back({r, name});
        }
    }

    const BatterySnapshot* snap_;
    PowerProfilesBackend* profiles_ = nullptr;
    std::vector<ProfileBtn> profileButtons_;
};
}  // namespace

BatteryIndicator::BatteryIndicator(const SystemBackends& backends)
    : StatusIndicator("battery", Zone::Right, 500),
      backend_(backends.battery),
      profiles_(backends.powerProfiles) {
    // With a backend, stay hidden until it pushes real data (no fake 0%).
    // Without one (tests, registry previews) render the defaults.
    if (backend_) visible = false;
}

std::string BatteryIndicator::icon() const {
    bool charging = lastSnap_.state == BatterySnapshot::Charging ||
                    lastSnap_.state == BatterySnapshot::PendingCharge;
    return batteryIcon(lastSnap_.percentage, charging);
}

std::string BatteryIndicator::label() const {
    return std::to_string(lastSnap_.percentage) + "%";
}

std::string BatteryIndicator::tooltip() const {
    std::string tip = "Battery " + std::to_string(lastSnap_.percentage) + "% — " +
                      stateString(lastSnap_.state);
    return tip;
}

Color BatteryIndicator::iconColor() const {
    int pct = lastSnap_.percentage;
    if (pct > 50) return theme::color::success;
    if (pct > 20) return theme::color::warning;
    return theme::color::error;
}

void BatteryIndicator::onBackendUpdate() {
    if (!backend_) return;
    lastSnap_ = backend_->snapshot();
    visible = lastSnap_.present;
}

std::unique_ptr<QSTile> BatteryIndicator::createTile() {
    auto snap = &lastSnap_;
    return std::make_unique<QSInfoTile>(
        "Battery", icon(),
        [snap]() { return snap->percentage / 100.0; },
        [snap]() {
            std::string info = stateString(snap->state);
            if (snap->state == BatterySnapshot::Discharging && snap->timeToEmpty > 0) {
                info += " — " + formatTime(snap->timeToEmpty) + " remaining";
            } else if (snap->state == BatterySnapshot::Charging && snap->timeToFull > 0) {
                info += " — " + formatTime(snap->timeToFull) + " to full";
            }
            if (snap->energyRate > 0) {
                info += "  •  " + std::to_string(static_cast<int>(snap->energyRate)) + "W";
            }
            return info;
        });
}

std::unique_ptr<DetailedPopover> BatteryIndicator::createDetailedView() {
    return std::make_unique<BatteryPopover>(&lastSnap_, profiles_);
}

REGISTER_INDICATOR("battery", Zone::Right, 500, BatteryIndicator)

}  // namespace qypr
