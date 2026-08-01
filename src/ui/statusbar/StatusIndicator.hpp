// StatusIndicator.hpp - Base class for all status bar applets.
#pragma once

#include "ui/Widget.hpp"
#include "core/Types.hpp"
#include "ui/statusbar/QSTile.hpp"
#include "ui/statusbar/DetailedPopover.hpp"
#include <string>
#include <memory>

// Forward declaration — used by the symbolic-icon cache (recolourable surfaces
// from the active icon theme). Cairo's full header is pulled in transitively by
// Painter.
typedef struct _cairo_surface cairo_surface_t;

namespace qypr {

enum class Zone { Left, Center, Right };

// Forward declarations of all backends
class BatteryBackend;
class VolumeBackend;
class BrightnessBackend;
class WifiBackend;
class BluetoothBackend;
class SNIBackend;
class DbusMenuBackend;
class WorkspaceBackend;
class ToplevelBackend;
class DndState;
class DndState;
class NightLightBackend;
class Config;
class PowerManager;
class PowerProfilesBackend;
class NotificationMonitor;
class NotificationActions;
class MprisController;
class IdleInhibitor;
class DesktopIndex;
class KeyboardLayout;

struct SystemBackends {
    BatteryBackend* battery = nullptr;
    VolumeBackend* volume = nullptr;
    BrightnessBackend* brightness = nullptr;
    WifiBackend* wifi = nullptr;
    BluetoothBackend* bluetooth = nullptr;
    SNIBackend* sni = nullptr;
    // Tray item context menus (com.canonical.dbusmenu). Supplied only by the
    // unlocked qypr-bar: the lock screen leaves this null so a locked machine
    // cannot open an app's menu (e.g. nm-applet's connection editor).
    DbusMenuBackend* dbusMenu = nullptr;
    WorkspaceBackend* workspace = nullptr;  // ext-workspace-v1 (session-sensitive)
    ToplevelBackend* toplevel = nullptr;    // active window (session-sensitive)
    DndState* dnd = nullptr;
    NightLightBackend* nightLight = nullptr;
    // Session surface (Phase 10): supplied only by the unlocked qypr-bar. The
    // lock screen leaves these null — it has its own in-lockscreen power dialog
    // and notification stack, and must never offer a shutdown button or reveal
    // notification contents from the bar.
    PowerManager* power = nullptr;
    PowerProfilesBackend* powerProfiles = nullptr;  // net.hadess.PowerProfiles
    IdleInhibitor* idleInhibitor = nullptr;         // zwp_idle_inhibit ("keep awake")
    DesktopIndex* desktopIndex = nullptr;
    KeyboardLayout* keyboardLayout = nullptr;
    NotificationMonitor* notifications = nullptr;
    NotificationActions* notificationActions = nullptr;  // dismiss/clear
    MprisController* mpris = nullptr;
    // User config, or nullptr when the host has none (qypr-lock). Indicators
    // read their own `[<id>]` section; every key must have a compiled default so
    // a null config is always valid.
    const Config* config = nullptr;
    // True only on the unlocked qypr-bar. Distinct from the individual session
    // backends above: it gates session-revealing *detail* that lives inside an
    // otherwise-safe applet — e.g. the audio panel's per-app stream list (app
    // names disclose what you are running) while keeping device switching.
    bool sessionSurface = false;
};

class StatusIndicator : public Widget {
public:
    StatusIndicator(const std::string& id, Zone zone, int priority)
        : id_(id), zone_(zone), priority_(priority) {}

    ~StatusIndicator() override;

    // --- Tray View (compact bar representation) ---
    // icon() may return "" for text-only indicators (e.g. the clock); the
    // base draw then renders just the label.
    virtual std::string icon() const = 0;
    // Preferred representation: a freedesktop symbolic icon name (e.g.
    // "network-wireless-signal-excellent-symbolic") resolved through the *active*
    // system icon theme — whatever gtk-icon-theme-name points at. Return "" for a
    // glyph-only indicator. Whether it is actually used depends on
    // theme::icons::mode and on the active theme providing the name; otherwise the
    // base draw falls back to the Nerd Font glyph from icon(). The base caches one
    // cairo surface per unique name and recolours it to iconColor() by masking —
    // symbolic SVGs (black-with-alpha) are designed for exactly this.
    virtual std::string themedIcon() const { return ""; }
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

