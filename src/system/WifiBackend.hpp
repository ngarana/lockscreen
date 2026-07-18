// WifiBackend.hpp - WiFi state via NetworkManager on the shared system bus.
//
// One startup fetch, then refreshes only when NetworkManager pushes a
// PropertiesChanged signal for an object we care about (the NM root, the
// wireless device, or the active access point) — no polling. The enable
// toggle writes WirelessEnabled asynchronously.

#pragma once

#include <functional>
#include <string>
#include <vector>

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

// One nearby access point, as shown in the picker. `saved` means NM already has
// a connection profile for this SSID, so it can be joined without a secret agent
// (unsaved secured networks would need one — out of scope, so they are shown but
// not joinable).
struct WifiAp {
    std::string ssid;
    int strength = 0;   // 0-100
    bool secured = false;
    bool active = false;
    bool saved = false;
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

    // On-demand enumeration of nearby access points for the picker (deduped by
    // SSID keeping the strongest, sorted strength-desc, active first). Reads the
    // bus each call — the popover fetches once on open, not per frame.
    std::vector<WifiAp> scanNetworks() const;
    // Ask NM to rescan (async, best-effort); results arrive on later refreshes.
    void requestScan();
    // Join a *saved* network by SSID (ActivateConnection — NM has the secret, so
    // no agent). No-op when nothing saved matches. Async.
    void connectSsid(const std::string& ssid);
    // Drop the current wireless connection (Device.Disconnect, async).
    void disconnect();

private:
    static int onPropsChanged(sd_bus_message* m, void* userdata, sd_bus_error* err);
    void refresh();                 // targeted property reads → snapshot
    std::string findWifiDevice();   // GetDevices → DeviceType == 2
    // Saved connection object path whose 802-11-wireless.ssid matches, or "".
    std::string findSavedConnection(const std::string& ssid) const;

    SystemBus& bus_;
    sd_bus_slot* slot_ = nullptr;
    std::string device_;            // WiFi device object path
    std::string activeAp_;          // active AP object path ("/" when none)
    WifiSnapshot snap_;
    std::function<void()> onChange_;
};

}  // namespace qypr
