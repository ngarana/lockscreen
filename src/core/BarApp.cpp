#include "core/BarApp.hpp"

#include <xkbcommon/xkbcommon-keysyms.h>

#include <cstdio>

#include "core/Types.hpp"
#include "render/Painter.hpp"
#include "wayland/Seat.hpp"  // Mod bits

namespace qypr {

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
    // arbitrary desktop wallpaper (the lock screen never does this).
    statusBar_.setBackdrop(true);

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
