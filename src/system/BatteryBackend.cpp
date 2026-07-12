// BatteryBackend.cpp - UPower D-Bus monitor implementation.
#include "system/BatteryBackend.hpp"

#include <cstring>
#include <systemd/sd-bus.h>

namespace qypr {

namespace {
constexpr const char* kUPowerBus = "org.freedesktop.UPower";
constexpr const char* kUPowerPath = "/org/freedesktop/UPower";
constexpr const char* kDisplayDevicePath = "/org/freedesktop/UPower/devices/DisplayDevice";
constexpr const char* kPropsInterface = "org.freedesktop.DBus.Properties";
constexpr const char* kBatteryInterface = "org.freedesktop.UPower.Device";

int stateToEnum(const char* state) {
    if (!state) return BatterySnapshot::Unknown;
    if (std::strcmp(state, "charging") == 0) return BatterySnapshot::Charging;
    if (std::strcmp(state, "discharging") == 0) return BatterySnapshot::Discharging;
    if (std::strcmp(state, "full") == 0) return BatterySnapshot::Full;
    if (std::strcmp(state, "pending-charge") == 0) return BatterySnapshot::PendingCharge;
    return BatterySnapshot::Unknown;
}
}  // namespace

BatteryBackend::BatteryBackend() {
    sd_bus_open_user(&bus_);
    refresh();
}

BatteryBackend::~BatteryBackend() {
    if (slot_) sd_bus_slot_unref(slot_);
    if (bus_) sd_bus_unref(bus_);
}

void BatteryBackend::refresh() {
    if (!bus_) return;
    pollFromDevice();
}

void BatteryBackend::pollFromDevice() {
    sd_bus_message* reply = nullptr;
    sd_bus_error error{};

    int r = sd_bus_call_method(bus_, kUPowerBus, kDisplayDevicePath,
                               kPropsInterface, "GetAll", &error, &reply,
                               "s", kBatteryInterface);
    if (r < 0) {
        // Fallback: try enumerating devices
        sd_bus_message* enumReply = nullptr;
        r = sd_bus_call_method(bus_, kUPowerBus, kUPowerPath,
                               "org.freedesktop.UPower", "EnumerateDevices",
                               &error, &enumReply, "");
        if (r >= 0 && enumReply) {
            sd_bus_message_enter_container(enumReply, 'a', "o");
            const char* devPath = nullptr;
            while (sd_bus_message_read(enumReply, "o", &devPath) > 0) {
                // Try each device until we find a battery
                sd_bus_message* devReply = nullptr;
                sd_bus_error devErr{};
                if (sd_bus_call_method(bus_, kUPowerBus, devPath,
                                       kPropsInterface, "GetAll", &devErr, &devReply,
                                       "s", kBatteryInterface) >= 0 && devReply) {
                    parseProperties(nullptr);
                    sd_bus_message_unref(devReply);
                    break;
                }
            }
            sd_bus_message_exit_container(enumReply);
            sd_bus_message_unref(enumReply);
        }
        return;
    }

    if (!reply) return;

    // Parse the a{sv} reply
    sd_bus_message_enter_container(reply, 'a', "{sv}");
    const char* key = nullptr;
    while (sd_bus_message_enter_container(reply, 'e', "sv") > 0) {
        sd_bus_message_read(reply, "s", &key);
        if (!key) { sd_bus_message_exit_container(reply); continue; }

        if (std::strcmp(key, "Percentage") == 0) {
            double pct = 0;
            sd_bus_message_read(reply, "d", &pct);
            snap_.percentage = static_cast<int>(pct);
        } else if (std::strcmp(key, "State") == 0) {
            const char* state = nullptr;
            sd_bus_message_read(reply, "s", &state);
            snap_.state = static_cast<BatterySnapshot::State>(stateToEnum(state));
        } else if (std::strcmp(key, "IsPresent") == 0) {
            int present = 0;
            sd_bus_message_read(reply, "b", &present);
            snap_.present = present != 0;
        } else if (std::strcmp(key, "TimeToEmpty") == 0) {
            int64_t t = 0;
            sd_bus_message_read(reply, "x", &t);
            snap_.timeToEmpty = t;
        } else if (std::strcmp(key, "TimeToFull") == 0) {
            int64_t t = 0;
            sd_bus_message_read(reply, "x", &t);
            snap_.timeToFull = t;
        } else if (std::strcmp(key, "EnergyRate") == 0) {
            double rate = 0;
            sd_bus_message_read(reply, "d", &rate);
            snap_.energyRate = rate;
        } else if (std::strcmp(key, "NativePath") == 0) {
            const char* path = nullptr;
            sd_bus_message_read(reply, "s", &path);
            snap_.nativePath = path ? path : "";
        } else {
            sd_bus_message_skip(reply, "v");
        }
        sd_bus_message_exit_container(reply);
    }
    sd_bus_message_exit_container(reply);
    sd_bus_message_unref(reply);
}

void BatteryBackend::parseProperties(const char*) {
    // Placeholder for future property parsing
}

int BatteryBackend::dbusFd() const {
    return bus_ ? sd_bus_get_fd(bus_) : -1;
}

void BatteryBackend::processEvents() {
    if (!bus_) return;
    sd_bus_message* msg = nullptr;
    while (sd_bus_process(bus_, &msg) > 0) {
        // Process signals
    }
}

}  // namespace qypr
