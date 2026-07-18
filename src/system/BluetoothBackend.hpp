// BluetoothBackend.hpp - Bluetooth state via BlueZ on the shared system bus.
//
// One GetManagedObjects fetch at start(), then refreshes only when BlueZ
// pushes a relevant change: adapter Powered/PowerState, device Connected,
// or interfaces appearing/vanishing. Device RSSI chatter from discovery is
// filtered out before it can trigger a refetch. The power toggle writes the
// adapter's Powered property asynchronously.

#pragma once

#include <functional>
#include <string>
#include <vector>

struct sd_bus_message;
struct sd_bus_slot;
struct sd_bus_error;

namespace qypr {

class SystemBus;

// One paired/known device. `battery` is -1 when the device exposes no
// org.bluez.Battery1 interface; `icon` is a freedesktop icon name from BlueZ.
struct BtDevice {
    std::string path;
    std::string name;
    std::string icon;
    bool connected = false;
    bool paired = false;
    int battery = -1;

    bool operator==(const BtDevice&) const = default;
};

struct BluetoothSnapshot {
    bool available = false;    // BlueZ reachable and an adapter exists
    bool powered = false;
    int connectedCount = 0;
    std::string firstDevice;   // name of one connected device (tile subtitle)
    std::vector<BtDevice> devices;  // paired/known devices (for the picker)

    bool operator==(const BluetoothSnapshot&) const = default;
};

class BluetoothBackend {
public:
    explicit BluetoothBackend(SystemBus& bus);
    ~BluetoothBackend();

    BluetoothBackend(const BluetoothBackend&) = delete;
    BluetoothBackend& operator=(const BluetoothBackend&) = delete;

    // Returns false when BlueZ or an adapter is unavailable (indicator hides).
    bool start();

    const BluetoothSnapshot& snapshot() const { return snap_; }

    // Fires whenever a pushed update actually changed the snapshot.
    void setOnChange(std::function<void()> cb) { onChange_ = std::move(cb); }

    // Toggle the adapter (async Powered write, optimistic update).
    void setPowered(bool on);

    // Connect / disconnect a device by object path (async; the Connected
    // PropertiesChanged confirms and refreshes the list).
    void connectDevice(const std::string& path);
    void disconnectDevice(const std::string& path);

private:
    static int onPropsChanged(sd_bus_message* m, void* userdata, sd_bus_error* err);
    static int onInterfacesChanged(sd_bus_message* m, void* userdata, sd_bus_error* err);
    void refreshAndNotify();
    void refresh();   // GetManagedObjects → snapshot

    SystemBus& bus_;
    sd_bus_slot* propsSlot_ = nullptr;
    sd_bus_slot* ifacesSlot_ = nullptr;
    std::string adapter_;   // adapter object path (e.g. /org/bluez/hci0)
    BluetoothSnapshot snap_;
    std::function<void()> onChange_;
};

}  // namespace qypr
