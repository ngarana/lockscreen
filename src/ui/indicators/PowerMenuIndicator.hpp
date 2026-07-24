// PowerMenuIndicator.hpp - Session/power menu for the unlocked bar.
//
// Surfaces the existing PowerManager (lock / suspend / hibernate / reboot /
// shut down) as a bar applet. Session-sensitive: the lock screen must never
// offer these — it has its own PowerDialog behind the reveal, and a shutdown
// button on a locked machine is a footgun. The indicator hides itself entirely
// when no PowerManager is supplied (qypr-lock supplies none).
//
// Destructive actions require a second click to confirm, so a stray click on a
// panel button can never power the machine off.

#pragma once

#include <string>

#include "ui/statusbar/StatusIndicator.hpp"

namespace qypr {

class PowerManager;

class PowerMenuIndicator : public StatusIndicator {
public:
    explicit PowerMenuIndicator(const SystemBackends& backends);

    std::string icon() const override { return "󰐥"; }  // nf-md-power
    std::string tooltip() const override { return "Session"; }
    Color iconColor() const override;
    bool qsOnly() const override { return true; }

    bool hasDetailedView() const override { return true; }
    std::unique_ptr<DetailedPopover> createDetailedView() override;

    // Reveals nothing by itself, but it *acts on* the session: never on a lock
    // screen.
    bool sensitive() const override { return true; }

private:
    PowerManager* power_ = nullptr;
};

}  // namespace qypr
