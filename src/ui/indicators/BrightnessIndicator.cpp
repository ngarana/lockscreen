// BrightnessIndicator.cpp - Status bar brightness indicator implementation.
#include "ui/indicators/BrightnessIndicator.hpp"

#include "ui/statusbar/IndicatorRegistry.hpp"
#include "ui/statusbar/QSTile.hpp"

namespace qypr {

namespace {
constexpr double kScrollStep = 0.05;  // ±5% per scroll tick

const char* brightnessIcon(double frac) {
    if (frac >= 0.66) return "󰃠";
    if (frac >= 0.33) return "󰃟";
    return "󰃞";
}
}  // namespace

BrightnessIndicator::BrightnessIndicator(const SystemBackends& backends)
    : StatusIndicator("brightness", Zone::Right, 100), backend_(backends.brightness) {
    // Hidden until the backend pushes real data; render defaults without one.
    if (backend_) visible = false;
}

std::string BrightnessIndicator::icon() const {
    return brightnessIcon(lastSnap_.fraction());
}

std::string BrightnessIndicator::tooltip() const {
    return "Brightness " + std::to_string(static_cast<int>(lastSnap_.fraction() * 100 + 0.5)) +
           "%";
}

void BrightnessIndicator::onBackendUpdate() {
    if (!backend_) return;
    lastSnap_ = backend_->snapshot();
    visible = lastSnap_.available;
}

bool BrightnessIndicator::onScroll(double dx, double dy, double x, double y) {
    (void)x; (void)y;
    (void)dx;
    if (!backend_ || !lastSnap_.available) return false;
    // Scroll up (negative dy in Wayland) brightens.
    const double delta = dy < 0 ? kScrollStep : -kScrollStep;
    backend_->setFraction(lastSnap_.fraction() + delta);
    return true;
}

std::unique_ptr<QSTile> BrightnessIndicator::createTile() {
    auto snap = &lastSnap_;
    auto backend = backend_;
    return std::make_unique<QSSliderTile>(
        "󰃠",
        [snap]() { return snap->fraction(); },
        [backend](double v) {
            if (backend) backend->setFraction(v);
        });
}

REGISTER_INDICATOR("brightness", Zone::Right, 100, BrightnessIndicator)

}  // namespace qypr
