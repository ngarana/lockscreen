// Painter.hpp - Thin cairo + pango drawing helpers.
//
// Wraps the low-level cairo/pango calls the widgets need (rounded rects,
// circles, gradients, measured/aligned/ellipsised text, text shadows) so no
// widget repeats them (DRY). Holds no state beyond the borrowed cairo context.

#pragma once

#include <cairo/cairo.h>
#include <pango/pangocairo.h>

#include <string>

#include "core/Types.hpp"

namespace qypr {

enum class HAlign { Left, Center, Right };

struct TextStyle {
    std::string family = "Inter";
    double size = 16;
    PangoWeight weight = PANGO_WEIGHT_NORMAL;
    Color color = {1, 1, 1, 1};
};

struct Size {
    double w = 0, h = 0;
};

class Painter {
public:
    explicit Painter(cairo_t* cr) : cr_(cr) {}

    cairo_t* cr() const { return cr_; }

    // Group opacity: draw a whole widget, then composite it at one alpha.
    void pushGroup() { cairo_push_group(cr_); }
    void popGroupWithAlpha(double alpha) {
        cairo_pop_group_to_source(cr_);
        cairo_paint_with_alpha(cr_, alpha);
    }

    // Shapes
    void fillRect(const Rect& r, const Color& c);
    void fillRoundedRect(const Rect& r, double radius, const Color& c);
    void strokeRoundedRect(const Rect& r, double radius, const Color& c, double lineWidth);
    void fillCircle(double cx, double cy, double radius, const Color& c);
    void strokeCircle(double cx, double cy, double radius, const Color& c, double lineWidth);

    // Full-cover vertical 3-stop gradient (the lock background).
    void verticalGradient(int w, int h, const Color& top, const Color& mid, const Color& bottom);

    // Text
    Size measureText(const std::string& text, const TextStyle& style, double maxWidth = -1);
    // Draws text anchored at (x, y): x is left/center/right per align, y is the top.
    void drawText(double x, double y, const std::string& text, const TextStyle& style,
                  HAlign align = HAlign::Left, double maxWidth = -1);
    // Same, but paints an offset drop-shadow underneath for readability.
    void drawTextShadowed(double x, double y, const std::string& text, const TextStyle& style,
                          HAlign align, double shadowAlpha, double shadowOffset);

private:
    // Builds a configured, ellipsised layout the caller must g_object_unref.
    PangoLayout* makeLayout(const std::string& text, const TextStyle& style, double maxWidth);
    double anchorX(double x, double layoutW, HAlign align);

    cairo_t* cr_;
};

}  // namespace qypr
