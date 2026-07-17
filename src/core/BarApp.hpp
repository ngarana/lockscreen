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
#include <string>

#include "core/EventLoop.hpp"
#include "core/Interfaces.hpp"
#include "system/BatteryBackend.hpp"
#include "system/BluetoothBackend.hpp"
#include "system/BrightnessBackend.hpp"
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

    // Reserved strip: topMargin(24) + bar height(36) + a 6px gap, matching the
    // chromeless floating strip the lock screen draws.
    static constexpr int kReserved = 66;

    EventLoop loop_;
    BarDisplay display_{loop_, kReserved};

    // One shared connection per bus (system + session), same as the lock app.
    SystemBus systemBus_{loop_};
    SystemBus sessionBus_{loop_, BusKind::Session};
    BatteryBackend battery_{systemBus_};
    BrightnessBackend brightness_{loop_, systemBus_};
    WifiBackend wifi_{systemBus_};
    BluetoothBackend bluetooth_{systemBus_};
    VolumeBackend volume_{loop_};
    SNIBackend sni_{sessionBus_};
    WorkspaceBackend workspace_;  // ext-workspace-v1 (on display_'s wl_display)
    ToplevelBackend toplevel_;    // active window (wlr-foreign-toplevel)
    DndState dnd_;
    SystemBackends backends_{.battery = &battery_,
                             .volume = &volume_,
                             .brightness = &brightness_,
                             .wifi = &wifi_,
                             .bluetooth = &bluetooth_,
                             .sni = &sni_,
                             .workspace = &workspace_,
                             .toplevel = &toplevel_,
                             .dnd = &dnd_};

    StatusBar statusBar_{loop_, *this, backends_};
    bool overlayActive_ = false;
};

}  // namespace qypr
