// LauncherIndicator.hpp - Application launcher trigger (Phase 15).
//
// A single bar button that opens a keyboard-driven app launcher (LauncherPopover
// over DesktopIndex). Bar-only: the lock screen supplies no DesktopIndex, so the
// indicator hides itself and can never spawn an application while locked.
#pragma once

#include <string>

#include "ui/statusbar/StatusIndicator.hpp"

namespace qypr {

class DesktopIndex;

class LauncherIndicator : public StatusIndicator {
public:
    explicit LauncherIndicator(const SystemBackends& backends);

    std::string icon() const override;
    std::string tooltip() const override;

    bool hasDetailedView() const override;
    std::unique_ptr<DetailedPopover> createDetailedView() override;

private:
    DesktopIndex* backend_ = nullptr;
};

}  // namespace qypr
