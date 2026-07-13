// BluetoothIndicator.cpp - Status bar Bluetooth indicator implementation.
#include "ui/indicators/BluetoothIndicator.hpp"

#include "ui/Theme.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"
#include "ui/statusbar/QSTile.hpp"

namespace qypr {

BluetoothIndicator::BluetoothIndicator(const SystemBackends& backends)
    : StatusIndicator("bluetooth", Zone::Right, 350), backend_(backends.bluetooth) {
    // Hidden until the backend pushes real data; render defaults without one.
    if (backend_) visible = false;
}

std::string BluetoothIndicator::icon() const {
    if (!lastSnap_.powered) return "󰂲";
    return lastSnap_.connectedCount > 0 ? "󰂱" : "󰂯";
}

std::string BluetoothIndicator::tooltip() const {
    if (!lastSnap_.powered) return "Bluetooth off";
    if (lastSnap_.connectedCount == 0) return "Bluetooth on";
    if (lastSnap_.connectedCount == 1) return "Connected: " + lastSnap_.firstDevice;
    return std::to_string(lastSnap_.connectedCount) + " devices connected";
}

Color BluetoothIndicator::iconColor() const {
    // Blue accent while something is connected.
    if (lastSnap_.powered && lastSnap_.connectedCount > 0) return theme::color::primary;
    return theme::color::text;
}

void BluetoothIndicator::onBackendUpdate() {
    if (!backend_) return;
    lastSnap_ = backend_->snapshot();
    visible = lastSnap_.available;
}

std::unique_ptr<QSTile> BluetoothIndicator::createTile() {
    auto snap = &lastSnap_;
    auto backend = backend_;
    return std::make_unique<QSToggleTile>(
        "Bluetooth", "󰂯",
        [snap]() { return snap->powered; },
        [backend, snap]() {
            if (backend) backend->setPowered(!snap->powered);
        },
        [snap]() -> std::string {
            if (!snap->powered) return "Off";
            if (snap->connectedCount == 0) return "No devices";
            if (snap->connectedCount == 1) return snap->firstDevice;
            return std::to_string(snap->connectedCount) + " devices";
        });
}

REGISTER_INDICATOR("bluetooth", Zone::Right, 350, BluetoothIndicator)

}  // namespace qypr
