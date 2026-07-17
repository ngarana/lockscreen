// BarApp.hpp - Standalone status-bar shell for the unlocked desktop.
//
// A waybar replacement: it hosts the *same* StatusBar as the lock screen, but
// on wlr-layer-shell instead of a session-lock surface, and turns on the
// session-sensitive WM widgets (workspaces + active window) that the lock
// screen keeps hidden. No LockScreen, no PAM, no video — just the bar.
//
// Implements Invalidator (StatusBar's repaint hook — never RenderHost, so the
// bar cannot reach lock-only powers) and InputSink (pointer routing from Seat).

#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "core/Config.hpp"
#include "core/EventLoop.hpp"
#include "core/Interfaces.hpp"
#include "mpris/MprisController.hpp"
#include "notifications/NotificationActions.hpp"
#include "notifications/NotificationMonitor.hpp"
#include "power/PowerManager.hpp"
#include "system/BatteryBackend.hpp"
#include "system/BluetoothBackend.hpp"
#include "system/BrightnessBackend.hpp"
#include "system/DbusMenuBackend.hpp"
#include "system/DndState.hpp"
#include "system/SNIBackend.hpp"
#include "system/SystemBus.hpp"
#include "system/ToplevelBackend.hpp"
#include "system/VolumeBackend.hpp"
#include "system/WifiBackend.hpp"
#include "system/WorkspaceBackend.hpp"
#include "ui/statusbar/StatusBar.hpp"
#include "wayland/BarDisplay.hpp"

namespace qypr {

class BarApp : public Invalidator, public InputSink {
public:
    BarApp();

    int run();

    // Invalidator
    void invalidate() override;

    // InputSink
    void onTextInput(const std::string& utf8) override;
    void onSpecialKey(uint32_t keysym, uint32_t modifiers) override;
    void onPointerMotion(int w, int h, double x, double y) override;
    void onPointerButton(int w, int h, double x, double y, uint32_t button, bool pressed) override;
    void onPointerScroll(int w, int h, double x, double y, double dx, double dy) override;
    void onPointerLeave() override;

private:
    void draw(cairo_t* cr, int w, int h, int scale);
    // Grow/shrink the layer surfaces when the open-overlay state flips.
    void syncOverlay();

    // --- config helpers (used in the member-init list; see the ctor) ---
    static Config loadConfig();
    static BarGeometry readGeometry(const Config& c);
    // std::nullopt when the config names no modules — StatusBar then falls back
    // to every registered indicator (the compiled default bar).
    static std::optional<IndicatorRegistry::ModuleSelection> readModules(const Config& c);
    // Strip footprint = edge gap + bar height + a 6px breathing gap. This is the
    // exclusive zone and the idle surface height.
    static int reservedFor(const BarGeometry& g) {
        return static_cast<int>(g.edgeMargin + g.height + 6.0);
    }

    // Declaration order is initialisation order: config_ must precede
    // everything that reads it (geometry_, modules_, display_, statusBar_).
    Config config_{loadConfig()};
    BarGeometry geom_{readGeometry(config_)};
    std::optional<IndicatorRegistry::ModuleSelection> modules_{readModules(config_)};

    EventLoop loop_;
    BarDisplay display_{loop_, reservedFor(geom_), geom_.bottom};

    // One shared connection per bus (system + session), same as the lock app.
    SystemBus systemBus_{loop_};
    SystemBus sessionBus_{loop_, BusKind::Session};
    BatteryBackend battery_{systemBus_};
    BrightnessBackend brightness_{loop_, systemBus_};
    WifiBackend wifi_{systemBus_};
    BluetoothBackend bluetooth_{systemBus_};
    VolumeBackend volume_{loop_};
    SNIBackend sni_{sessionBus_};
    // Tray item context menus, on the shared session bus (bar-only).
    DbusMenuBackend dbusMenu_{sessionBus_};
    WorkspaceBackend workspace_;  // ext-workspace-v1 (on display_'s wl_display)
    ToplevelBackend toplevel_;    // active window (wlr-foreign-toplevel)
    DndState dnd_;
    // Session surface (Phase 10): already-built subsystems the lock app owns
    // too — the bar simply hosts them as applets.
    NotificationMonitor notifications_{loop_};
    // Dismissal must not ride the monitor connection (BecomeMonitor may never
    // send); it reuses the shared session bus instead.
    NotificationActions notificationActions_{sessionBus_};
    PowerManager power_;
    MprisController mpris_;
    SystemBackends backends_{.battery = &battery_,
                             .volume = &volume_,
                             .brightness = &brightness_,
                             .wifi = &wifi_,
                             .bluetooth = &bluetooth_,
                             .sni = &sni_,
                             .dbusMenu = &dbusMenu_,
                             .workspace = &workspace_,
                             .toplevel = &toplevel_,
                             .dnd = &dnd_,
                             .power = &power_,
                             .notifications = &notifications_,
                             .notificationActions = &notificationActions_,
                             .mpris = &mpris_,
                             .config = &config_};

    StatusBar statusBar_{loop_, *this, backends_, modules_ ? &*modules_ : nullptr};
    bool overlayActive_ = false;
};

}  // namespace qypr
