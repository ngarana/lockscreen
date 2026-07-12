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

private:
    void toggleQuickSettings();
    void activateIndicator(StatusIndicator& ind);
    void notifyBackendUpdate();

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

    // 1s tick driving indicator poll() — the bar's own timer, never
    // LockScreen's (decoupling principle 1).
    int tickTimer_ = -1;

    // Offscreen 1x1 context so layout() can measure text without a frame.
    cairo_surface_t* measureSurface_ = nullptr;
    cairo_t* measureCr_ = nullptr;
};

}  // namespace qypr
