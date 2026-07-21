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

void NotificationActions::invoke(uint32_t daemonId, const std::string& actionKey) {
    if (daemonId == 0 || !bus_.available()) return;

    // Async: invoking an action must not block the UI thread.
    sd_bus_call_method_async(bus_.get(), nullptr, "org.freedesktop.Notifications",
                             "/org/freedesktop/Notifications", "org.freedesktop.Notifications",
                             "InvokeAction", nullptr, nullptr, "us", daemonId, actionKey.c_str());
}

}  // namespace qypr
