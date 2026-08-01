// WifiBackend.cpp - NetworkManager monitor implementation (async startup, push-driven).
#include "system/WifiBackend.hpp"

#include <systemd/sd-bus.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "system/SystemBus.hpp"

namespace qypr {

namespace {
constexpr const char* kNM = "org.freedesktop.NetworkManager";
constexpr const char* kNMPath = "/org/freedesktop/NetworkManager";
constexpr const char* kDeviceIface = "org.freedesktop.NetworkManager.Device";
constexpr const char* kWirelessIface = "org.freedesktop.NetworkManager.Device.Wireless";
constexpr const char* kApIface = "org.freedesktop.NetworkManager.AccessPoint";
constexpr const char* kPropsIface = "org.freedesktop.DBus.Properties";
constexpr const char* kSettingsPath = "/org/freedesktop/NetworkManager/Settings";
constexpr const char* kSettingsIface = "org.freedesktop.NetworkManager.Settings";
constexpr const char* kSettingsConnIface = "org.freedesktop.NetworkManager.Settings.Connection";

constexpr uint32_t kApFlagPrivacy = 0x1;

std::string readSsidProp(sd_bus* bus, const char* path, const char* iface) {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    std::string ssid;
    if (sd_bus_get_property(bus, kNM, path, iface, "Ssid", &err, &reply, "ay") >= 0 && reply) {
        const void* data = nullptr;
        size_t len = 0;
        if (sd_bus_message_read_array(reply, 'y', &data, &len) >= 0 && data && len > 0) {
            ssid.assign(static_cast<const char*>(data), len);
        }
        sd_bus_message_unref(reply);
    }
    sd_bus_error_free(&err);
    return ssid;
}

uint32_t getU(sd_bus* bus, const char* path, const char* iface, const char* prop) {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    uint32_t v = 0;
    sd_bus_get_property_trivial(bus, kNM, path, iface, prop, &err, 'u', &v);
    sd_bus_error_free(&err);
    return v;
}

uint8_t getY(sd_bus* bus, const char* path, const char* iface, const char* prop) {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    uint8_t v = 0;
    sd_bus_get_property_trivial(bus, kNM, path, iface, prop, &err, 'y', &v);
    sd_bus_error_free(&err);
    return v;
}

std::string connectionSsid(sd_bus* bus, const char* conn) {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* r = nullptr;
    if (sd_bus_call_method(bus, kNM, conn, kSettingsConnIface, "GetSettings", &err, &r, "") < 0 ||
        !r) {
        sd_bus_error_free(&err);
        return "";
    }
    sd_bus_error_free(&err);

    std::string ssid;
    if (sd_bus_message_enter_container(r, 'a', "{sa{sv}}") >= 0) {
        while (sd_bus_message_enter_container(r, 'e', "sa{sv}") > 0) {
            const char* group = nullptr;
            sd_bus_message_read(r, "s", &group);
            if (group && std::strcmp(group, "802-11-wireless") == 0 &&
                sd_bus_message_enter_container(r, 'a', "{sv}") >= 0) {
                while (sd_bus_message_enter_container(r, 'e', "sv") > 0) {
                    const char* key = nullptr;
                    sd_bus_message_read(r, "s", &key);
                    if (key && std::strcmp(key, "ssid") == 0 &&
                        sd_bus_message_enter_container(r, 'v', "ay") >= 0) {
                        const void* data = nullptr;
                        size_t len = 0;
                        if (sd_bus_message_read_array(r, 'y', &data, &len) >= 0 && data && len > 0)
                            ssid.assign(static_cast<const char*>(data), len);
                        sd_bus_message_exit_container(r);
                    } else {
                        sd_bus_message_skip(r, "v");
                    }
                    sd_bus_message_exit_container(r);
                }
                sd_bus_message_exit_container(r);
            } else {
                sd_bus_message_skip(r, "a{sv}");
            }
            sd_bus_message_exit_container(r);
        }
        sd_bus_message_exit_container(r);
    }
    sd_bus_message_unref(r);
    return ssid;
}

std::vector<std::pair<std::string, std::string>> savedConnections(sd_bus* bus) {
    std::vector<std::pair<std::string, std::string>> out;
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    if (sd_bus_call_method(bus, kNM, kSettingsPath, kSettingsIface, "ListConnections", &err, &reply,
                           "") < 0 ||
        !reply) {
        sd_bus_error_free(&err);
        return out;
    }
    sd_bus_error_free(&err);
    if (sd_bus_message_enter_container(reply, 'a', "o") >= 0) {
        const char* conn = nullptr;
        while (sd_bus_message_read(reply, "o", &conn) > 0 && conn) {
            std::string ssid = connectionSsid(bus, conn);
            if (!ssid.empty()) out.emplace_back(ssid, conn);
        }
        sd_bus_message_exit_container(reply);
    }
    sd_bus_message_unref(reply);
    return out;
}

bool getObjectPath(sd_bus* bus, const char* path, const char* iface, const char* prop,
                   std::string* out) {
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

// Helpers to extract values from a Get property reply (v{type} container).
bool extractBool(sd_bus_message* m, bool* out) {
    if (sd_bus_message_enter_container(m, 'v', "b") < 0) {
        sd_bus_message_skip(m, "v");
        return false;
    }
    int v = 0;
    int r = sd_bus_message_read_basic(m, 'b', &v);
    sd_bus_message_exit_container(m);
    if (r < 0) return false;
    *out = v != 0;
    return true;
}

bool extractObjPath(sd_bus_message* m, std::string* out) {
    if (sd_bus_message_enter_container(m, 'v', "o") < 0) {
        sd_bus_message_skip(m, "v");
        return false;
    }
    const char* s = nullptr;
    int r = sd_bus_message_read_basic(m, 'o', &s);
    sd_bus_message_exit_container(m);
    if (r < 0 || !s) return false;
    *out = s;
    return true;
}

bool extractByteArray(sd_bus_message* m, std::string* out) {
    if (sd_bus_message_enter_container(m, 'v', "ay") < 0) {
        sd_bus_message_skip(m, "v");
        return false;
    }
    const void* data = nullptr;
    size_t len = 0;
    int r = sd_bus_message_read_array(m, 'y', &data, &len);
    sd_bus_message_exit_container(m);
    if (r < 0 || !data || len == 0) return false;
    out->assign(static_cast<const char*>(data), len);
    return true;
}

bool extractByte(sd_bus_message* m, uint8_t* out) {
    if (sd_bus_message_enter_container(m, 'v', "y") < 0) {
        sd_bus_message_skip(m, "v");
        return false;
    }
    int r = sd_bus_message_read_basic(m, 'y', out);
    sd_bus_message_exit_container(m);
    return r >= 0;
}

// Issue an async Properties.Get for a single property.
void asyncGetProp(sd_bus* bus, const char* dest, const char* path, const char* iface,
                  const char* prop, sd_bus_message_handler_t cb, void* userdata) {
    sd_bus_call_method_async(bus, nullptr, dest, path, kPropsIface, "Get", cb, userdata, "ss", iface,
                             prop);
}
}  // namespace

WifiBackend::WifiBackend(SystemBus& bus) : bus_(bus) {}

WifiBackend::~WifiBackend() {
    if (slot_) sd_bus_slot_unref(slot_);
}

bool WifiBackend::start() {
    if (!bus_.available()) return false;

    // Async GetDevices → callback finds WiFi device → refreshAsync().
    sd_bus_call_method_async(bus_.get(), nullptr, kNM, kNMPath, kNM, "GetDevices",
                             &WifiBackend::onGetDevices, this, "");
    return true;
}

int WifiBackend::onGetDevices(sd_bus_message* reply, void* userdata, sd_bus_error*) {
    auto* self = static_cast<WifiBackend*>(userdata);
    if (sd_bus_message_is_method_error(reply, nullptr)) return 0;

    if (sd_bus_message_enter_container(reply, 'a', "o") < 0) return 0;
    const char* path = nullptr;
    std::string found;
    while (sd_bus_message_read(reply, "o", &path) > 0 && path) {
        if (std::strstr(path, "/wireless") || std::strstr(path, "/wlan")) {
            found = path;
            break;
        }
    }
    sd_bus_message_exit_container(reply);

    if (found.empty()) {
        std::fprintf(stderr, "qypr: no WiFi device via NetworkManager; wifi indicator disabled\n");
        self->snap_ = {};  // available=false → hide the placeholder
        if (self->onChange_) self->onChange_();
        return 0;
    }

    self->device_ = found;
    self->refreshAsync();
    return 0;
}

void WifiBackend::refreshAsync() {
    sd_bus* bus = bus_.get();
    if (!bus || device_.empty()) return;

    refreshStep_ = RefreshStep::WirelessEnabled;
    pendingSnap_ = {};
    asyncGetProp(bus, kNM, kNMPath, kNM, "WirelessEnabled", &WifiBackend::onRefreshStep, this);
}

int WifiBackend::onRefreshStep(sd_bus_message* reply, void* userdata, sd_bus_error*) {
    auto* self = static_cast<WifiBackend*>(userdata);
    if (sd_bus_message_is_method_error(reply, nullptr)) return 0;
    sd_bus* bus = self->bus_.get();
    if (!bus) return 0;

    switch (self->refreshStep_) {
        case RefreshStep::WirelessEnabled: {
            bool enabled = false;
            if (!extractBool(reply, &enabled)) return 0;
            self->pendingSnap_.enabled = enabled;
            self->pendingSnap_.available = true;
            if (!enabled) {
                self->activeAp_.clear();
                self->snap_ = self->pendingSnap_;
                self->subscribeSignal();
                if (self->onChange_) self->onChange_();
                return 0;
            }
            // Publish the enabled/available state now so the module appears
            // within a single round trip; the AP/SSID/strength steps below refine
            // it in place. subscribeSignal() guards against a double subscribe.
            self->snap_ = self->pendingSnap_;
            self->subscribeSignal();
            if (self->onChange_) self->onChange_();

            self->refreshStep_ = RefreshStep::ActiveAccessPoint;
            asyncGetProp(bus, kNM, self->device_.c_str(), kWirelessIface, "ActiveAccessPoint",
                         &WifiBackend::onRefreshStep, self);
            return 0;
        }
        case RefreshStep::ActiveAccessPoint: {
            std::string ap;
            if (!extractObjPath(reply, &ap) || ap.empty() || ap == "/") {
                self->activeAp_.clear();
                self->snap_ = self->pendingSnap_;
                self->subscribeSignal();
                if (self->onChange_) self->onChange_();
                return 0;
            }
            self->activeAp_ = ap;
            self->pendingSnap_.connected = true;
            self->refreshStep_ = RefreshStep::Ssid;
            asyncGetProp(bus, kNM, ap.c_str(), kApIface, "Ssid", &WifiBackend::onRefreshStep,
                         self);
            return 0;
        }
        case RefreshStep::Ssid: {
            std::string ssid;
            extractByteArray(reply, &ssid);
            self->pendingSnap_.ssid = std::move(ssid);
            self->refreshStep_ = RefreshStep::Strength;
            asyncGetProp(bus, kNM, self->activeAp_.c_str(), kApIface, "Strength",
                         &WifiBackend::onRefreshStep, self);
            return 0;
        }
        case RefreshStep::Strength: {
            uint8_t strength = 0;
            extractByte(reply, &strength);
            self->pendingSnap_.strength = strength;
            self->snap_ = self->pendingSnap_;
            self->subscribeSignal();
            if (self->onChange_) self->onChange_();
            return 0;
        }
    }
    return 0;
}

void WifiBackend::subscribeSignal() {
    if (slot_) return;  // already subscribed
    slot_ = bus_.addMatch(
        "type='signal',sender='org.freedesktop.NetworkManager',"
        "interface='org.freedesktop.DBus.Properties',member='PropertiesChanged'",
        &WifiBackend::onPropsChanged, this);
}

int WifiBackend::onPropsChanged(sd_bus_message* m, void* userdata, sd_bus_error*) {
    auto* self = static_cast<WifiBackend*>(userdata);
    const char* path = sd_bus_message_get_path(m);
    if (!path) return 0;

    if (std::strcmp(path, kNMPath) != 0 && self->device_ != path && self->activeAp_ != path) {
        return 0;
    }

    // On any relevant property change, do a full async refresh.
    self->refreshAsync();
    return 0;
}

void WifiBackend::setEnabled(bool on) {
    if (!bus_.available()) return;

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

std::vector<WifiAp> WifiBackend::scanNetworks() const {
    std::vector<WifiAp> out;
    sd_bus* bus = bus_.get();
    if (!bus || device_.empty()) return out;

    std::unordered_set<std::string> saved;
    for (auto& p : savedConnections(bus)) saved.insert(p.first);

    std::string active;
    getObjectPath(bus, device_.c_str(), kWirelessIface, "ActiveAccessPoint", &active);

    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    if (sd_bus_get_property(bus, kNM, device_.c_str(), kWirelessIface, "AccessPoints", &err, &reply,
                            "ao") < 0 ||
        !reply) {
        sd_bus_error_free(&err);
        return out;
    }
    sd_bus_error_free(&err);

    std::vector<std::string> paths;
    if (sd_bus_message_enter_container(reply, 'a', "o") >= 0) {
        const char* p = nullptr;
        while (sd_bus_message_read(reply, "o", &p) > 0 && p) paths.emplace_back(p);
        sd_bus_message_exit_container(reply);
    }
    sd_bus_message_unref(reply);

    std::unordered_map<std::string, size_t> bySsid;
    for (const auto& ap : paths) {
        std::string ssid = readSsidProp(bus, ap.c_str(), kApIface);
        if (ssid.empty()) continue;
        const int strength = getY(bus, ap.c_str(), kApIface, "Strength");
        const bool secured = (getU(bus, ap.c_str(), kApIface, "Flags") & kApFlagPrivacy) ||
                             getU(bus, ap.c_str(), kApIface, "WpaFlags") ||
                             getU(bus, ap.c_str(), kApIface, "RsnFlags");
        const bool isActive = ap == active;

        auto it = bySsid.find(ssid);
        if (it != bySsid.end()) {
            WifiAp& e = out[it->second];
            e.strength = std::max(e.strength, strength);
            e.active = e.active || isActive;
            continue;
        }
        bySsid[ssid] = out.size();
        out.push_back({ssid, strength, secured, isActive, saved.count(ssid) > 0});
    }

    std::stable_sort(out.begin(), out.end(), [](const WifiAp& a, const WifiAp& b) {
        if (a.active != b.active) return a.active > b.active;
        return a.strength > b.strength;
    });
    return out;
}

void WifiBackend::requestScan() {
    sd_bus* bus = bus_.get();
    if (!bus || device_.empty()) return;
    sd_bus_message* msg = nullptr;
    if (sd_bus_message_new_method_call(bus, &msg, kNM, device_.c_str(), kWirelessIface,
                                       "RequestScan") < 0)
        return;
    sd_bus_message_open_container(msg, 'a', "{sv}");
    sd_bus_message_close_container(msg);
    sd_bus_call_async(bus, nullptr, msg, nullptr, nullptr, 0);
    sd_bus_message_unref(msg);
}

std::string WifiBackend::findSavedConnection(const std::string& ssid) const {
    sd_bus* bus = bus_.get();
    if (!bus) return {};
    for (auto& p : savedConnections(bus))
        if (p.first == ssid) return p.second;
    return {};
}

void WifiBackend::connectSsid(const std::string& ssid) {
    sd_bus* bus = bus_.get();
    if (!bus || ssid.empty() || device_.empty()) return;
    const std::string conn = findSavedConnection(ssid);
    if (conn.empty()) return;
    sd_bus_call_method_async(bus, nullptr, kNM, kNMPath, kNM, "ActivateConnection", nullptr,
                             nullptr, "ooo", conn.c_str(), device_.c_str(), "/");
}

void WifiBackend::disconnect() {
    sd_bus* bus = bus_.get();
    if (!bus || device_.empty()) return;
    sd_bus_call_method_async(bus, nullptr, kNM, device_.c_str(), kDeviceIface, "Disconnect",
                             nullptr, nullptr, "");
}

}  // namespace qypr
