// BrightnessIndicator.hpp - Status bar brightness icon + QS slider + scroll.
//
// Pure consumer of BrightnessBackend snapshots; scroll on the icon and the
// Quick Settings slider write back through the backend (logind).

#pragma once

#include "ui/statusbar/StatusIndicator.hpp"
#include "system/BrightnessBackend.hpp"
#include <string>

namespace qypr {

class BrightnessIndicator : public StatusIndicator {
public:
    explicit BrightnessIndicator(const SystemBackends& backends);

    std::string icon() const override;
    std::string tooltip() const override;

    void onBackendUpdate() override;
    bool onScroll(double dx, double dy) override;

    std::unique_ptr<QSTile> createTile() override;

private:
    BrightnessBackend* backend_ = nullptr;
    BrightnessSnapshot lastSnap_;
};

}  // namespace qypr
