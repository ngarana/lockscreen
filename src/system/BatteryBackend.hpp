// BatteryBackend.hpp - UPower battery state over the shared system bus.
//
// Async startup: GetAll is issued via sd_bus_call_method_async so the event
// loop is never blocked. Push-only afterwards via PropertiesChanged.

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
    int64_t timeToEmpty = 0;  // seconds
    int64_t timeToFull = 0;   // seconds
    double energyRate = 0.0;  // watts (power draw)
    std::string nativePath;   // e.g. "BAT0"
};

class BatteryBackend {
public:
    explicit BatteryBackend(SystemBus& bus);
    ~BatteryBackend();

    BatteryBackend(const BatteryBackend&) = delete;
    BatteryBackend& operator=(const BatteryBackend&) = delete;

    // Non-blocking: fires an async GetAll, returns immediately.
    // The indicator stays hidden until the first onChange_ fires with real data.
    bool start();

    const BatterySnapshot& snapshot() const { return snap_; }

    void setOnChange(std::function<void()> cb) { onChange_ = std::move(cb); }

    // True once the backend has produced its first result — real data or a
    // definitive "absent". Indicators show a neutral placeholder until then, so
    // an unrelated backend's push cannot prematurely mark this one loaded.
    bool ready() const { return ready_; }

private:
    // Async call chain: DisplayDevice GetAll → (fallback) EnumerateDevices → device GetAll
    static int onGetAllDisplay(sd_bus_message* reply, void* userdata, sd_bus_error* err);
    static int onEnumerateDevices(sd_bus_message* reply, void* userdata, sd_bus_error* err);
    static int onGetAllDevice(sd_bus_message* reply, void* userdata, sd_bus_error* err);
    void subscribeSignal();
    bool parseProps(sd_bus_message* m);

    static int onPropertiesChanged(sd_bus_message* m, void* userdata, sd_bus_error* err);

    SystemBus& bus_;
    sd_bus_slot* signalSlot_ = nullptr;
    std::string devicePath_;
    BatterySnapshot snap_;
    std::function<void()> onChange_;
    // Every result path calls this instead of onChange_ directly, so ready()
    // flips true exactly when the first real snapshot is published.
    void notifyReady() {
        ready_ = true;
        if (onChange_) onChange_();
    }
    bool ready_ = false;
};

}  // namespace qypr
