// BatteryBackend.hpp - UPower battery state over the shared system bus.
//
// Spec-correct per docs/STATUS_BAR.md: one synchronous GetAll at start(),
// push-only afterwards via org.freedesktop.DBus.Properties.PropertiesChanged.
// UPower lives on the SYSTEM bus and its State property is a uint32 enum.

#pragma once

#include <cstdint>
#include <functional>
#include <string>

struct sd_bus_message;
struct sd_bus_slot;
struct sd_bus_error;

namespace qypr {

class SystemBus;

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
    explicit BatteryBackend(SystemBus& bus);
    ~BatteryBackend();

    BatteryBackend(const BatteryBackend&) = delete;
    BatteryBackend& operator=(const BatteryBackend&) = delete;

    // One startup fetch + PropertiesChanged subscription. Returns false when
    // the bus, UPower, or a battery is unavailable (indicator stays hidden).
    bool start();

    const BatterySnapshot& snapshot() const { return snap_; }

    // Fires on every pushed update (and once after the successful start fetch).
    void setOnChange(std::function<void()> cb) { onChange_ = std::move(cb); }

private:
    static int onPropertiesChanged(sd_bus_message* m, void* userdata, sd_bus_error* err);
    bool fetchAll(const char* devicePath);
    bool parseProps(sd_bus_message* m);   // a{sv} at cursor → snapshot fields
    std::string findBatteryDevice();      // fallback: EnumerateDevices, Type==2

    SystemBus& bus_;
    sd_bus_slot* slot_ = nullptr;
    std::string devicePath_;
    BatterySnapshot snap_;
    std::function<void()> onChange_;
};

}  // namespace qypr
