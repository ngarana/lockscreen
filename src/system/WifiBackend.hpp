// WifiBackend.hpp - WiFi state via NetworkManager on the shared system bus.
//
// Async startup: all D-Bus calls use sd_bus_call_method_async so the event
// loop is never blocked. Push-only afterwards via PropertiesChanged.

#pragma once

#include <functional>
#include <string>
#include <vector>

struct sd_bus_message;
struct sd_bus_slot;
struct sd_bus_error;

namespace qypr {

class SystemBus;

struct WifiSnapshot {
    bool available = false;
    bool enabled = false;
    bool connected = false;
    std::string ssid;
    int strength = 0;

    bool operator==(const WifiSnapshot&) const = default;
};

struct WifiAp {
    std::string ssid;
    int strength = 0;
    bool secured = false;
    bool active = false;
    bool saved = false;
};

class WifiBackend {
public:
    explicit WifiBackend(SystemBus& bus);
    ~WifiBackend();

    WifiBackend(const WifiBackend&) = delete;
    WifiBackend& operator=(const WifiBackend&) = delete;

    bool start();

    const WifiSnapshot& snapshot() const { return snap_; }

    void setOnChange(std::function<void()> cb) { onChange_ = std::move(cb); }

    // True once the backend has produced its first result — real data or a
    // definitive "absent". Indicators show a neutral placeholder until then, so
    // an unrelated backend's push cannot prematurely mark this one loaded.
    bool ready() const { return ready_; }

    void setEnabled(bool on);
    std::vector<WifiAp> scanNetworks() const;
    void requestScan();
    void connectSsid(const std::string& ssid);
    void disconnect();

private:
    enum class RefreshStep { WirelessEnabled, ActiveAccessPoint, Ssid, Strength };

    static int onGetDevices(sd_bus_message* reply, void* userdata, sd_bus_error* err);
    void refreshAsync();
    static int onRefreshStep(sd_bus_message* reply, void* userdata, sd_bus_error* err);
    void subscribeSignal();
    std::string findSavedConnection(const std::string& ssid) const;

    static int onPropsChanged(sd_bus_message* m, void* userdata, sd_bus_error* err);

    SystemBus& bus_;
    sd_bus_slot* slot_ = nullptr;
    std::string device_;
    std::string activeAp_;
    RefreshStep refreshStep_ = RefreshStep::WirelessEnabled;
    WifiSnapshot pendingSnap_;  // built up across async refresh steps
    WifiSnapshot snap_;
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
