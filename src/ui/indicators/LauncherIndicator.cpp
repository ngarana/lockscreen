// LauncherIndicator.cpp - Application launcher trigger implementation.
#include "ui/indicators/LauncherIndicator.hpp"

#include "system/DesktopIndex.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"
#include "ui/statusbar/LauncherPopover.hpp"

namespace qypr {

namespace {
constexpr const char* kAppsGlyph = "󰀻";  // nf-md-apps (grid)
}  // namespace

LauncherIndicator::LauncherIndicator(const SystemBackends& backends)
    : StatusIndicator("launcher", Zone::Left, 5),
      backend_(backends.desktopIndex) {
    // Present only when a DesktopIndex is supplied — i.e. on the unlocked bar.
    // The lock screen leaves it null, so the launcher never appears while locked.
    visible = backend_ != nullptr;
}

std::string LauncherIndicator::icon() const {
    return kAppsGlyph;
}

std::string LauncherIndicator::tooltip() const {
    return "Applications";
}

bool LauncherIndicator::hasDetailedView() const {
    return backend_ != nullptr;
}

std::unique_ptr<DetailedPopover> LauncherIndicator::createDetailedView() {
    if (!backend_) return nullptr;
    // Build the index here — this click is the first moment the user has asked
    // for it. Scanning every .desktop file in the XDG data dirs is real disk
    // I/O, and doing it at startup put that in front of the bar's first frame
    // for a popover that may never be opened. LauncherPopover's constructor
    // queries immediately, so the index must be populated before it is built.
    if (!backend_->loaded()) backend_->load();
    return std::make_unique<LauncherPopover>(backend_);
}

REGISTER_INDICATOR("launcher", Zone::Left, 5, LauncherIndicator)

}  // namespace qypr
