// BluetoothBackend.cpp - BlueZ monitor implementation (push-driven).
#include "system/BluetoothBackend.hpp"

#include <systemd/sd-bus.h>

#include <cstdio>
#include <cstring>

#include "system/SystemBus.hpp"

namespace qypr {

namespace {
constexpr const char* kBlueZ = "org.bluez";
constexpr const char* kAdapterIface = "org.bluez.Adapter1";
constexpr const char* kDeviceIface = "org.bluez.Device1";
constexpr const char* kPropsIface = "org.freedesktop.DBus.Properties";

// Returns true if the a{sv} dict at the cursor contains any of the keys.
// Consumes the dict either way.
bool dictHasKey(sd_bus_message* m, std::initializer_list<const char*> keys) {
    bool found = false;
    if (sd_bus_message_enter_container(m, 'a', "{sv}") < 0) return false;
    while (sd_bus_message_enter_container(m, 'e', "sv") > 0) {
        const char* key = nullptr;
        if (sd_bus_message_read(m, "s", &key) >= 0 && key) {
            for (const char* k : keys) {
                if (std::strcmp(key, k) == 0) found = true;
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
    if (propsSlot_) sd_bus_slot_unref(propsSlot_);
    if (ifacesSlot_) sd_bus_slot_unref(ifacesSlot_);
}

bool BluetoothBackend::start() {
    if (!bus_.available()) return false;

    refresh();
    if (!snap_.available) {
        std::fprintf(stderr, "qypr: no Bluetooth adapter via BlueZ; indicator disabled\n");
        return false;
    }

    propsSlot_ = bus_.addMatch(
        "type='signal',sender='org.bluez',interface='org.freedesktop.DBus.Properties',"
        "member='PropertiesChanged'",
        &BluetoothBackend::onPropsChanged, this);
    ifacesSlot_ = bus_.addMatch(
        "type='signal',sender='org.bluez',"
        "interface='org.freedesktop.DBus.ObjectManager'",
        &BluetoothBackend::onInterfacesChanged, this);

    if (onChange_) onChange_();
    return true;
}

int BluetoothBackend::onPropsChanged(sd_bus_message* m, void* userdata, sd_bus_error*) {
    auto* self = static_cast<BluetoothBackend*>(userdata);
    const char* iface = nullptr;
    if (sd_bus_message_read(m, "s", &iface) < 0 || !iface) return 0;

    // Only adapter power flips and device connect/disconnect matter; RSSI
    // updates from discovery would otherwise cause a GetManagedObjects storm.
    if (std::strcmp(iface, kAdapterIface) == 0) {
        if (!dictHasKey(m, {"Powered", "PowerState"})) return 0;
    } else if (std::strcmp(iface, kDeviceIface) == 0) {
        if (!dictHasKey(m, {"Connected"})) return 0;
    } else {
        return 0;
    }

    self->refreshAndNotify();
    return 0;
}

int BluetoothBackend::onInterfacesChanged(sd_bus_message*, void* userdata, sd_bus_error*) {
    static_cast<BluetoothBackend*>(userdata)->refreshAndNotify();
    return 0;
}

void BluetoothBackend::refreshAndNotify() {
    BluetoothSnapshot before = snap_;
    refresh();
    if (!(snap_ == before) && onChange_) onChange_();
}

void BluetoothBackend::refresh() {
    sd_bus* bus = bus_.get();
    if (!bus) return;

    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    int r = sd_bus_call_method(bus, kBlueZ, "/", "org.freedesktop.DBus.ObjectManager",
                               "GetManagedObjects", &err, &reply, "");
    sd_bus_error_free(&err);
    if (r < 0 || !reply) {
        snap_ = {};
        return;
    }

    BluetoothSnapshot next;
    adapter_.clear();

    // a{oa{sa{sv}}}: object path → interface → properties
    sd_bus_message_enter_container(reply, 'a', "{oa{sa{sv}}}");
    while (sd_bus_message_enter_container(reply, 'e', "oa{sa{sv}}") > 0) {
        const char* path = nullptr;
        sd_bus_message_read(reply, "o", &path);

        sd_bus_message_enter_container(reply, 'a', "{sa{sv}}");
        while (sd_bus_message_enter_container(reply, 'e', "sa{sv}") > 0) {
            const char* iface = nullptr;
            sd_bus_message_read(reply, "s", &iface);

            bool isAdapter = iface && std::strcmp(iface, kAdapterIface) == 0;
            bool isDevice = iface && std::strcmp(iface, kDeviceIface) == 0;
            if (!isAdapter && !isDevice) {
                sd_bus_message_skip(reply, "a{sv}");
                sd_bus_message_exit_container(reply);
                continue;
            }

            bool connected = false;
            std::string name;
            sd_bus_message_enter_container(reply, 'a', "{sv}");
            while (sd_bus_message_enter_container(reply, 'e', "sv") > 0) {
                const char* key = nullptr;
                sd_bus_message_read(reply, "s", &key);
                if (isAdapter && key && std::strcmp(key, "Powered") == 0) {
                    int b = 0;
                    if (sd_bus_message_enter_container(reply, 'v', "b") >= 0) {
                        sd_bus_message_read_basic(reply, 'b', &b);
                        sd_bus_message_exit_container(reply);
                        next.powered = b != 0;
                    } else {
                        sd_bus_message_skip(reply, "v");
                    }
                } else if (isDevice && key && std::strcmp(key, "Connected") == 0) {
                    int b = 0;
                    if (sd_bus_message_enter_container(reply, 'v', "b") >= 0) {
                        sd_bus_message_read_basic(reply, 'b', &b);
                        sd_bus_message_exit_container(reply);
                        connected = b != 0;
                    } else {
                        sd_bus_message_skip(reply, "v");
                    }
                } else if (isDevice && key && std::strcmp(key, "Alias") == 0) {
                    if (sd_bus_message_enter_container(reply, 'v', "s") >= 0) {
                        const char* s = nullptr;
                        sd_bus_message_read_basic(reply, 's', &s);
                        sd_bus_message_exit_container(reply);
                        if (s) name = s;
                    } else {
                        sd_bus_message_skip(reply, "v");
                    }
                } else {
                    sd_bus_message_skip(reply, "v");
                }
                sd_bus_message_exit_container(reply);
            }
            sd_bus_message_exit_container(reply);

            if (isAdapter) {
                next.available = true;
                if (adapter_.empty() && path) adapter_ = path;
            }
            if (isDevice && connected) {
                ++next.connectedCount;
                if (next.firstDevice.empty()) next.firstDevice = name;
            }
            sd_bus_message_exit_container(reply);
        }
        sd_bus_message_exit_container(reply);
        sd_bus_message_exit_container(reply);
    }
    sd_bus_message_exit_container(reply);
    sd_bus_message_unref(reply);

    snap_ = next;
}

void BluetoothBackend::setPowered(bool on) {
    if (!bus_.available() || adapter_.empty()) return;

    // Optimistic flip; BlueZ's PropertiesChanged confirms shortly after.
    snap_.powered = on;
    if (!on) {
        snap_.connectedCount = 0;
        snap_.firstDevice.clear();
    }
    if (onChange_) onChange_();

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

}  // namespace qypr
