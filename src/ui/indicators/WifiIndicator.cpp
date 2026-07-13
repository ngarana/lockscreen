// WifiIndicator.cpp - Status bar WiFi indicator implementation.
#include "ui/indicators/WifiIndicator.hpp"

#include "ui/statusbar/IndicatorRegistry.hpp"
#include "ui/statusbar/QSTile.hpp"

namespace qypr {

namespace {
const char* wifiIcon(const WifiSnapshot& s) {
    if (!s.enabled) return "󰤮";
    if (!s.connected) return "󰤭";
    if (s.strength >= 75) return "󰤨";
    if (s.strength >= 50) return "󰤥";
    if (s.strength >= 25) return "󰤢";
    return "󰤯";
}
}  // namespace

WifiIndicator::WifiIndicator(const SystemBackends& backends)
    : StatusIndicator("wifi", Zone::Right, 300), backend_(backends.wifi) {
    // Hidden until the backend pushes real data; render defaults without one.
    if (backend_) visible = false;
}

std::string WifiIndicator::icon() const {
    return wifiIcon(lastSnap_);
}

std::string WifiIndicator::tooltip() const {
    if (!lastSnap_.enabled) return "WiFi off";
    if (!lastSnap_.connected) return "WiFi disconnected";
    return lastSnap_.ssid + " (" + std::to_string(lastSnap_.strength) + "%)";
}

void WifiIndicator::onBackendUpdate() {
    if (!backend_) return;
    lastSnap_ = backend_->snapshot();
    visible = lastSnap_.available;
}

std::unique_ptr<QSTile> WifiIndicator::createTile() {
    auto snap = &lastSnap_;
    auto backend = backend_;
    return std::make_unique<QSToggleTile>(
        "WiFi", "󰤨",
        [snap]() { return snap->enabled; },
        [backend, snap]() {
            if (backend) backend->setEnabled(!snap->enabled);
        },
        [snap]() -> std::string {
            if (!snap->enabled) return "Off";
            if (!snap->connected) return "Not connected";
            return snap->ssid;
        });
}

REGISTER_INDICATOR("wifi", Zone::Right, 300, WifiIndicator)

}  // namespace qypr
