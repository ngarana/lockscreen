// BluetoothBackend.hpp - Bluetooth state via BlueZ on the shared system bus.
//
// Async startup: GetManagedObjects is issued via sd_bus_call_method_async so
// the event loop is never blocked. Push-only afterwards via PropertiesChanged.

#pragma once

#include <functional>
#include <string>
#include <vector>

struct sd_bus_message;
struct sd_bus_slot;
struct sd_bus_error;

namespace qypr {

class SystemBus;

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
    bool available = false;
    bool powered = false;
    int connectedCount = 0;
    std::string firstDevice;
    std::vector<BtDevice> devices;

    bool operator==(const BluetoothSnapshot&) const = default;
};

class BluetoothBackend {
public:
    explicit BluetoothBackend(SystemBus& bus);
    ~BluetoothBackend();

    BluetoothBackend(const BluetoothBackend&) = delete;
    BluetoothBackend& operator=(const BluetoothBackend&) = delete;

    bool start();

    const BluetoothSnapshot& snapshot() const { return snap_; }

    void setOnChange(std::function<void()> cb) { onChange_ = std::move(cb); }

    void setPowered(bool on);
    void connectDevice(const std::string& path);
    void disconnectDevice(const std::string& path);

private:
    static int onGetManagedObjects(sd_bus_message* reply, void* userdata, sd_bus_error* err);
    void parseManagedObjects(sd_bus_message* m);
    void subscribeSignals();

    static int onPropsChanged(sd_bus_message* m, void* userdata, sd_bus_error* err);
    static int onInterfacesChanged(sd_bus_message* m, void* userdata, sd_bus_error* err);

    SystemBus& bus_;
    sd_bus_slot* propsSlot_ = nullptr;
    sd_bus_slot* ifacesSlot_ = nullptr;
    std::string adapter_;
    BluetoothSnapshot snap_;
    std::function<void()> onChange_;
};

}  // namespace qypr
