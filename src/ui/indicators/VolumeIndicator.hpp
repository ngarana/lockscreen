// VolumeIndicator.hpp - Status bar volume icon + QS slider + scroll.
//
// Pure consumer of VolumeBackend snapshots; scroll on the icon and the
// Quick Settings slider write back through the backend (libpulse).

#pragma once

#include "ui/statusbar/StatusIndicator.hpp"
#include "system/VolumeBackend.hpp"
#include <string>

namespace qypr {

class VolumeIndicator : public StatusIndicator {
public:
    explicit VolumeIndicator(const SystemBackends& backends);

    std::string icon() const override;
    std::string tooltip() const override;

    void onBackendUpdate() override;
    bool onScroll(double dx, double dy) override;

    std::unique_ptr<QSTile> createTile() override;

private:
    VolumeBackend* backend_ = nullptr;
    VolumeSnapshot lastSnap_;
};

}  // namespace qypr
