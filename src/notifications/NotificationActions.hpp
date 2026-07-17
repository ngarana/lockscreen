// NotificationActions.hpp - Send-side companion to NotificationMonitor.
//
// NotificationMonitor's connection is a *monitor* (BecomeMonitor): the D-Bus
// spec forbids it from ever sending a message, so it can watch notifications but
// can never dismiss one. This class is the other half — a normal caller on the
// shared session bus that invokes the daemon's CloseNotification.
//
// Daemon-agnostic: org.freedesktop.Notifications is the freedesktop spec
// interface every daemon implements (SwayNC, dunst, mako, …), not a
// daemon-specific API (STATUS_BAR.md decision D6).
//
// Fire-and-forget: the daemon answers by emitting NotificationClosed, which the
// monitor already observes — so the card disappears through the normal push
// path and this class holds no state of its own.

#pragma once

#include <cstdint>

namespace qypr {

class SystemBus;

class NotificationActions {
public:
    explicit NotificationActions(SystemBus& sessionBus) : bus_(sessionBus) {}

    // Ask the daemon to close `daemonId` (the id it assigned via its Notify
    // reply). A zero id means the reply has not landed yet — nothing to close.
    void close(uint32_t daemonId);

private:
    SystemBus& bus_;
};

}  // namespace qypr
