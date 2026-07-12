// BatteryBackend.cpp - UPower monitor implementation (push via PropertiesChanged).
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

// org.freedesktop.UPower.Device.Type: 2 = battery.
constexpr uint32_t kTypeBattery = 2;

// org.freedesktop.UPower.Device.State (uint32).
BatterySnapshot::State mapState(uint32_t s) {
    switch (s) {
        case 1: return BatterySnapshot::Charging;
        case 2: return BatterySnapshot::Discharging;
        case 3: return BatterySnapshot::Discharging;  // empty
        case 4: return BatterySnapshot::Full;
        case 5: return BatterySnapshot::PendingCharge;
        case 6: return BatterySnapshot::Discharging;  // pending discharge
        default: return BatterySnapshot::Unknown;
    }
}

// Read one variant holding the expected basic type; skips the variant whole
// when the daemon sends something else.
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
    if (slot_) sd_bus_slot_unref(slot_);
}

bool BatteryBackend::start() {
    if (!bus_.available()) return false;

    devicePath_ = kDisplayDevice;
    bool ok = fetchAll(devicePath_.c_str());
    if (!ok || !snap_.present) {
        // Desktop machines expose a DisplayDevice with IsPresent=false.
        std::string alt = findBatteryDevice();
        if (!alt.empty()) {
            devicePath_ = alt;
            ok = fetchAll(devicePath_.c_str());
        }
    }
    if (!ok || !snap_.present) {
        std::fprintf(stderr, "qypr: no battery via UPower; battery indicator disabled\n");
        return false;
    }

    std::string rule = std::string("type='signal',sender='") + kUPower + "',path='" +
                       devicePath_ + "',interface='" + kPropsIface +
                       "',member='PropertiesChanged'";
    slot_ = bus_.addMatch(rule.c_str(), &BatteryBackend::onPropertiesChanged, this);

    if (onChange_) onChange_();
    return true;
}

int BatteryBackend::onPropertiesChanged(sd_bus_message* m, void* userdata, sd_bus_error*) {
    auto* self = static_cast<BatteryBackend*>(userdata);
    const char* iface = nullptr;
    if (sd_bus_message_read(m, "s", &iface) < 0 || !iface) return 0;
    if (std::strcmp(iface, kDeviceIface) != 0) return 0;
    if (self->parseProps(m) && self->onChange_) self->onChange_();
    return 0;
}

bool BatteryBackend::fetchAll(const char* devicePath) {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    int r = sd_bus_call_method(bus_.get(), kUPower, devicePath, kPropsIface, "GetAll",
                               &err, &reply, "s", kDeviceIface);
    sd_bus_error_free(&err);
    if (r < 0 || !reply) return false;

    bool ok = parseProps(reply);
    sd_bus_message_unref(reply);
    return ok;
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

std::string BatteryBackend::findBatteryDevice() {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    int r = sd_bus_call_method(bus_.get(), kUPower, kUPowerPath, kUPower,
                               "EnumerateDevices", &err, &reply, "");
    sd_bus_error_free(&err);
    if (r < 0 || !reply) return {};

    std::string found;
    if (sd_bus_message_enter_container(reply, 'a', "o") >= 0) {
        const char* path = nullptr;
        while (sd_bus_message_read(reply, "o", &path) > 0 && path) {
            sd_bus_error derr = SD_BUS_ERROR_NULL;
            uint32_t type = 0;
            int tr = sd_bus_get_property_trivial(bus_.get(), kUPower, path, kDeviceIface,
                                                 "Type", &derr, 'u', &type);
            sd_bus_error_free(&derr);
            if (tr >= 0 && type == kTypeBattery) {
                found = path;
                break;
            }
        }
        sd_bus_message_exit_container(reply);
    }
    sd_bus_message_unref(reply);
    return found;
}

}  // namespace qypr
