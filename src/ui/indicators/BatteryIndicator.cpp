// BatteryIndicator.cpp - Status bar battery indicator implementation.
#include "ui/indicators/BatteryIndicator.hpp"

#include "render/Painter.hpp"
#include "ui/Theme.hpp"
#include "ui/statusbar/QSTile.hpp"
#include "ui/statusbar/DetailedPopover.hpp"

namespace qypr {

namespace {
constexpr const char* kDischargingIcons[] = {
    "󰂎", "󰁺", "󰁻", "󰁼", "󰁽", "󰁾", "󰁿", "󰂀", "󰂁", "󰂂", "󰁹"
};
constexpr const char* kChargingIcons[] = {
    "󰢟", "󰢜", "󰂆", "󰂇", "󰂈", "󰢝", "󰂉", "󰂊", "󰂋", "󰂅", "󰢟"
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
}  // namespace

BatteryIndicator::BatteryIndicator(const SystemBackends& backends)
    : StatusIndicator("battery", Zone::Right, 500),
      backend_(backends.battery) {}

std::string BatteryIndicator::icon() const {
    bool charging = lastSnap_.state == BatterySnapshot::Charging;
    return batteryIcon(lastSnap_.percentage, charging);
}

std::string BatteryIndicator::tooltip() const {
    return "Battery " + std::to_string(lastSnap_.percentage) + "%";
}

Color BatteryIndicator::iconColor() const {
    int pct = lastSnap_.percentage;
    if (pct > 50) return theme::color::success;
    if (pct > 20) return theme::color::warning;
    return theme::color::error;
}

double BatteryIndicator::measureWidth(Painter& p) {
    TextStyle style{theme::font::iconFamily, theme::statusbar::iconSize, PANGO_WEIGHT_NORMAL, iconColor()};
    Size iconSz = p.measureText(icon(), style);
    TextStyle pctStyle{theme::font::family, 12.0, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
    std::string pctText = std::to_string(lastSnap_.percentage) + "%";
    Size pctSz = p.measureText(pctText, pctStyle);
    return iconSz.w + 6.0 + pctSz.w + 16.0;
}

void BatteryIndicator::poll(int64_t) {
    if (!backend_) return;
    backend_->refresh();
    backend_->processEvents();
    lastSnap_ = backend_->snapshot();
}

void BatteryIndicator::onBackendUpdate() {
    if (backend_) {
        lastSnap_ = backend_->snapshot();
    }
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
    // TODO: Implement detailed battery popover with charging animation
    return nullptr;
}

}  // namespace qypr
