// BatteryBackend.cpp - UPower monitor implementation (async startup, push via PropertiesChanged).
#include "system/BatteryBackend.hpp"

#include <systemd/sd-bus.h>

#include <cstdio>
#include <cstring>

#include "system/SystemBus.hpp"

namespace qypr {

namespace {
constexpr const char* kUPower = "org.freedesktop.UPower";
constexpr const char* kUPowerPath = "/org/freedesktop/UPower";
constexpr const char* kDisplayDevice = "/org/freedesktop/UPower/devices/DisplayDevice";
constexpr const char* kDeviceIface = "org.freedesktop.UPower.Device";
constexpr const char* kPropsIface = "org.freedesktop.DBus.Properties";

constexpr uint32_t kTypeBattery = 2;

BatterySnapshot::State mapState(uint32_t s) {
    switch (s) {
        case 1: return BatterySnapshot::Charging;
        case 2: return BatterySnapshot::Discharging;
        case 3: return BatterySnapshot::Discharging;
        case 4: return BatterySnapshot::Full;
        case 5: return BatterySnapshot::PendingCharge;
        case 6: return BatterySnapshot::Discharging;
        default: return BatterySnapshot::Unknown;
    }
}

bool readVariant(sd_bus_message* m, const char* contents, void* out) {
    if (sd_bus_message_enter_container(m, 'v', contents) < 0) {
        sd_bus_message_skip(m, "v");
        return false;
    }
    int r = sd_bus_message_read_basic(m, contents[0], out);
    sd_bus_message_exit_container(m);
    return r >= 0;
}
}  // namespace

BatteryBackend::BatteryBackend(SystemBus& bus) : bus_(bus) {}

BatteryBackend::~BatteryBackend() {
    if (signalSlot_) sd_bus_slot_unref(signalSlot_);
}

bool BatteryBackend::start() {
    if (!bus_.available()) return false;

    // Async: GetAll on DisplayDevice → callback decides next step.
    sd_bus_call_method_async(bus_.get(), nullptr, kUPower, kDisplayDevice, kPropsIface, "GetAll",
                             &BatteryBackend::onGetAllDisplay, this, "s", kDeviceIface);
    return true;
}

int BatteryBackend::onGetAllDisplay(sd_bus_message* reply, void* userdata, sd_bus_error*) {
    auto* self = static_cast<BatteryBackend*>(userdata);
    if (sd_bus_message_is_method_error(reply, nullptr)) {
        // DisplayDevice unavailable — try EnumerateDevices fallback.
        sd_bus_call_method_async(self->bus_.get(), nullptr, kUPower, kUPowerPath, kUPower,
                                 "EnumerateDevices", &BatteryBackend::onEnumerateDevices, self, "");
        return 0;
    }

    if (self->parseProps(reply) && self->snap_.present) {
        self->devicePath_ = kDisplayDevice;
        self->subscribeSignal();
        if (self->onChange_) self->onChange_();
        return 0;
    }

    // DisplayDevice says not present — try EnumerateDevices fallback.
    sd_bus_call_method_async(self->bus_.get(), nullptr, kUPower, kUPowerPath, kUPower,
                             "EnumerateDevices", &BatteryBackend::onEnumerateDevices, self, "");
    return 0;
}

int BatteryBackend::onEnumerateDevices(sd_bus_message* reply, void* userdata, sd_bus_error*) {
    auto* self = static_cast<BatteryBackend*>(userdata);
    if (sd_bus_message_is_method_error(reply, nullptr)) return 0;

    // Walk object paths, find first that looks like a battery.
    if (sd_bus_message_enter_container(reply, 'a', "o") < 0) return 0;
    const char* path = nullptr;
    std::string found;
    while (sd_bus_message_read(reply, "o", &path) > 0 && path) {
        if (std::strstr(path, "/devices/bat")) {
            found = path;
            break;
        }
    }
    sd_bus_message_exit_container(reply);

    if (found.empty()) {
        std::fprintf(stderr, "qypr: no battery via UPower; battery indicator disabled\n");
        self->snap_.present = false;
        if (self->onChange_) self->onChange_();  // hide the placeholder
        return 0;
    }

    self->devicePath_ = found;
    sd_bus_call_method_async(self->bus_.get(), nullptr, kUPower, found.c_str(), kPropsIface,
                             "GetAll", &BatteryBackend::onGetAllDevice, self, "s", kDeviceIface);
    return 0;
}

int BatteryBackend::onGetAllDevice(sd_bus_message* reply, void* userdata, sd_bus_error*) {
    auto* self = static_cast<BatteryBackend*>(userdata);
    if (sd_bus_message_is_method_error(reply, nullptr)) {
        std::fprintf(stderr, "qypr: no battery via UPower; battery indicator disabled\n");
        self->snap_.present = false;
        if (self->onChange_) self->onChange_();  // hide the placeholder
        return 0;
    }

    self->parseProps(reply);
    if (!self->snap_.present) {
        std::fprintf(stderr, "qypr: no battery via UPower; battery indicator disabled\n");
        if (self->onChange_) self->onChange_();  // hide the placeholder
        return 0;
    }

    self->subscribeSignal();
    if (self->onChange_) self->onChange_();
    return 0;
}

void BatteryBackend::subscribeSignal() {
    std::string rule = std::string("type='signal',sender='") + kUPower + "',path='" +
                       devicePath_ + "',interface='" + kPropsIface +
                       "',member='PropertiesChanged'";
    signalSlot_ = bus_.addMatch(rule.c_str(), &BatteryBackend::onPropertiesChanged, this);
}

bool BatteryBackend::parseProps(sd_bus_message* m) {
    if (sd_bus_message_enter_container(m, 'a', "{sv}") < 0) return false;

    bool any = false;
    while (sd_bus_message_enter_container(m, 'e', "sv") > 0) {
        const char* key = nullptr;
        if (sd_bus_message_read(m, "s", &key) < 0 || !key) {
            sd_bus_message_skip(m, "v");
            sd_bus_message_exit_container(m);
            continue;
        }

        if (std::strcmp(key, "Percentage") == 0) {
            double d = 0;
            if (readVariant(m, "d", &d)) {
                snap_.percentage = static_cast<int>(d + 0.5);
                any = true;
            }
        } else if (std::strcmp(key, "State") == 0) {
            uint32_t s = 0;
            if (readVariant(m, "u", &s)) {
                snap_.state = mapState(s);
                any = true;
            }
        } else if (std::strcmp(key, "IsPresent") == 0) {
            int b = 0;
            if (readVariant(m, "b", &b)) {
                snap_.present = b != 0;
                any = true;
            }
        } else if (std::strcmp(key, "TimeToEmpty") == 0) {
            int64_t t = 0;
            if (readVariant(m, "x", &t)) {
                snap_.timeToEmpty = t;
                any = true;
            }
        } else if (std::strcmp(key, "TimeToFull") == 0) {
            int64_t t = 0;
            if (readVariant(m, "x", &t)) {
                snap_.timeToFull = t;
                any = true;
            }
        } else if (std::strcmp(key, "EnergyRate") == 0) {
            double d = 0;
            if (readVariant(m, "d", &d)) {
                snap_.energyRate = d;
                any = true;
            }
        } else if (std::strcmp(key, "NativePath") == 0) {
            const char* s = nullptr;
            if (readVariant(m, "s", &s) && s) {
                snap_.nativePath = s;
                any = true;
            }
        } else {
            sd_bus_message_skip(m, "v");
        }
        sd_bus_message_exit_container(m);
    }
    sd_bus_message_exit_container(m);
    return any;
}

int BatteryBackend::onPropertiesChanged(sd_bus_message* m, void* userdata, sd_bus_error*) {
    auto* self = static_cast<BatteryBackend*>(userdata);
    const char* iface = nullptr;
    if (sd_bus_message_read(m, "s", &iface) < 0 || !iface) return 0;
    if (std::strcmp(iface, kDeviceIface) != 0) return 0;
    if (self->parseProps(m) && self->onChange_) self->onChange_();
    return 0;
}

}  // namespace qypr
