// BluetoothBackend.cpp - BlueZ monitor implementation (async startup, push-driven).
#include "system/BluetoothBackend.hpp"

#include <systemd/sd-bus.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <utility>

#include "system/SystemBus.hpp"

namespace qypr {

namespace {
constexpr const char* kBlueZ = "org.bluez";
constexpr const char* kAdapterIface = "org.bluez.Adapter1";
constexpr const char* kDeviceIface = "org.bluez.Device1";
constexpr const char* kBatteryIface = "org.bluez.Battery1";
constexpr const char* kPropsIface = "org.freedesktop.DBus.Properties";
constexpr const char* kObjectManagerIface = "org.freedesktop.DBus.ObjectManager";

bool dictHasKey(sd_bus_message* m, std::initializer_list<const char*> keys) {
    bool found = false;
    if (sd_bus_message_enter_container(m, 'a', "{sv}") < 0) { return false; }
    while (sd_bus_message_enter_container(m, 'e', "sv") > 0) {
        const char* key = nullptr;
        if (sd_bus_message_read(m, "s", &key) >= 0 && (key != nullptr)) {
            for (const char* k : keys) {
                if (std::strcmp(key, k) == 0) { found = true; }
            }
        }
        sd_bus_message_skip(m, "v");
        sd_bus_message_exit_container(m);
    }
    sd_bus_message_exit_container(m);
    return found;
}
}  // namespace

BluetoothBackend::BluetoothBackend(SystemBus& bus) : bus_(bus) {}

BluetoothBackend::~BluetoothBackend() {
    sd_bus_slot_unref(propsSlot_);
    sd_bus_slot_unref(ifacesSlot_);
    sd_bus_slot_unref(ownerSlot_);
}

bool BluetoothBackend::start() {
    if (!bus_.available()) { return false; }

    // Subscribe before the first fetch so no state change can fall in the gap
    // (standard subscribe-then-fetch order).
    subscribeSignals();
    refetch();
    return true;
}

// One GetManagedObjects in flight at a time: a signal during a fetch only marks
// pendingFetch_, and the next fetch runs when the current reply lands. This is
// what keeps two replies from arriving out of order and regressing the
// snapshot to stale state.
void BluetoothBackend::refetch() {
    if (!bus_.available()) { return; }
    if (fetchInFlight_) {
        pendingFetch_ = true;
        return;
    }
    fetchInFlight_ = true;
    sd_bus_call_method_async(bus_.get(), nullptr, kBlueZ, "/", kObjectManagerIface,
                             "GetManagedObjects", &BluetoothBackend::onGetManagedObjects, this, "");
}

void BluetoothBackend::endFetch() {
    fetchInFlight_ = false;
    if (pendingFetch_) {
        pendingFetch_ = false;
        refetch();
    }
}

int BluetoothBackend::onGetManagedObjects(sd_bus_message* reply, void* userdata,
                                          sd_bus_error* /*unused*/) {
    auto* self = static_cast<BluetoothBackend*>(userdata);
    if (sd_bus_message_is_method_error(reply, nullptr) != 0) {
        const sd_bus_error* e = sd_bus_message_get_error(reply);
        std::fprintf(stderr, "qypr: BlueZ unavailable (%s: %s); bluetooth indicator disabled\n",
                     e != nullptr && e->name != nullptr ? e->name : "unknown",
                     e != nullptr && e->message != nullptr ? e->message : "");
        self->snap_.available = false;
        self->notifyReady();  // hide the placeholder
        self->endFetch();
        return 0;
    }

    self->parseManagedObjects(reply);
    if (!self->snap_.available) {
        std::fprintf(stderr, "qypr: no Bluetooth adapter via BlueZ; indicator disabled\n");
        self->notifyReady();  // hide the placeholder
        self->endFetch();
        return 0;
    }

    self->notifyReady();
    self->endFetch();
    return 0;
}

void BluetoothBackend::parseManagedObjects(sd_bus_message* m) {
    BluetoothSnapshot next;
    adapter_.clear();

    if (sd_bus_message_enter_container(m, 'a', "{oa{sa{sv}}}") < 0) { return; }
    while (sd_bus_message_enter_container(m, 'e', "oa{sa{sv}}") > 0) {
        const char* path = nullptr;
        sd_bus_message_read(m, "o", &path);

        BtDevice dev;
        if (path != nullptr) { dev.path = path; }
        bool isDeviceObj = false;

        sd_bus_message_enter_container(m, 'a', "{sa{sv}}");
        while (sd_bus_message_enter_container(m, 'e', "sa{sv}") > 0) {
            const char* iface = nullptr;
            sd_bus_message_read(m, "s", &iface);

            const bool isAdapter = (iface != nullptr) && std::strcmp(iface, kAdapterIface) == 0;
            const bool isDevice = (iface != nullptr) && std::strcmp(iface, kDeviceIface) == 0;
            const bool isBattery = (iface != nullptr) && std::strcmp(iface, kBatteryIface) == 0;
            if (!isAdapter && !isDevice && !isBattery) {
                sd_bus_message_skip(m, "a{sv}");
                sd_bus_message_exit_container(m);
                continue;
            }
            if (isDevice) { isDeviceObj = true; }

            sd_bus_message_enter_container(m, 'a', "{sv}");
            while (sd_bus_message_enter_container(m, 'e', "sv") > 0) {
                const char* key = nullptr;
                sd_bus_message_read(m, "s", &key);
                auto readBool = [&](bool& out) {
                    int b = 0;
                    if (sd_bus_message_enter_container(m, 'v', "b") >= 0) {
                        sd_bus_message_read_basic(m, 'b', &b);
                        sd_bus_message_exit_container(m);
                        out = b != 0;
                    } else {
                        sd_bus_message_skip(m, "v");
                    }
                };
                auto readStr = [&](std::string& out, bool overwrite) {
                    if (sd_bus_message_enter_container(m, 'v', "s") >= 0) {
                        const char* s = nullptr;
                        sd_bus_message_read_basic(m, 's', static_cast<void*>(&s));
                        sd_bus_message_exit_container(m);
                        if (s && (overwrite || out.empty())) { out = s; }
                    } else {
                        sd_bus_message_skip(m, "v");
                    }
                };
                if (isAdapter && (key != nullptr) && std::strcmp(key, "Powered") == 0) {
                    readBool(next.powered);
                } else if (isDevice && (key != nullptr) && std::strcmp(key, "Connected") == 0) {
                    readBool(dev.connected);
                } else if (isDevice && (key != nullptr) && std::strcmp(key, "Paired") == 0) {
                    readBool(dev.paired);
                } else if (isDevice && (key != nullptr) && std::strcmp(key, "Alias") == 0) {
                    readStr(dev.name, true);
                } else if (isDevice && (key != nullptr) && std::strcmp(key, "Name") == 0) {
                    readStr(dev.name, false);
                } else if (isDevice && (key != nullptr) && std::strcmp(key, "Icon") == 0) {
                    readStr(dev.icon, true);
                } else if (isBattery && (key != nullptr) && std::strcmp(key, "Percentage") == 0) {
                    uint8_t pct = 0;
                    if (sd_bus_message_enter_container(m, 'v', "y") >= 0) {
                        sd_bus_message_read_basic(m, 'y', &pct);
                        sd_bus_message_exit_container(m);
                        dev.battery = pct;
                    } else {
                        sd_bus_message_skip(m, "v");
                    }
                } else {
                    sd_bus_message_skip(m, "v");
                }
                sd_bus_message_exit_container(m);
            }
            sd_bus_message_exit_container(m);

            if (isAdapter) {
                next.available = true;
                if (adapter_.empty() && (path != nullptr)) { adapter_ = path; }
            }
            sd_bus_message_exit_container(m);
        }
        sd_bus_message_exit_container(m);

        if (isDeviceObj) {
            if (dev.connected) {
                ++next.connectedCount;
                if (next.firstDevice.empty()) { next.firstDevice = dev.name; }
            }
            next.devices.push_back(std::move(dev));
        }
        sd_bus_message_exit_container(m);
    }
    sd_bus_message_exit_container(m);

    snap_ = std::move(next);
}

void BluetoothBackend::subscribeSignals() {
    if (subscribed_) { return; }
    subscribed_ = true;
    propsSlot_ = bus_.addMatch(
        "type='signal',sender='org.bluez',interface='org.freedesktop.DBus.Properties',"
        "member='PropertiesChanged'",
        &BluetoothBackend::onPropsChanged, this);
    ifacesSlot_ = bus_.addMatch("type='signal',sender='org.bluez',"
                                "interface='org.freedesktop.DBus.ObjectManager'",
                                &BluetoothBackend::onInterfacesChanged, this);
    // BlueZ may not be up when the bar starts; re-fetch when it (re)appears.
    ownerSlot_ = bus_.addMatch(
        "type='signal',sender='org.freedesktop.DBus',interface='org.freedesktop.DBus',"
        "member='NameOwnerChanged',arg0='org.bluez'",
        &BluetoothBackend::onNameOwnerChanged, this);
}

int BluetoothBackend::onPropsChanged(sd_bus_message* m, void* userdata, sd_bus_error* /*unused*/) {
    auto* self = static_cast<BluetoothBackend*>(userdata);
    const char* iface = nullptr;
    if (sd_bus_message_read(m, "s", &iface) < 0 || (iface == nullptr)) { return 0; }

    if (std::strcmp(iface, kAdapterIface) == 0) {
        if (!dictHasKey(m, {"Powered", "PowerState"})) { return 0; }
    } else if (std::strcmp(iface, kDeviceIface) == 0) {
        if (!dictHasKey(m, {"Connected", "Paired"})) { return 0; }
    } else if (std::strcmp(iface, kBatteryIface) == 0) {
        if (!dictHasKey(m, {"Percentage"})) { return 0; }
    } else {
        return 0;
    }

    // Re-fetch: async GetManagedObjects (serialized against any in-flight one).
    self->refetch();
    return 0;
}

int BluetoothBackend::onInterfacesChanged(sd_bus_message* /*unused*/, void* userdata,
                                          sd_bus_error* /*unused*/) {
    // Re-fetch: async GetManagedObjects (serialized against any in-flight one).
    static_cast<BluetoothBackend*>(userdata)->refetch();
    return 0;
}

int BluetoothBackend::onNameOwnerChanged(sd_bus_message* m, void* userdata,
                                         sd_bus_error* /*unused*/) {
    auto* self = static_cast<BluetoothBackend*>(userdata);
    const char* name = nullptr;
    const char* oldOwner = nullptr;
    const char* newOwner = nullptr;
    if (sd_bus_message_read(m, "sss", &name, &oldOwner, &newOwner) < 0 || (name == nullptr)) {
        return 0;
    }
    if (newOwner == nullptr || *newOwner == '\0') { return 0; }  // gone: keep last state
    self->refetch();                                             // (re)appeared: re-fetch
    return 0;
}

void BluetoothBackend::setPowered(bool on) {
    if (!bus_.available() || adapter_.empty()) { return; }

    snap_.powered = on;
    if (!on) {
        snap_.connectedCount = 0;
        snap_.firstDevice.clear();
    }
    notifyReady();

    sd_bus_message* msg = nullptr;
    if (sd_bus_message_new_method_call(bus_.get(), &msg, kBlueZ, adapter_.c_str(), kPropsIface,
                                       "Set") < 0) {
        return;
    }
    sd_bus_message_append(msg, "ss", kAdapterIface, "Powered");
    sd_bus_message_open_container(msg, 'v', "b");
    sd_bus_message_append(msg, "b", on ? 1 : 0);
    sd_bus_message_close_container(msg);
    sd_bus_call_async(bus_.get(), nullptr, msg, nullptr, nullptr, 0);
    sd_bus_message_unref(msg);
}

void BluetoothBackend::connectDevice(const std::string& path) {
    if (!bus_.available() || path.empty()) { return; }
    sd_bus_call_method_async(bus_.get(), nullptr, kBlueZ, path.c_str(), kDeviceIface, "Connect",
                             nullptr, nullptr, "");
}

void BluetoothBackend::disconnectDevice(const std::string& path) {
    if (!bus_.available() || path.empty()) { return; }
    sd_bus_call_method_async(bus_.get(), nullptr, kBlueZ, path.c_str(), kDeviceIface, "Disconnect",
                             nullptr, nullptr, "");
}

}  // namespace qypr
