// VolumeIndicator.cpp - Status bar volume indicator implementation.
#include "ui/indicators/VolumeIndicator.hpp"

#include "ui/statusbar/IndicatorRegistry.hpp"
#include "ui/statusbar/QSTile.hpp"

namespace qypr {

namespace {
constexpr double kScrollStep = 0.05;  // ±5% per scroll tick

const char* volumeIcon(const VolumeSnapshot& s) {
    if (s.muted || s.level <= 0.001) return "󰝟";
    if (s.level >= 0.66) return "󰕾";
    if (s.level >= 0.33) return "󰖀";
    return "󰕿";
}
}  // namespace

VolumeIndicator::VolumeIndicator(const SystemBackends& backends)
    : StatusIndicator("volume", Zone::Right, 200), backend_(backends.volume) {
    // Hidden until the backend pushes real data; render defaults without one.
    if (backend_) visible = false;
}

std::string VolumeIndicator::icon() const {
    return volumeIcon(lastSnap_);
}

std::string VolumeIndicator::tooltip() const {
    if (lastSnap_.muted) return "Muted — " + lastSnap_.sinkName;
    return "Volume " + std::to_string(static_cast<int>(lastSnap_.level * 100 + 0.5)) + "% — " +
           lastSnap_.sinkName;
}

void VolumeIndicator::onBackendUpdate() {
    if (!backend_) return;
    lastSnap_ = backend_->snapshot();
    visible = lastSnap_.available;
}

bool VolumeIndicator::onScroll(double dx, double dy) {
    (void)dx;
    if (!backend_ || !lastSnap_.available) return false;
    // Scroll up (negative dy in Wayland) raises the volume.
    const double delta = dy < 0 ? kScrollStep : -kScrollStep;
    backend_->setLevel(lastSnap_.level + delta);
    return true;
}

std::unique_ptr<QSTile> VolumeIndicator::createTile() {
    auto snap = &lastSnap_;
    auto backend = backend_;
    return std::make_unique<QSSliderTile>(
        "󰕾",
        [snap]() { return snap->level; },
        [backend](double v) {
            if (backend) backend->setLevel(v);
        });
}

REGISTER_INDICATOR("volume", Zone::Right, 200, VolumeIndicator)

}  // namespace qypr
