// StatusBar.hpp - Container for zones, indicators, popovers, and layout.
#pragma once

#include <cairo/cairo.h>

#include <memory>
#include <vector>

#include "ui/Widget.hpp"
#include "ui/statusbar/PopoverManager.hpp"
#include "ui/statusbar/QuickSettingsPanel.hpp"
#include "ui/statusbar/StatusIndicator.hpp"

namespace qypr {

class EventLoop;
class Invalidator;

class StatusBar : public Widget {
public:
    // Deliberately takes Invalidator, not RenderHost: the status bar is
    // lock-agnostic and must stay hostable outside the lockscreen.
    StatusBar(EventLoop& loop, Invalidator& host, const SystemBackends& backends);
    ~StatusBar() override;

    // Layout the bar based on focused screen width
    void layout(int screenW, int screenH);

    void draw(Painter& p, int64_t now) override;

    bool animating(int64_t now) const;

    // Event routing. Handlers return true when the event was consumed.
    bool handlePointerMotion(double x, double y, int64_t now);
    bool handlePointerButton(double x, double y, uint32_t button, bool pressed, int64_t now);
    void handlePointerLeave(int64_t now);
    bool handleScroll(double x, double y, double dx, double dy);
    bool handleKey(uint32_t keysym);

    // Focus navigation
    bool cycleFocus(bool reverse);
    void clearFocus();
    bool hasFocusedChild() const;

    QuickSettingsPanel& quickSettings() { return qsPanel_; }
    PopoverManager& popovers() { return popovers_; }

    // Opt into showing session-sensitive indicators (workspaces, active
    // window). The lock screen never enables this, so those widgets stay
    // hidden while locked; the standalone (unlocked) bar turns it on.
    void setSessionContentVisible(bool v);

    // True while Quick Settings or a popover is open (or animating closed). The
    // layer-shell host grows its surface to full height when this is set so the
    // overlay — drawn at absolute coordinates — is visible and interactive.
    bool hasOpenOverlay() const;

    // Draw a subtle rounded backdrop behind the strip. Off by default (the lock
    // screen stays chromeless over its dark video); the standalone desktop bar
    // turns it on so the glyphs stay legible over an arbitrary wallpaper.
    void setBackdrop(bool enabled) { backdrop_ = enabled; }

private:
    void toggleQuickSettings();
    void activateIndicator(StatusIndicator& ind);
    void notifyBackendUpdate();

    // Effective visibility: a sensitive indicator is hidden unless session
    // content is enabled. All layout/draw/hit-testing goes through this.
    bool isShown(const StatusIndicator& ind) const {
        return ind.visible && (!ind.sensitive() || sessionContentVisible_);
    }

    EventLoop& loop_;
    Invalidator& host_;

    // Indicators split by zone
    std::vector<std::unique_ptr<StatusIndicator>> leftIndicators_;
    std::vector<std::unique_ptr<StatusIndicator>> centerIndicators_;
    std::vector<std::unique_ptr<StatusIndicator>> rightIndicators_;

    // Specialized trigger for Quick Settings
    Rect qsButtonBounds_;
    bool qsButtonHovered_ = false;

    // Popover Management
    QuickSettingsPanel qsPanel_;
    PopoverManager popovers_;

    // Gates session-sensitive indicators (default hidden — lock screen safe).
    bool sessionContentVisible_ = false;

    // Subtle strip backdrop (standalone desktop bar only; lock screen stays
    // chromeless). Off by default.
    bool backdrop_ = false;

    // 1s tick driving indicator poll() — the bar's own timer, never
    // LockScreen's (decoupling principle 1).
    int tickTimer_ = -1;

    // Offscreen 1x1 context so layout() can measure text without a frame.
    cairo_surface_t* measureSurface_ = nullptr;
    cairo_t* measureCr_ = nullptr;
};

}  // namespace qypr
