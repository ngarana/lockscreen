// WifiIndicator.hpp - Status bar WiFi signal icon + QS toggle tile.
//
// Pure consumer of WifiBackend snapshots; the Quick Settings toggle flips
// the radio through the backend (async WirelessEnabled write).

#pragma once

#include "ui/statusbar/StatusIndicator.hpp"
#include "system/WifiBackend.hpp"
#include <string>

namespace qypr {

class WifiIndicator : public StatusIndicator {
public:
    explicit WifiIndicator(const SystemBackends& backends);

    std::string icon() const override;
    std::string tooltip() const override;

    void onBackendUpdate() override;

    std::unique_ptr<QSTile> createTile() override;

private:
    WifiBackend* backend_ = nullptr;
    WifiSnapshot lastSnap_;
};

}  // namespace qypr
