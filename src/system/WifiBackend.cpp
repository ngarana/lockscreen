// WifiBackend.cpp - NetworkManager monitor implementation (push-driven).
#include "system/WifiBackend.hpp"

#include <systemd/sd-bus.h>

#include <cstdio>
#include <cstring>

#include "system/SystemBus.hpp"

namespace qypr {

namespace {
constexpr const char* kNM = "org.freedesktop.NetworkManager";
constexpr const char* kNMPath = "/org/freedesktop/NetworkManager";
constexpr const char* kDeviceIface = "org.freedesktop.NetworkManager.Device";
constexpr const char* kWirelessIface = "org.freedesktop.NetworkManager.Device.Wireless";
constexpr const char* kApIface = "org.freedesktop.NetworkManager.AccessPoint";
constexpr const char* kPropsIface = "org.freedesktop.DBus.Properties";

constexpr uint32_t kDeviceTypeWifi = 2;

bool getBool(sd_bus* bus, const char* path, const char* iface, const char* prop, bool* out) {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    int v = 0;
    int r = sd_bus_get_property_trivial(bus, kNM, path, iface, prop, &err, 'b', &v);
    sd_bus_error_free(&err);
    if (r < 0) return false;
    *out = v != 0;
    return true;
}

bool getObjectPath(sd_bus* bus, const char* path, const char* iface, const char* prop,
                   std::string* out) {
    // Object-path properties are type "o" — sd_bus_get_property_string only
    // reads "s" and fails on the variant type mismatch.
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    int r = sd_bus_get_property(bus, kNM, path, iface, prop, &err, &reply, "o");
    sd_bus_error_free(&err);
    if (r < 0 || !reply) return false;
    const char* s = nullptr;
    r = sd_bus_message_read(reply, "o", &s);
    if (r >= 0 && s) *out = s;
    sd_bus_message_unref(reply);
    return r >= 0 && s != nullptr;
}
}  // namespace

WifiBackend::WifiBackend(SystemBus& bus) : bus_(bus) {}

WifiBackend::~WifiBackend() {
    if (slot_) sd_bus_slot_unref(slot_);
}

bool WifiBackend::start() {
    if (!bus_.available()) return false;

    device_ = findWifiDevice();
    if (device_.empty()) {
        std::fprintf(stderr, "qypr: no WiFi device via NetworkManager; wifi indicator disabled\n");
        return false;
    }

    refresh();
    if (!snap_.available) return false;

    // One broad match on NM's PropertiesChanged; the handler reacts only to
    // the root, our device, and the current AP (other APs' strength updates
    // are ignored without a refetch).
    slot_ = bus_.addMatch(
        "type='signal',sender='org.freedesktop.NetworkManager',"
        "interface='org.freedesktop.DBus.Properties',member='PropertiesChanged'",
        &WifiBackend::onPropsChanged, this);

    if (onChange_) onChange_();
    return true;
}

int WifiBackend::onPropsChanged(sd_bus_message* m, void* userdata, sd_bus_error*) {
    auto* self = static_cast<WifiBackend*>(userdata);
    const char* path = sd_bus_message_get_path(m);
    if (!path) return 0;

    if (std::strcmp(path, kNMPath) != 0 && self->device_ != path && self->activeAp_ != path) {
        return 0;
    }

    WifiSnapshot before = self->snap_;
    self->refresh();
    if (!(self->snap_ == before) && self->onChange_) self->onChange_();
    return 0;
}

void WifiBackend::refresh() {
    sd_bus* bus = bus_.get();
    if (!bus || device_.empty()) return;

    WifiSnapshot next;
    next.available = getBool(bus, kNMPath, kNM, "WirelessEnabled", &next.enabled);
    if (!next.available) {
        snap_ = next;
        return;
    }

    activeAp_.clear();
    std::string ap;
    if (next.enabled &&
        getObjectPath(bus, device_.c_str(), kWirelessIface, "ActiveAccessPoint", &ap) &&
        ap != "/") {
        activeAp_ = ap;
        next.connected = true;

        // Ssid is a byte array (not NUL-terminated, not guaranteed UTF-8).
        sd_bus_error err = SD_BUS_ERROR_NULL;
        sd_bus_message* reply = nullptr;
        if (sd_bus_get_property(bus, kNM, ap.c_str(), kApIface, "Ssid", &err, &reply, "ay") >= 0 &&
            reply) {
            const void* data = nullptr;
            size_t len = 0;
            if (sd_bus_message_read_array(reply, 'y', &data, &len) >= 0 && data && len > 0) {
                next.ssid.assign(static_cast<const char*>(data), len);
            }
            sd_bus_message_unref(reply);
        }
        sd_bus_error_free(&err);

        sd_bus_error serr = SD_BUS_ERROR_NULL;
        uint8_t strength = 0;
        if (sd_bus_get_property_trivial(bus, kNM, ap.c_str(), kApIface, "Strength", &serr, 'y',
                                        &strength) >= 0) {
            next.strength = strength;
        }
        sd_bus_error_free(&serr);
    }

    snap_ = next;
}

std::string WifiBackend::findWifiDevice() {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    int r = sd_bus_call_method(bus_.get(), kNM, kNMPath, kNM, "GetDevices", &err, &reply, "");
    sd_bus_error_free(&err);
    if (r < 0 || !reply) return {};

    std::string found;
    if (sd_bus_message_enter_container(reply, 'a', "o") >= 0) {
        const char* path = nullptr;
        while (sd_bus_message_read(reply, "o", &path) > 0 && path) {
            sd_bus_error derr = SD_BUS_ERROR_NULL;
            uint32_t type = 0;
            int tr = sd_bus_get_property_trivial(bus_.get(), kNM, path, kDeviceIface,
                                                 "DeviceType", &derr, 'u', &type);
            sd_bus_error_free(&derr);
            if (tr >= 0 && type == kDeviceTypeWifi) {
                found = path;
                break;
            }
        }
        sd_bus_message_exit_container(reply);
    }
    sd_bus_message_unref(reply);
    return found;
}

void WifiBackend::setEnabled(bool on) {
    if (!bus_.available()) return;

    // Optimistic: the radio flip is reflected immediately; NM's
    // PropertiesChanged confirms (or corrects) shortly after.
    snap_.enabled = on;
    if (!on) {
        snap_.connected = false;
        snap_.ssid.clear();
        snap_.strength = 0;
        activeAp_.clear();
    }
    if (onChange_) onChange_();

    sd_bus_message* msg = nullptr;
    if (sd_bus_message_new_method_call(bus_.get(), &msg, kNM, kNMPath, kPropsIface, "Set") < 0) {
        return;
    }
    sd_bus_message_append(msg, "ss", kNM, "WirelessEnabled");
    sd_bus_message_open_container(msg, 'v', "b");
    sd_bus_message_append(msg, "b", on ? 1 : 0);
    sd_bus_message_close_container(msg);
    sd_bus_call_async(bus_.get(), nullptr, msg, nullptr, nullptr, 0);
    sd_bus_message_unref(msg);
}

}  // namespace qypr
