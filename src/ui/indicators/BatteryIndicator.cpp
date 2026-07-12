// BatteryIndicator.cpp - Status bar battery indicator implementation.
#include "ui/indicators/BatteryIndicator.hpp"

#include "render/Painter.hpp"
#include "ui/Theme.hpp"
#include "ui/statusbar/DetailedPopover.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"
#include "ui/statusbar/QSTile.hpp"

namespace qypr {

namespace {
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
    explicit BatteryPopover(const BatterySnapshot* snap) : snap_(snap) {}

    double contentHeight() const override { return 148.0; }

    void draw(Painter& p, int64_t now) override {
        Rect b = getBounds();
        // Slide down 6px while opening (fade is applied by PopoverManager).
        b.y -= (1.0 - openProgress_.value(now)) * 6.0;

        p.fillRoundedRect(b, theme::statusbar::popoverRadius, theme::color::glass);
        p.strokeRoundedRect(b, theme::statusbar::popoverRadius, theme::color::glassBorder, 1.0);

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

private:
    const BatterySnapshot* snap_;
};
}  // namespace

BatteryIndicator::BatteryIndicator(const SystemBackends& backends)
    : StatusIndicator("battery", Zone::Right, 500), backend_(backends.battery) {
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
    return std::make_unique<BatteryPopover>(&lastSnap_);
}

REGISTER_INDICATOR("battery", Zone::Right, 500, BatteryIndicator)

}  // namespace qypr
