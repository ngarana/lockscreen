#include "notifications/NotificationActions.hpp"

#include <systemd/sd-bus.h>

#include "system/SystemBus.hpp"

namespace qypr {

void NotificationActions::close(uint32_t daemonId) {
    if (daemonId == 0 || !bus_.available()) return;

    // Async and reply-less: dismissing must never block the UI thread, and the
    // outcome arrives as a NotificationClosed signal on the monitor anyway.
    sd_bus_call_method_async(bus_.get(), nullptr, "org.freedesktop.Notifications",
                             "/org/freedesktop/Notifications", "org.freedesktop.Notifications",
                             "CloseNotification", nullptr, nullptr, "u", daemonId);
}

}  // namespace qypr
