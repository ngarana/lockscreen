// BatteryBackend.hpp - UPower D-Bus monitor for battery state.
#pragma once

#include "core/Types.hpp"
#include <string>

struct sd_bus;
struct sd_bus_slot;

namespace qypr {

struct BatterySnapshot {
    int percentage = 0;
    enum State { Unknown, Charging, Discharging, Full, PendingCharge } state = Unknown;
    bool present = false;
    int64_t timeToEmpty = 0;    // seconds
    int64_t timeToFull = 0;     // seconds
    double energyRate = 0.0;    // watts (power draw)
    std::string nativePath;     // e.g. "BAT0"
};

class BatteryBackend {
public:
    BatteryBackend();
    ~BatteryBackend();

    // Poll current state (called on timer tick).
    void refresh();

    // Get latest snapshot.
    const BatterySnapshot& snapshot() const { return snap_; }

    // D-Bus file descriptor for polling (returns -1 if unavailable).
    int dbusFd() const;

    // Process D-Bus events (call after poll).
    void processEvents();

private:
    void pollFromDevice();
    void parseProperties(const char* props);

    BatterySnapshot snap_;
    sd_bus* bus_ = nullptr;
    sd_bus_slot* slot_ = nullptr;
};

}  // namespace qypr
