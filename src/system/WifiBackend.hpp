// WifiBackend.hpp - WiFi state via NetworkManager on the shared system bus.
//
// One startup fetch, then refreshes only when NetworkManager pushes a
// PropertiesChanged signal for an object we care about (the NM root, the
// wireless device, or the active access point) — no polling. The enable
// toggle writes WirelessEnabled asynchronously.

#pragma once

#include <functional>
#include <string>

struct sd_bus_message;
struct sd_bus_slot;
struct sd_bus_error;

namespace qypr {

class SystemBus;

struct WifiSnapshot {
    bool available = false;   // NetworkManager reachable and a WiFi device exists
    bool enabled = false;     // radio switch (WirelessEnabled)
    bool connected = false;   // has an active access point
    std::string ssid;
    int strength = 0;         // 0-100

    bool operator==(const WifiSnapshot&) const = default;
};

class WifiBackend {
public:
    explicit WifiBackend(SystemBus& bus);
    ~WifiBackend();

    WifiBackend(const WifiBackend&) = delete;
    WifiBackend& operator=(const WifiBackend&) = delete;

    // Returns false when NetworkManager or a WiFi device is unavailable
    // (indicator stays hidden).
    bool start();

    const WifiSnapshot& snapshot() const { return snap_; }

    // Fires whenever a pushed update actually changed the snapshot.
    void setOnChange(std::function<void()> cb) { onChange_ = std::move(cb); }

    // Toggle the radio (async WirelessEnabled write, optimistic update).
    void setEnabled(bool on);

private:
    static int onPropsChanged(sd_bus_message* m, void* userdata, sd_bus_error* err);
    void refresh();                 // targeted property reads → snapshot
    std::string findWifiDevice();   // GetDevices → DeviceType == 2

    SystemBus& bus_;
    sd_bus_slot* slot_ = nullptr;
    std::string device_;            // WiFi device object path
    std::string activeAp_;          // active AP object path ("/" when none)
    WifiSnapshot snap_;
    std::function<void()> onChange_;
};

}  // namespace qypr
