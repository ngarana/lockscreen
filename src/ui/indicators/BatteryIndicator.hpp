// BatteryIndicator.hpp - Status bar battery icon + QS tile + detailed popover.
//
// Pure consumer: BatteryBackend pushes snapshots via onBackendUpdate();
// the indicator never touches D-Bus and stays hidden until real data arrives.

#pragma once

#include "ui/statusbar/StatusIndicator.hpp"
#include "system/BatteryBackend.hpp"
#include <string>

namespace qypr {

class BatteryIndicator : public StatusIndicator {
public:
    explicit BatteryIndicator(const SystemBackends& backends);

    std::string icon() const override;
    std::string label() const override;
    std::string tooltip() const override;
    Color iconColor() const override;

    void onBackendUpdate() override;

    std::unique_ptr<QSTile> createTile() override;
    bool hasDetailedView() const override { return true; }
    std::unique_ptr<DetailedPopover> createDetailedView() override;

private:
    BatteryBackend* backend_ = nullptr;
    BatterySnapshot lastSnap_;
};

}  // namespace qypr
