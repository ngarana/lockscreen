// StatusIndicator.hpp - Base class for all status bar applets.
#pragma once

#include "ui/Widget.hpp"
#include "core/Types.hpp"
#include "ui/statusbar/QSTile.hpp"
#include "ui/statusbar/DetailedPopover.hpp"
#include <string>
#include <memory>

namespace qypr {

enum class Zone { Left, Center, Right };

// Forward declarations of all backends
class BatteryBackend;
class VolumeBackend;
class BrightnessBackend;
class WifiBackend;
class BluetoothBackend;
class SNIBackend;
class DndState;

struct SystemBackends {
    BatteryBackend* battery = nullptr;
    VolumeBackend* volume = nullptr;
    BrightnessBackend* brightness = nullptr;
    WifiBackend* wifi = nullptr;
    BluetoothBackend* bluetooth = nullptr;
    SNIBackend* sni = nullptr;
    DndState* dnd = nullptr;
};

class StatusIndicator : public Widget {
public:
    StatusIndicator(const std::string& id, Zone zone, int priority)
        : id_(id), zone_(zone), priority_(priority) {}

    virtual ~StatusIndicator() = default;

    // --- Tray View (compact bar representation) ---
    // icon() may return "" for text-only indicators (e.g. the clock); the
    // base draw then renders just the label.
    virtual std::string icon() const = 0;
    virtual std::string label() const { return ""; }
    virtual std::string tooltip() const = 0;
    virtual Color iconColor() const;
    // Point size of the label text; text-only indicators bump this up.
    virtual double labelFontSize() const { return 13.0; }

    // Measure indicator width based on current icon/label/etc.
    virtual double measureWidth(Painter& p);

    // Render compact view inside bounds
    void draw(Painter& p, int64_t now) override;

    // --- Default View (Quick Settings Tile) ---
    virtual std::unique_ptr<QSTile> createTile() { return nullptr; }

    // --- Detailed View (Individual Popover) ---
    virtual bool hasDetailedView() const { return false; }
    virtual std::unique_ptr<DetailedPopover> createDetailedView() { return nullptr; }

    // --- Lifecycle ---
    virtual void poll(int64_t now) {}
    virtual void onBackendUpdate() {}
    virtual void onActivate() {}

    // --- Input (forwarded by StatusBar) ---
    virtual bool onScroll(double dx, double dy) { return false; }
    bool interactive() const override { return true; }

    // --- Getters ---
    std::string id() const { return id_; }
    Zone zone() const { return zone_; }
    int priority() const { return priority_; }

    bool hovered = false;
    bool focused = false;
    Animated hoverScale_{1.0};
    Animated hoverAlpha_{0.0};

protected:
    std::string id_;
    Zone zone_;
    int priority_;
};

}  // namespace qypr
