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
constexpr const char* kObjectManagerIface = "org.freedesktop.DBus.ObjectManager";
constexpr const char* kSettingsPath = "/org/freedesktop/NetworkManager/Settings";
constexpr const char* kSettingsIface = "org.freedesktop.NetworkManager.Settings";
constexpr const char* kSettingsConnIface = "org.freedesktop.NetworkManager.Settings.Connection";

constexpr uint32_t kApFlagPrivacy = 0x1;
constexpr uint32_t kWifiDeviceType = 2;          // NM_DEVICE_TYPE_WIFI
constexpr uint32_t kDeviceStateActivated = 100;  // NM_DEVICE_STATE_ACTIVATED

std::string readSsidProp(sd_bus* bus, const char* path, const char* iface) {
    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    std::string ssid;
    if (sd_bus_get_property(bus, kNM, path, iface, "Ssid", &err, &reply, "ay") >= 0 &&
        (reply != nullptr)) {
        const void* data = nullptr;
        size_t len = 0;
        if (sd_bus_message_read_array(reply, 'y', &data, &len) >= 0 && (data != nullptr) &&
            len > 0) {
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
        (r == nullptr)) {
        sd_bus_error_free(&err);
        return "";
    }
    sd_bus_error_free(&err);

    std::string ssid;
    if (sd_bus_message_enter_container(r, 'a', "{sa{sv}}") >= 0) {
        while (sd_bus_message_enter_container(r, 'e', "sa{sv}") > 0) {
            const char* group = nullptr;
            sd_bus_message_read(r, "s", &group);
            if ((group != nullptr) && std::strcmp(group, "802-11-wireless") == 0 &&
                sd_bus_message_enter_container(r, 'a', "{sv}") >= 0) {
                while (sd_bus_message_enter_container(r, 'e', "sv") > 0) {
                    const char* key = nullptr;
                    sd_bus_message_read(r, "s", &key);
                    if ((key != nullptr) && std::strcmp(key, "ssid") == 0 &&
                        sd_bus_message_enter_container(r, 'v', "ay") >= 0) {
                        const void* data = nullptr;
                        size_t len = 0;
                        if (sd_bus_message_read_array(r, 'y', &data, &len) >= 0 &&
                            (data != nullptr) && len > 0) {
                            ssid.assign(static_cast<const char*>(data), len);
                        }
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
        (reply == nullptr)) {
        sd_bus_error_free(&err);
        return out;
    }
    sd_bus_error_free(&err);
    if (sd_bus_message_enter_container(reply, 'a', "o") >= 0) {
        const char* conn = nullptr;
        while (sd_bus_message_read(reply, "o", &conn) > 0 && (conn != nullptr)) {
            std::string const ssid = connectionSsid(bus, conn);
            if (!ssid.empty()) { out.emplace_back(ssid, conn); }
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
    if (r < 0 || (reply == nullptr)) { return false; }
    const char* s = nullptr;
    r = sd_bus_message_read(reply, "o", &s);
    if (r >= 0 && (s != nullptr)) { *out = s; }
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
    int const r = sd_bus_message_read_basic(m, 'b', &v);
    sd_bus_message_exit_container(m);
    if (r < 0) { return false; }
    *out = v != 0;
    return true;
}

bool extractObjPath(sd_bus_message* m, std::string* out) {
    if (sd_bus_message_enter_container(m, 'v', "o") < 0) {
        sd_bus_message_skip(m, "v");
        return false;
    }
    const char* s = nullptr;
    int const r = sd_bus_message_read_basic(m, 'o', static_cast<void*>(&s));
    sd_bus_message_exit_container(m);
    if (r < 0 || (s == nullptr)) { return false; }
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
    int const r = sd_bus_message_read_array(m, 'y', &data, &len);
    sd_bus_message_exit_container(m);
    if (r < 0 || (data == nullptr) || len == 0) { return false; }
    out->assign(static_cast<const char*>(data), len);
    return true;
}

bool extractByte(sd_bus_message* m, uint8_t* out) {
    if (sd_bus_message_enter_container(m, 'v', "y") < 0) {
        sd_bus_message_skip(m, "v");
        return false;
    }
    int const r = sd_bus_message_read_basic(m, 'y', out);
    sd_bus_message_exit_container(m);
    return r >= 0;
}

bool extractU32(sd_bus_message* m, uint32_t* out) {
    if (sd_bus_message_enter_container(m, 'v', "u") < 0) {
        sd_bus_message_skip(m, "v");
        return false;
    }
    int const r = sd_bus_message_read_basic(m, 'u', out);
    sd_bus_message_exit_container(m);
    return r >= 0;
}
}  // namespace

WifiBackend::WifiBackend(SystemBus& bus) : bus_(bus) {}

WifiBackend::~WifiBackend() {
    sd_bus_slot_unref(propsSlot_);
    sd_bus_slot_unref(addedSlot_);
    sd_bus_slot_unref(removedSlot_);
    sd_bus_slot_unref(ownerSlot_);
}

bool WifiBackend::start() {
    if (!bus_.available()) { return false; }

    // Subscribe before the first fetch: a change landing between the fetch and
    // the subscription would be lost (standard subscribe-then-fetch order).
    subscribeSignals();
    refreshAsync();
    return true;
}

void WifiBackend::subscribeSignals() {
    if (subscribed_) { return; }
    subscribed_ = true;
    // Scoped to the NM object tree; the handler still filters by path.
    propsSlot_ =
        bus_.addMatch("type='signal',sender='org.freedesktop.NetworkManager',"
                      "path_namespace='/org/freedesktop/NetworkManager',"
                      "interface='org.freedesktop.DBus.Properties',member='PropertiesChanged'",
                      &WifiBackend::onPropsChanged, this);
    // Adapters appear/disappear (USB dongles) outside PropertiesChanged.
    addedSlot_ = bus_.addMatch("type='signal',sender='org.freedesktop.NetworkManager',"
                               "interface='org.freedesktop.NetworkManager',member='DeviceAdded'",
                               &WifiBackend::onDeviceAdded, this);
    removedSlot_ =
        bus_.addMatch("type='signal',sender='org.freedesktop.NetworkManager',"
                      "interface='org.freedesktop.NetworkManager',member='DeviceRemoved'",
                      &WifiBackend::onDeviceRemoved, this);
    // NM may not be up when the bar starts; re-enumerate when it (re)appears.
    ownerSlot_ = bus_.addMatch(
        "type='signal',sender='org.freedesktop.DBus',interface='org.freedesktop.DBus',"
        "member='NameOwnerChanged',arg0='org.freedesktop.NetworkManager'",
        &WifiBackend::onNameOwnerChanged, this);
}

// One fetch chain in flight at a time: a signal during the chain only marks
// pendingRefresh_, and the next chain runs when the current one lands. This is
// what keeps replies from different chains from arriving out of order and
// regressing the snapshot to stale state.
void WifiBackend::refreshAsync() {
    if (!bus_.available()) { return; }
    if (fetchInFlight_) {
        pendingRefresh_ = true;  // a change arrived mid-fetch; re-run after
        return;
    }
    fetchInFlight_ = true;
    devices_.clear();
    devIndex_ = 0;
    devState_ = 0;
    apStrength_ = 0;
    apSsid_.clear();
    fetchStep_ = 0;  // waiting for GetDevices
    // NetworkManager 1.58 removed org.freedesktop.DBus.ObjectManager, so
    // enumerate with GetDevices and read the properties we need per object.
    sd_bus_call_method_async(bus_.get(), nullptr, kNM, kNMPath, kNM, "GetDevices",
                             &WifiBackend::onFetchStep, this, "");
}

void WifiBackend::endFetch() {
    fetchInFlight_ = false;
    if (pendingRefresh_) {
        pendingRefresh_ = false;
        refreshAsync();
    }
}

int WifiBackend::onFetchStep(sd_bus_message* reply, void* userdata, sd_bus_error* /*unused*/) {
    auto* self = static_cast<WifiBackend*>(userdata);
    if (sd_bus_message_is_method_error(reply, nullptr) != 0) {
        // Transient failure (NM restarting at bar start): publish a definitive
        // "absent" instead of leaving the placeholder pending forever.
        self->publishWifiFailure("NetworkManager unavailable; wifi indicator disabled");
        self->endFetch();
        return 0;
    }
    // Dispatch to the step the chain is currently waiting on; a reply can only
    // arrive for the request the chain last issued.
    switch (self->fetchStep_) {
        case 0:
            self->stepDevices(reply);
            break;
        case 1:
            self->stepDeviceProps(reply);
            break;
        case 2:
            self->stepWirelessProps(reply);
            break;
        case 3:
            self->stepApProps(reply);
            break;
        default:
            self->publishWifiFailure("internal: unexpected wifi fetch step");
            self->endFetch();
    }
    return 0;
}

// GetDevices → ao: collect the device paths; then GetAll on the NM root to
// learn WirelessEnabled, then walk the devices to find the WiFi one.
void WifiBackend::stepDevices(sd_bus_message* reply) {
    devices_.clear();
    if (sd_bus_message_enter_container(reply, 'a', "o") >= 0) {
        const char* path = nullptr;
        while (sd_bus_message_read(reply, "o", &path) > 0 && (path != nullptr)) {
            devices_.emplace_back(path);
        }
        sd_bus_message_exit_container(reply);
    }

    if (devices_.empty()) {
        finishNoWifi();
        return;
    }
    sd_bus_call_method_async(bus_.get(), nullptr, kNM, kNMPath, kPropsIface, "GetAll",
                             &WifiBackend::onFetchStep, this, "s", kNM);
    fetchStep_ = 1;  // waiting for root GetAll
}

// GetAll on a device → DeviceType/State; the root object's GetAll carries
// WirelessEnabled (GetAll returns every property, so the reply shape is
// identical regardless of object).
void WifiBackend::stepDeviceProps(sd_bus_message* reply) {
    uint32_t devType = 0;
    uint32_t state = 0;
    bool sawWirelessEnabled = false;
    if (sd_bus_message_enter_container(reply, 'a', "{sv}") >= 0) {
        while (sd_bus_message_enter_container(reply, 'e', "sv") > 0) {
            const char* key = nullptr;
            sd_bus_message_read(reply, "s", &key);
            if (key == nullptr) {
                sd_bus_message_skip(reply, "v");
                sd_bus_message_exit_container(reply);
                continue;
            }
            if (std::strcmp(key, "WirelessEnabled") == 0) {
                extractBool(reply, &wirelessEnabled_);
                sawWirelessEnabled = true;
            } else if (std::strcmp(key, "DeviceType") == 0) {
                extractU32(reply, &devType);
            } else if (std::strcmp(key, "State") == 0) {
                extractU32(reply, &state);
            } else {
                sd_bus_message_skip(reply, "v");
            }
            sd_bus_message_exit_container(reply);
        }
        sd_bus_message_exit_container(reply);
    }

    if (sawWirelessEnabled) {
        // Root GetAll: only WirelessEnabled is interesting; start walking the
        // devices.
        fetchDeviceAt(0);
        return;
    }
    if (devType != kWifiDeviceType) {
        fetchDeviceAt(devIndex_ + 1);
        return;
    }
    // Found the WiFi device: remember it and its state, then read the active
    // access point.
    device_ = devices_.at(devIndex_);
    devState_ = state;
    fetchStep_ = 2;  // waiting for Wireless GetAll
    sd_bus_call_method_async(bus_.get(), nullptr, kNM, device_.c_str(), kPropsIface, "GetAll",
                             &WifiBackend::onFetchStep, this, "s", kWirelessIface);
}

// GetAll on the WiFi device (Wireless interface) → ActiveAccessPoint.
void WifiBackend::stepWirelessProps(sd_bus_message* reply) {
    std::string ap;
    if (sd_bus_message_enter_container(reply, 'a', "{sv}") >= 0) {
        while (sd_bus_message_enter_container(reply, 'e', "sv") > 0) {
            const char* key = nullptr;
            sd_bus_message_read(reply, "s", &key);
            if (key == nullptr) {
                sd_bus_message_skip(reply, "v");
                sd_bus_message_exit_container(reply);
                continue;
            }
            if (std::strcmp(key, "ActiveAccessPoint") == 0) {
                extractObjPath(reply, &ap);
            } else {
                sd_bus_message_skip(reply, "v");
            }
            sd_bus_message_exit_container(reply);
        }
        sd_bus_message_exit_container(reply);
    }

    if (ap.empty() || ap == "/") {
        // Not connected: publish the enabled/available state immediately.
        activeAp_.clear();
        apSsid_.clear();
        apStrength_ = 0;
        publish();
        endFetch();
        return;
    }
    activeAp_ = ap;
    fetchStep_ = 3;  // waiting for AP GetAll
    sd_bus_call_method_async(bus_.get(), nullptr, kNM, ap.c_str(), kPropsIface, "GetAll",
                             &WifiBackend::onFetchStep, this, "s", kApIface);
}

// GetAll on the active AP → Ssid/Strength; terminal step of the chain.
void WifiBackend::stepApProps(sd_bus_message* reply) {
    if (sd_bus_message_enter_container(reply, 'a', "{sv}") >= 0) {
        while (sd_bus_message_enter_container(reply, 'e', "sv") > 0) {
            const char* key = nullptr;
            sd_bus_message_read(reply, "s", &key);
            if (key == nullptr) {
                sd_bus_message_skip(reply, "v");
                sd_bus_message_exit_container(reply);
                continue;
            }
            if (std::strcmp(key, "Ssid") == 0) {
                extractByteArray(reply, &apSsid_);
            } else if (std::strcmp(key, "Strength") == 0) {
                extractByte(reply, &apStrength_);
            } else {
                sd_bus_message_skip(reply, "v");
            }
            sd_bus_message_exit_container(reply);
        }
        sd_bus_message_exit_container(reply);
    }
    publish();
    endFetch();
}

void WifiBackend::fetchDeviceAt(size_t index) {
    devIndex_ = index;
    if (devIndex_ >= devices_.size()) {
        finishNoWifi();
        return;
    }
    fetchStep_ = 1;  // waiting for device GetAll
    sd_bus_call_method_async(bus_.get(), nullptr, kNM, devices_.at(devIndex_).c_str(), kPropsIface,
                             "GetAll", &WifiBackend::onFetchStep, this, "s", kDeviceIface);
}

void WifiBackend::finishNoWifi() {
    if (!ready_) {
        std::fprintf(stderr, "qypr: no WiFi device via NetworkManager; wifi indicator disabled\n");
    }
    device_.clear();
    activeAp_.clear();
    apSsid_.clear();
    apStrength_ = 0;
    publish();
    endFetch();
}

void WifiBackend::publishWifiFailure(const char* what) {
    std::fprintf(stderr, "qypr: %s\n", what);
    device_.clear();
    activeAp_.clear();
    apSsid_.clear();
    apStrength_ = 0;
    publish();
}

void WifiBackend::publish() {
    WifiSnapshot next;
    next.available = !device_.empty();
    next.enabled = wirelessEnabled_ && next.available;
    if (next.available) {
        // Connection truth is the device state (100 = activated), not the mere
        // presence of an access point.
        next.connected = std::cmp_equal(devState_, kDeviceStateActivated);
        if (next.connected && !apSsid_.empty()) {
            next.ssid = apSsid_;
            next.strength = apStrength_;
        }
    }
    if (ready_ && next == snap_) { return; }  // no change: skip repaint
    snap_ = next;
    notifyReady();
}

int WifiBackend::onPropsChanged(sd_bus_message* m, void* userdata, sd_bus_error* /*unused*/) {
    auto* self = static_cast<WifiBackend*>(userdata);
    const char* path = sd_bus_message_get_path(m);
    if (path == nullptr) { return 0; }

    // Only NM-wide and our-device/AP changes refresh; everything else is
    // irrelevant to the module (serialized by refreshAsync anyway).
    if (std::strcmp(path, kNMPath) != 0 && self->device_ != path && self->activeAp_ != path) {
        return 0;
    }
    self->refreshAsync();
    return 0;
}

int WifiBackend::onDeviceAdded(sd_bus_message* /*unused*/, void* userdata,
                               sd_bus_error* /*unused*/) {
    static_cast<WifiBackend*>(userdata)->refreshAsync();
    return 0;
}

int WifiBackend::onDeviceRemoved(sd_bus_message* /*unused*/, void* userdata,
                                 sd_bus_error* /*unused*/) {
    static_cast<WifiBackend*>(userdata)->refreshAsync();
    return 0;
}

int WifiBackend::onNameOwnerChanged(sd_bus_message* m, void* userdata, sd_bus_error* /*unused*/) {
    auto* self = static_cast<WifiBackend*>(userdata);
    const char* name = nullptr;
    const char* oldOwner = nullptr;
    const char* newOwner = nullptr;
    if (sd_bus_message_read(m, "sss", &name, &oldOwner, &newOwner) < 0 || (name == nullptr)) {
        return 0;
    }
    if (newOwner == nullptr || *newOwner == '\0') { return 0; }  // gone: keep last state
    self->refreshAsync();                                        // (re)appeared: re-enumerate
    return 0;
}

void WifiBackend::setEnabled(bool on) {
    if (!bus_.available()) { return; }

    snap_.enabled = on;
    if (!on) {
        snap_.connected = false;
        snap_.ssid.clear();
        snap_.strength = 0;
        activeAp_.clear();
    }
    notifyReady();

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
    if ((bus == nullptr) || device_.empty()) { return out; }

    std::unordered_set<std::string> saved;
    for (auto& p : savedConnections(bus)) { saved.insert(p.first); }

    std::string active;
    getObjectPath(bus, device_.c_str(), kWirelessIface, "ActiveAccessPoint", &active);

    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    if (sd_bus_get_property(bus, kNM, device_.c_str(), kWirelessIface, "AccessPoints", &err, &reply,
                            "ao") < 0 ||
        (reply == nullptr)) {
        sd_bus_error_free(&err);
        return out;
    }
    sd_bus_error_free(&err);

    std::vector<std::string> paths;
    if (sd_bus_message_enter_container(reply, 'a', "o") >= 0) {
        const char* p = nullptr;
        while (sd_bus_message_read(reply, "o", &p) > 0 && (p != nullptr)) { paths.emplace_back(p); }
        sd_bus_message_exit_container(reply);
    }
    sd_bus_message_unref(reply);

    std::unordered_map<std::string, size_t> bySsid;
    for (const auto& ap : paths) {
        std::string const ssid = readSsidProp(bus, ap.c_str(), kApIface);
        if (ssid.empty()) { continue; }
        const int strength = getY(bus, ap.c_str(), kApIface, "Strength");
        const bool secured = ((getU(bus, ap.c_str(), kApIface, "Flags") & kApFlagPrivacy) != 0U) ||
                             (getU(bus, ap.c_str(), kApIface, "WpaFlags") != 0U) ||
                             (getU(bus, ap.c_str(), kApIface, "RsnFlags") != 0U);
        const bool isActive = ap == active;

        auto it = bySsid.find(ssid);
        if (it != bySsid.end()) {
            WifiAp& e = out.at(it->second);
            e.strength = std::max(e.strength, strength);
            e.active = e.active || isActive;
            continue;
        }
        bySsid[ssid] = out.size();
        out.push_back({.ssid = ssid,
                       .strength = strength,
                       .secured = secured,
                       .active = isActive,
                       .saved = saved.contains(ssid)});
    }

    std::ranges::stable_sort(out, [](const WifiAp& a, const WifiAp& b) {
        if (a.active != b.active) { return a.active > b.active; }
        return a.strength > b.strength;
    });
    return out;
}

void WifiBackend::requestScan() {
    sd_bus* bus = bus_.get();
    if ((bus == nullptr) || device_.empty()) { return; }
    sd_bus_message* msg = nullptr;
    if (sd_bus_message_new_method_call(bus, &msg, kNM, device_.c_str(), kWirelessIface,
                                       "RequestScan") < 0) {
        return;
    }
    sd_bus_message_open_container(msg, 'a', "{sv}");
    sd_bus_message_close_container(msg);
    sd_bus_call_async(bus, nullptr, msg, nullptr, nullptr, 0);
    sd_bus_message_unref(msg);
}

std::string WifiBackend::findSavedConnection(const std::string& ssid) const {
    sd_bus* bus = bus_.get();
    if (bus == nullptr) { return {}; }
    for (auto& p : savedConnections(bus)) {
        if (p.first == ssid) { return p.second; }
    }
    return {};
}

void WifiBackend::connectSsid(const std::string& ssid) {
    sd_bus* bus = bus_.get();
    if ((bus == nullptr) || ssid.empty() || device_.empty()) { return; }
    const std::string conn = findSavedConnection(ssid);
    if (conn.empty()) { return; }
    sd_bus_call_method_async(bus, nullptr, kNM, kNMPath, kNM, "ActivateConnection", nullptr,
                             nullptr, "ooo", conn.c_str(), device_.c_str(), "/");
}

void WifiBackend::disconnect() {
    sd_bus* bus = bus_.get();
    if ((bus == nullptr) || device_.empty()) { return; }
    sd_bus_call_method_async(bus, nullptr, kNM, device_.c_str(), kDeviceIface, "Disconnect",
                             nullptr, nullptr, "");
}

}  // namespace qypr
