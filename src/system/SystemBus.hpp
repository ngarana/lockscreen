// SystemBus.hpp - Shared sd-bus system-bus connection for status bar backends.
//
// One connection, one fd in the epoll EventLoop, any number of subscribers
// (minimal-footprint principle: one bus connection per bus). Backends add
// signal matches through addMatch() and may issue their single startup fetch
// on get(); after that everything is push — no polling.

#pragma once

#include <systemd/sd-bus.h>

namespace qypr {

class EventLoop;

class SystemBus {
public:
    explicit SystemBus(EventLoop& loop);
    ~SystemBus();

    SystemBus(const SystemBus&) = delete;
    SystemBus& operator=(const SystemBus&) = delete;

    // nullptr when the system bus is unavailable; backends degrade, non-fatal.
    sd_bus* get() const { return bus_; }
    bool available() const { return bus_ != nullptr; }

    // Subscribe to a match rule. The returned slot is owned by the caller
    // (sd_bus_slot_unref to unsubscribe); nullptr on failure.
    sd_bus_slot* addMatch(const char* rule, sd_bus_message_handler_t handler, void* userdata);

private:
    void drain();
    void teardown();

    EventLoop& loop_;
    sd_bus* bus_ = nullptr;
    int fd_ = -1;
};

}  // namespace qypr
