// BatteryIndicator.hpp - Status bar battery icon + popover.
#pragma once

#include "ui/statusbar/StatusIndicator.hpp"
#include "system/BatteryBackend.hpp"
#include <string>

namespace qypr {

class BatteryIndicator : public StatusIndicator {
public:
    BatteryIndicator(const SystemBackends& backends);

    std::string icon() const override;
    std::string tooltip() const override;
    Color iconColor() const override;
    double measureWidth(Painter& p) override;

    void poll(int64_t now) override;
    void onBackendUpdate() override;

    std::unique_ptr<QSTile> createTile() override;
    bool hasDetailedView() const override { return true; }
    std::unique_ptr<DetailedPopover> createDetailedView() override;

private:
    std::string iconForLevel(bool charging) const;

    BatteryBackend* backend_ = nullptr;
    BatterySnapshot lastSnap_;
};

}  // namespace qypr
