// BrightnessBackend.hpp - Backlight state via sysfs, pushed via udev,
// written via logind.
//
// Reads /sys/class/backlight/<dev>/{brightness,max_brightness}. External
// changes (brightness keys, other tools) arrive as kernel "change" uevents
// on the udev netlink monitor — push, no polling. Writes go through
// org.freedesktop.login1 Session.SetBrightness on the shared system bus
// (the session owner may call it unprivileged; no sysfs write permissions
// needed). Everything is fd-driven on the epoll EventLoop.

#pragma once

#include <cstdint>
#include <functional>
#include <string>

struct udev;
struct udev_monitor;

namespace qypr {

class EventLoop;
class SystemBus;

struct BrightnessSnapshot {
    int current = 0;            // raw hardware value
    int max = 0;                // raw hardware maximum
    bool available = false;
    std::string device;         // e.g. "intel_backlight"

    double fraction() const { return max > 0 ? static_cast<double>(current) / max : 0.0; }
};

class BrightnessBackend {
public:
    BrightnessBackend(EventLoop& loop, SystemBus& bus);
    ~BrightnessBackend();

    BrightnessBackend(const BrightnessBackend&) = delete;
    BrightnessBackend& operator=(const BrightnessBackend&) = delete;

    // Scan sysfs and subscribe to backlight uevents. Returns false when no
    // backlight device exists (indicator stays hidden).
    bool start();

    const BrightnessSnapshot& snapshot() const { return snap_; }

    // Fires on every pushed update (and once after a successful start).
    void setOnChange(std::function<void()> cb) { onChange_ = std::move(cb); }

    // Set brightness as a fraction 0..1 (clamped; never fully off). Applied
    // optimistically to the snapshot, written asynchronously via logind.
    void setFraction(double frac);

private:
    bool readSysfs();
    void onUdevEvent();

    EventLoop& loop_;
    SystemBus& bus_;
    BrightnessSnapshot snap_;
    std::function<void()> onChange_;

    struct udev* udev_ = nullptr;
    struct udev_monitor* mon_ = nullptr;
    int monFd_ = -1;
};

}  // namespace qypr