    // True while this indicator still owes the frame loop an animation (hover
    // scale/alpha, charging pulse, …). The host asks every frame and keeps the
    // loops alive while any indicator returns true.
    virtual bool animating(int64_t now) const;

    // --- Input (forwarded by StatusBar) ---
    // `x,y` are pointer coordinates, so multi-element indicators (the tray host)
    // can route the gesture to the specific sub-item under the cursor. Indicators
    // that don't use the position can leave the args bound but unused.
    virtual bool onScroll(double dx, double dy, double x, double y) { (void)x; (void)y; (void)dx; (void)dy; return false; }
    // Custom per-position click handling (e.g. the tray host, which maps the
    // click to one of several sub-icons). Return true to consume; false falls
    // through to the default activate (tile/popover).
    virtual bool onClick(double x, double y) { return false; }
    // Secondary (right) and middle clicks. Unlike onClick, a false return does
    // NOT fall through to the default activate — an unhandled right/middle click
    // is simply a no-op. Used by the tray host (right = dbusmenu, middle =
    // SecondaryActivate) and the taskbar (middle = close).
    virtual bool onSecondaryClick(double x, double y) { return false; }
    virtual bool onMiddleClick(double x, double y) { return false; }
    bool interactive() const override { return true; }

    // Session-sensitive indicators reveal what you are doing (workspaces, the
    // focused window). The host hides these unless it has opted into showing
    // session content — so they never appear on the lock screen. See
    // StatusBar::setSessionContentVisible.
    virtual bool sensitive() const { return false; }

    // QS-only indicators live only in the Quick Settings panel and never appear
    // as bar applets.  The StatusBar skips them during layout/draw while still
    // calling createTile() so their QS toggle is present.
    virtual bool qsOnly() const { return false; }

    // --- Getters ---
    std::string id() const { return id_; }
    Zone zone() const { return zone_; }
    // Re-home this indicator. Set by IndicatorRegistry when config lists a
    // module under a zone other than its compiled-in default; StatusBar buckets
    // by zone() at construction, so this must be called before that.
    void setZone(Zone z) { zone_ = z; }
    int priority() const { return priority_; }

    bool hovered = false;
    bool focused = false;
    Animated hoverScale_{1.0};
    Animated hoverAlpha_{0.0};

protected:
    std::string id_;
    Zone zone_;
    int priority_;

    // Placeholder-until-loaded (instant startup, waybar/polybar style). A
    // hardware-presence indicator (battery, wifi, bluetooth, volume, brightness)
    // reserves its slot and draws a neutral resting glyph with no value text from
    // the first frame, then flips this true on the first backend push so real
    // data replaces the placeholder in place — no appear-and-reflow lag. It is
    // set true immediately when the indicator has no backend (tests / registry
    // previews render their sample defaults). Content indicators (media, tray,
    // taskbar, …) ignore this and stay hidden until they have something to show.
    bool loaded_ = false;

    // ── Icon crossfade (Phase 6 polish) ─────────────────────────────────
    // When icon() changes between draws, the base draws the outgoing glyph
    // fading out under the incoming one over `theme::anim::medium` (300ms,
    // ease-in-out). Both share the icon footprint so the swap reads as a
    // dissolve, not a hard cut. Battery (level/charging) and WiFi (signal
    // tiers) get this for free by going through the base draw.
    std::string lastDrawnIcon_;     // most-recent icon() the base has rendered
    std::string prevDrawnIcon_;     // outgoing glyph during a crossfade
    int64_t crossfadeStartMs_ = 0;  // wall clock the swap started
    static constexpr int kCrossfadeMs = 300;  // cf. theme::anim::medium

    // ── Symbolic icon cache (recolourable surfaces from the active theme) ──
    // Resolved via IconResolver from the active system icon theme and drawn with
    // `iconColor()` as the recolour tint. Keyed by the themedIcon() name so a
    // battery tier change refreshes the surface lazily on the next draw.
    cairo_surface_t* themedIconCache_ = nullptr;
    std::string themedIconCacheKey_;
    cairo_surface_t* themedIconSurface();

    // The symbolic surface to actually display this frame, or nullptr to render
    // the Nerd Font glyph instead. Applies theme::icons::mode and returns null
    // when the active theme does not provide the name (graceful fallback). Shared
    // by measureWidth() and draw() so layout and paint always agree.
    cairo_surface_t* displayIconSurface();
};

}  // namespace qypr
