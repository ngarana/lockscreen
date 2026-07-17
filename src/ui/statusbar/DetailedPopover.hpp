// DetailedPopover.hpp - Base popover widget (glass card, popover contents).
#pragma once

#include "core/Types.hpp"
#include "ui/Theme.hpp"
#include <cstdint>

namespace qypr {

class Painter;

class DetailedPopover {
public:
    virtual ~DetailedPopover() = default;

    virtual void draw(Painter& p, int64_t now) = 0;
    virtual double contentHeight() const = 0;
    virtual double contentWidth() const { return 280.0; }

    // Input
    virtual bool handleClick(double x, double y) { return false; }
    virtual bool handleDrag(double x, double y) { return false; }
    virtual bool handleScroll(double dx, double dy) { return false; }
    virtual bool handleKey(uint32_t keysym) { return false; }

    // Polled by the host right after handleClick: return true (once) to ask the
    // manager to close this popover — e.g. a menu that just fired an item. The
    // default popover never self-closes.
    virtual bool consumeCloseRequest() { return false; }

    // Check if pointer is inside popover bounds
    bool contains(double px, double py) const {
        return getBounds().contains(px, py);
    }

    // Get popover rectangle bounds (calculated dynamically based on anchor)
    Rect getBounds() const {
        double w = contentWidth();
        double h = contentHeight();
        // Shift popover left so the anchor point aligns with the top-right of
        // popover. growUp extends upward from the anchor instead of down, so a
        // bottom-anchored bar opens its panels toward the screen centre.
        return {anchorX - w, growUp ? anchorY - h : anchorY, w, h};
    }

    // Anchor point (set by PopoverManager or StatusBar)
    double anchorX = 0;
    double anchorY = 0;
    // Open upward from the anchor (set by StatusBar for a bottom-edge bar).
    bool growUp = false;

    // Animation progress (0.0 to 1.0)
    Animated openProgress_{0.0};

    bool isOpen() const { return openProgress_.target() > 0.5; }
    void open() { openProgress_.animateTo(1.0, theme::anim::fast, ease::inOutQuad); }
    void close() { openProgress_.animateTo(0.0, theme::anim::fast, ease::inOutQuad); }
};

}  // namespace qypr
