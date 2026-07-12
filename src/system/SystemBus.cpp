// SystemBus.cpp - Shared system-bus connection implementation.
#include "system/SystemBus.hpp"

#include <cstdio>

#include "core/EventLoop.hpp"

namespace qypr {

SystemBus::SystemBus(EventLoop& loop) : loop_(loop) {
    int r = sd_bus_open_system(&bus_);
    if (r < 0) {
        std::fprintf(stderr, "qypr: system bus unavailable (%d); system indicators disabled\n", r);
        bus_ = nullptr;
        return;
    }
    fd_ = sd_bus_get_fd(bus_);
    loop_.addFd(fd_, [this](uint32_t) { drain(); });
    drain();
}

SystemBus::~SystemBus() { teardown(); }

sd_bus_slot* SystemBus::addMatch(const char* rule, sd_bus_message_handler_t handler,
                                 void* userdata) {
    if (!bus_) return nullptr;
    sd_bus_slot* slot = nullptr;
    int r = sd_bus_add_match(bus_, &slot, rule, handler, userdata);
    if (r < 0) {
        std::fprintf(stderr, "qypr: sd_bus_add_match failed (%d)\n", r);
        return nullptr;
    }
    return slot;
}

void SystemBus::drain() {
    int r;
    while ((r = sd_bus_process(bus_, nullptr)) > 0) {
    }
    if (r < 0) {
        std::fprintf(stderr, "qypr: system bus error (%d); disconnecting\n", r);
        // Deferred: tearing down from inside the fd callback would destroy the
        // std::function currently executing.
        loop_.post([this] { teardown(); });
    }
}

void SystemBus::teardown() {
    if (fd_ >= 0) {
        loop_.removeFd(fd_);
        fd_ = -1;
    }
    if (bus_) {
        sd_bus_unref(bus_);
        bus_ = nullptr;
    }
}

}  // namespace qypr
