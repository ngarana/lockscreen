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

    // True once the backend has produced its first result — real data or a
    // definitive "absent". Indicators show a neutral placeholder until then, so
    // an unrelated backend's push cannot prematurely mark this one loaded.
    bool ready() const { return ready_; }

    // Seed from the previous session's persisted snapshot (see StateCache).
    // The daemon that owns this state is often not running yet when the bar
    // starts — UPower in particular is D-Bus-activated and comes up *after* it
    // — so without a seed the indicator sits on its neutral "unknown" glyph for
    // seconds. Seeding marks the backend ready() so the very first frame
    // carries real values; the first live reply overwrites both the snapshot
    // and this flag. A no-op once a live reply has landed.
    void seed(const BluetoothSnapshot& s) {
        if (ready_) { return; }
        snap_ = s;
        ready_ = true;
    }

    void setPowered(bool on);
    void connectDevice(const std::string& path);
    void disconnectDevice(const std::string& path);

private:
    static int onGetManagedObjects(sd_bus_message* reply, void* userdata, sd_bus_error* err);
    void parseManagedObjects(sd_bus_message* m);
    void refetch();
    void endFetch();
    void subscribeSignals();
    void sendSetPowered(bool on);
    static int onSetPowerReply(sd_bus_message* reply, void* userdata, sd_bus_error* err);

    static int onPropsChanged(sd_bus_message* m, void* userdata, sd_bus_error* err);
    static int onInterfacesChanged(sd_bus_message* m, void* userdata, sd_bus_error* err);
    static int onNameOwnerChanged(sd_bus_message* m, void* userdata, sd_bus_error* err);

    SystemBus& bus_;
    sd_bus_slot* propsSlot_ = nullptr;
    sd_bus_slot* ifacesSlot_ = nullptr;
    sd_bus_slot* ownerSlot_ = nullptr;  // BlueZ service (re)appearance
    std::string adapter_;
    // Refetch serialization: one GetManagedObjects in flight at a time; a
    // signal during a fetch only marks pendingFetch_ (interleaved replies used
    // to regress the snapshot to stale state).
    bool fetchInFlight_ = false;
    bool pendingFetch_ = false;
    bool subscribed_ = false;
    // A toggle requested while the adapter path was still unknown: applied once
    // a successful fetch (or BlueZ (re)appearance) has produced an adapter.
    bool pendingPowerSet_ = false;
    bool pendingPowerOn_ = false;
    BluetoothSnapshot snap_;
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
