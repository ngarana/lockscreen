// NotificationIndicator.hpp - Notification centre for the unlocked bar.
//
// A bell with a count, plus a history popover, over the existing
// NotificationMonitor — the same daemon-agnostic D-Bus observer the lock screen
// uses (SwayNC, dunst, mako, …). No daemon-specific client is involved
// (STATUS_BAR.md decision D6), so this works wherever the monitor does.
//
// Session-sensitive: notification titles and bodies are exactly the content the
// lock screen redacts, so this applet must never appear while locked. The lock
// screen keeps its own notification stack behind the reveal.

#pragma once

#include <string>

#include "ui/statusbar/StatusIndicator.hpp"

namespace qypr {

class NotificationMonitor;
class NotificationActions;
class DndState;

class NotificationIndicator : public StatusIndicator {
public:
    explicit NotificationIndicator(const SystemBackends& backends);

    std::string icon() const override;
    std::string themedIcon() const override;
    std::string label() const override;
    std::string tooltip() const override;
    Color iconColor() const override;

    bool hasDetailedView() const override { return true; }
    std::unique_ptr<DetailedPopover> createDetailedView() override;

    void onBackendUpdate() override;

    // Reveals notification content: never on the lock screen.
    bool sensitive() const override { return true; }

private:
    size_t count() const;

    NotificationMonitor* monitor_ = nullptr;
    NotificationActions* actions_ = nullptr;  // dismiss/clear (null → read-only)
    DndState* dnd_ = nullptr;
};

}  // namespace qypr
