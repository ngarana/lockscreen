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

    void draw(Painter& p, int64_t now) override;
    void poll(int64_t now) override;
    void onBackendUpdate() override;

    // The 1Hz charging pulse drives a redraw every frame; while charging we
    // report "animating" so the host keeps the loop alive at full rate.
    bool animating(int64_t now) const override {
        return StatusIndicator::animating(now) || charging_;
    }

    std::unique_ptr<QSTile> createTile() override;
    bool hasDetailedView() const override { return true; }
    std::unique_ptr<DetailedPopover> createDetailedView() override;

private:
    BatteryBackend* backend_ = nullptr;
    PowerProfilesBackend* profiles_ = nullptr;  // null on the lock screen
    BatterySnapshot lastSnap_;
    // Phase 6 polish: charging-pulse glow. Re-evaluated each poll() frame so a
    // freshly-snapshot even when the icon glyph itself did not change still
    // drives the 1Hz oscillation.
    bool charging_ = false;
};

}  // namespace qypr
