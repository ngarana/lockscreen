// StatusBar.hpp - Container for zones, indicators, popovers, and layout.
#pragma once

#include "ui/Widget.hpp"
#include "ui/statusbar/StatusIndicator.hpp"
#include "ui/statusbar/PopoverManager.hpp"
#include "ui/statusbar/QuickSettingsPanel.hpp"
#include <vector>
#include <memory>

namespace qypr {

class EventLoop;
class RenderHost;

class StatusBar : public Widget {
public:
    StatusBar(EventLoop& loop, RenderHost& host, const SystemBackends& backends);
    ~StatusBar() override = default;

    // Layout the bar based on focused screen width
    void layout(int screenW, int screenH);

    void draw(Painter& p, int64_t now) override;

    bool animating(int64_t now) const;

    // Event routing
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

    EventLoop& loop_;
    RenderHost& host_;

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
};

}  // namespace qypr
