#include "core/BarApp.hpp"

#include <xkbcommon/xkbcommon-keysyms.h>

#include <algorithm>
#include <cstdio>
#include <string>

#include "core/Types.hpp"
#include "render/Painter.hpp"
#include "wayland/Seat.hpp"  // Mod bits

namespace qypr {

// -----------------------------------------------------------------------------
// Config (Phase 9). Every knob falls back to the compiled default, so a missing
// or partial bar.conf still yields exactly the bar we shipped before.
// -----------------------------------------------------------------------------
Config BarApp::loadConfig() {
    Config c;
    if (c.load()) {
        std::fprintf(stderr, "qypr-bar: config %s\n", c.path().c_str());
    }
    return c;
}

BarGeometry BarApp::readGeometry(const Config& c) {
    BarGeometry g;  // defaults = theme constants (the lockscreen strip)
    g.height = c.getDouble("bar", "height", g.height);
    g.edgeMargin = c.getDouble("bar", "margin", g.edgeMargin);
    g.sideMargin = c.getDouble("bar", "margin-side", g.sideMargin);
    const std::string pos = c.getString("bar", "position", "top");
    g.bottom = (pos == "bottom");
    if (pos != "top" && pos != "bottom") {
        std::fprintf(stderr, "qypr-bar: unknown position '%s' (want top|bottom); using top\n",
                     pos.c_str());
    }
    return g;
}

std::optional<IndicatorRegistry::ModuleSelection> BarApp::readModules(const Config& c) {
    const bool any = c.has("bar", "modules-left") || c.has("bar", "modules-center") ||
                     c.has("bar", "modules-right");
    if (!any) return std::nullopt;  // no module keys: keep the compiled default set

    IndicatorRegistry::ModuleSelection sel;
    sel.left = c.getList("bar", "modules-left");
    sel.center = c.getList("bar", "modules-center");
    sel.right = c.getList("bar", "modules-right");

    // A typo silently drops a module, which is baffling in a bar you cannot
    // introspect — so name the unknown ids and list what was available.
    const auto known = IndicatorRegistry::instance().registeredIds();
    for (const auto* zone : {&sel.left, &sel.center, &sel.right}) {
        for (const auto& id : *zone) {
            if (std::find(known.begin(), known.end(), id) == known.end()) {
                std::string all;
                for (const auto& k : known) all += (all.empty() ? "" : ", ") + k;
                std::fprintf(stderr, "qypr-bar: unknown module '%s' (have: %s)\n", id.c_str(),
                             all.c_str());
            }
        }
    }
    return sel;
}

BarApp::BarApp() = default;

int BarApp::run() {
    if (!display_.connect()) {
        std::fprintf(stderr,
                     "qypr-bar: no Wayland display or no wlr-layer-shell support\n");
        return 1;
    }

    display_.setInputSink(this);
    display_.setRenderFn([this](cairo_t* cr, int w, int h, int s) { draw(cr, w, h, s); });
    display_.setAnimatingFn([this] { return statusBar_.animating(nowMs()); });

    // This is the unlocked bar: reveal the session-sensitive WM widgets and
    // start their backends on the same wl_display (they bind their own registry).
    statusBar_.setSessionContentVisible(true);
    // Give the chromeless strip a subtle backdrop so it stays legible over an
    // arbitrary desktop wallpaper (the lock screen never does this). `backdrop`
    // is 0..1; 0 restores the pure chromeless look.
    const double alpha = config_.getDouble("bar", "backdrop", -1.0);
    statusBar_.setBackdrop(alpha != 0.0, alpha);
    statusBar_.setGeometry(geom_);

    // Status bar backends: one startup fetch, push-only afterwards (each is
    // non-fatal — a missing daemon just hides its indicator).
    battery_.start();
    brightness_.start();
    wifi_.start();
    bluetooth_.start();
    volume_.start();
    sni_.start();
    workspace_.start(display_.display());
    toplevel_.start(display_.display());

    loop_.run();
    return 0;
}

void BarApp::invalidate() { display_.invalidateAll(); }

void BarApp::draw(cairo_t* cr, int w, int h, int scale) {
    // Transparent surface. Reused shm buffers keep old pixels, so clear first,
    // then let StatusBar paint its chromeless strip (and any open overlay). The
    // desktop shows through everywhere the bar does not draw.
    cairo_save(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_restore(cr);

    Painter p(cr);
    statusBar_.layout(w, h);
    statusBar_.draw(p, nowMs());

    // Keep the surface height in step with the overlay state (deferred; never
    // resize mid-render).
    syncOverlay();
}

void BarApp::syncOverlay() {
    const bool want = statusBar_.hasOpenOverlay();
    if (want == overlayActive_) return;
    overlayActive_ = want;
    // Defer the resize+commit off the current dispatch/render so we never
    // double-commit a surface that is mid-frame.
    loop_.post([this, want] { display_.setOverlayActive(want); });
}

// -----------------------------------------------------------------------------
// InputSink — route straight to StatusBar (no LockScreen peer to arbitrate).
// -----------------------------------------------------------------------------
void BarApp::onTextInput(const std::string&) {}

void BarApp::onSpecialKey(uint32_t keysym, uint32_t modifiers) {
    if (keysym == XKB_KEY_Tab || keysym == XKB_KEY_ISO_Left_Tab) {
        const bool reverse = keysym == XKB_KEY_ISO_Left_Tab || (modifiers & MOD_SHIFT);
        if (!statusBar_.cycleFocus(reverse)) statusBar_.clearFocus();
    } else {
        statusBar_.handleKey(keysym);
    }
    invalidate();
    syncOverlay();
}

void BarApp::onPointerMotion(int, int, double x, double y) {
    statusBar_.handlePointerMotion(x, y, nowMs());
    invalidate();
}

void BarApp::onPointerButton(int, int, double x, double y, uint32_t button, bool pressed) {
    statusBar_.handlePointerButton(x, y, button, pressed, nowMs());
    invalidate();
    syncOverlay();
}

void BarApp::onPointerScroll(int, int, double x, double y, double dx, double dy) {
    statusBar_.handleScroll(x, y, dx, dy);
    invalidate();
    syncOverlay();
}

void BarApp::onPointerLeave() {
    statusBar_.handlePointerLeave(nowMs());
    invalidate();
}

}  // namespace qypr
