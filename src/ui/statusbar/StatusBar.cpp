// StatusBar.cpp - Container for zones, indicators, popovers, and layout implementation
#include "ui/statusbar/StatusBar.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"
#include "render/Painter.hpp"
#include "ui/Theme.hpp"
#include "core/Interfaces.hpp"

namespace qypr {

StatusBar::StatusBar(EventLoop& loop, RenderHost& host, const SystemBackends& backends)
    : loop_(loop), host_(host) {
    
    // Create all indicators registered by plugins
    auto all = IndicatorRegistry::instance().createAll(backends);
    for (auto& ind : all) {
        if (ind->zone() == Zone::Left) {
            leftIndicators_.push_back(std::move(ind));
        } else if (ind->zone() == Zone::Center) {
            centerIndicators_.push_back(std::move(ind));
        } else {
            rightIndicators_.push_back(std::move(ind));
        }
    }

    // Populate Quick Settings tiles from indicators
    for (const auto& ind : leftIndicators_) {
        if (auto tile = ind->createTile()) qsPanel_.addTile(std::move(tile));
    }
    for (const auto& ind : centerIndicators_) {
        if (auto tile = ind->createTile()) qsPanel_.addTile(std::move(tile));
    }
    for (const auto& ind : rightIndicators_) {
        if (auto tile = ind->createTile()) qsPanel_.addTile(std::move(tile));
    }
}

void StatusBar::layout(int screenW, int screenH) {
    double topMargin = theme::statusbar::topMargin;
    double sideMargin = theme::statusbar::sideMargin;
    double barH = theme::statusbar::height;
    double pad = theme::statusbar::padding;
    double sp = theme::statusbar::iconSpacing;

    bounds = {sideMargin, topMargin, screenW - 2 * sideMargin, barH};

    Painter dummyPainter(nullptr);

    // 1. Layout Left Zone (flows right)
    double lx = bounds.x + pad;
    for (auto& ind : leftIndicators_) {
        double w = ind->measureWidth(dummyPainter);
        ind->bounds = {lx, bounds.y, w, barH};
        lx += w + sp;
    }

    // 2. Layout Rightmost Gear Button ⚙
    double gearW = 32.0;
    qsButtonBounds_ = {bounds.x + bounds.w - pad - gearW, bounds.y, gearW, barH};

    // 3. Layout Right Zone (flows left from gear button)
    double rx = qsButtonBounds_.x - sp;
    for (auto it = rightIndicators_.rbegin(); it != rightIndicators_.rend(); ++it) {
        double w = (*it)->measureWidth(dummyPainter);
        rx -= w;
        (*it)->bounds = {rx, bounds.y, w, barH};
        rx -= sp;
    }

    // 4. Layout Center Zone
    double totalCenterW = 0;
    for (auto& ind : centerIndicators_) {
        totalCenterW += ind->measureWidth(dummyPainter);
    }
    if (!centerIndicators_.empty()) {
        totalCenterW += (centerIndicators_.size() - 1) * sp;
    }
    double cx = bounds.x + (bounds.w - totalCenterW) / 2.0;
    for (auto& ind : centerIndicators_) {
        double w = ind->measureWidth(dummyPainter);
        ind->bounds = {cx, bounds.y, w, barH};
        cx += w + sp;
    }

    // 5. Position popovers
    if (popovers_.active()) {
        if (popovers_.active() == &qsPanel_) {
            // Anchor quick settings to the right edge of the bar
            popovers_.active()->anchorX = bounds.x + bounds.w;
            popovers_.active()->anchorY = bounds.y + barH + 6.0;
        } else {
            // Anchor detailed popover to its corresponding indicator's center
            for (const auto& ind : rightIndicators_) {
                // Check if this indicator owns the active popover (detailed view)
                // Since popover is dynamic, we match by anchor position or save it on open.
            }
        }
    }
}

void StatusBar::draw(Painter& p, int64_t now) {
    if (!visible) return;

    // Draw glass bar background strip
    p.fillRoundedRect(bounds, theme::statusbar::cornerRadius, theme::color::glass);
    p.strokeRoundedRect(bounds, theme::statusbar::cornerRadius, theme::color::glassBorder, 1.0);

    // Draw all indicators
    for (auto& ind : leftIndicators_) {
        ind->hoverAlpha_.animateTo(ind->hovered ? 1.0 : 0.0, theme::anim::fast, ease::inOutQuad);
        ind->hoverScale_.animateTo(ind->hovered ? 1.05 : 1.0, theme::anim::fast, ease::inOutQuad);
        ind->draw(p, now);
    }
    for (auto& ind : centerIndicators_) {
        ind->hoverAlpha_.animateTo(ind->hovered ? 1.0 : 0.0, theme::anim::fast, ease::inOutQuad);
        ind->hoverScale_.animateTo(ind->hovered ? 1.05 : 1.0, theme::anim::fast, ease::inOutQuad);
        ind->draw(p, now);
    }
    for (auto& ind : rightIndicators_) {
        ind->hoverAlpha_.animateTo(ind->hovered ? 1.0 : 0.0, theme::anim::fast, ease::inOutQuad);
        ind->hoverScale_.animateTo(ind->hovered ? 1.05 : 1.0, theme::anim::fast, ease::inOutQuad);
        ind->draw(p, now);
    }

    // Draw Gear Button ⚙
    if (qsButtonHovered_) {
        p.fillRoundedRect(qsButtonBounds_, 8.0, theme::color::glassHover);
    }
    TextStyle gearStyle{theme::font::iconFamily, theme::statusbar::iconSize, PANGO_WEIGHT_NORMAL, theme::color::text};
    Size gearSz = p.measureText("⚙", gearStyle);
    p.drawText(qsButtonBounds_.x + (qsButtonBounds_.w - gearSz.w) / 2.0,
               qsButtonBounds_.y + (qsButtonBounds_.h - gearSz.h) / 2.0, "⚙", gearStyle);

    // Draw popovers
    popovers_.draw(p, now);
}

bool StatusBar::animating(int64_t now) const {
    if (popovers_.animating(now)) return true;
    for (const auto& ind : leftIndicators_) {
        if (ind->hoverAlpha_.active(now) || ind->hoverScale_.active(now)) return true;
    }
    for (const auto& ind : centerIndicators_) {
        if (ind->hoverAlpha_.active(now) || ind->hoverScale_.active(now)) return true;
    }
    for (const auto& ind : rightIndicators_) {
        if (ind->hoverAlpha_.active(now) || ind->hoverScale_.active(now)) return true;
    }
    return false;
}

bool StatusBar::handlePointerMotion(double x, double y, int64_t now) {
    if (popovers_.active()) {
        // Update popover hover tile states
        if (popovers_.active() == &qsPanel_) {
            Rect popBounds = qsPanel_.getBounds();
            // Layout is run in draw, but coordinates match
            // Handle slider drag first
            if (qsPanel_.activeDragTile_) {
                popovers_.handleDrag(x, y);
                host_.invalidate();
                return true;
            }
        }
    }

    // Hit test gear button
    bool lastGearHover = qsButtonHovered_;
    qsButtonHovered_ = qsButtonBounds_.contains(x, y);
    if (qsButtonHovered_ != lastGearHover) host_.invalidate();

    // Hit test indicators
    auto checkHover = [&](auto& list) {
        for (auto& ind : list) {
            bool prev = ind->hovered;
            ind->hovered = ind->bounds.contains(x, y);
            if (ind->hovered != prev) host_.invalidate();
        }
    };
    checkHover(leftIndicators_);
    checkHover(centerIndicators_);
    checkHover(rightIndicators_);

    return bounds.contains(x, y) || (popovers_.active() && popovers_.active()->contains(x, y));
}

bool StatusBar::handlePointerButton(double x, double y, uint32_t button, bool pressed, int64_t now) {
    if (popovers_.active()) {
        if (popovers_.active()->contains(x, y)) {
            if (pressed) {
                popovers_.handleClick(x, y);
            } else {
                // Release slider drag
                if (popovers_.active() == &qsPanel_) {
                    qsPanel_.activeDragTile_ = nullptr;
                }
            }
            host_.invalidate();
            return true;
        } else if (pressed) {
            // Clicked outside active popover: dismiss it
            popovers_.closeActive();
            host_.invalidate();
            return true;
        }
    }

    if (!pressed) return false;

    // Gear button click
    if (qsButtonBounds_.contains(x, y)) {
        toggleQuickSettings();
        host_.invalidate();
        return true;
    }

    // Indicator click
    auto checkClick = [&](auto& list) {
        for (auto& ind : list) {
            if (ind->bounds.contains(x, y)) {
                ind->onActivate();
                if (ind->hasDetailedView()) {
                    popovers_.open(ind->createDetailedView(), ind->bounds.x + ind->bounds.w, ind->bounds.y + ind->bounds.h + 6.0);
                } else {
                    toggleQuickSettings();
                }
                host_.invalidate();
                return true;
            }
        }
        return false;
    };

    if (checkClick(leftIndicators_)) return true;
    if (checkClick(centerIndicators_)) return true;
    if (checkClick(rightIndicators_)) return true;

    return bounds.contains(x, y);
}

void StatusBar::handlePointerLeave(int64_t now) {
    qsButtonHovered_ = false;
    auto clear = [&](auto& list) {
        for (auto& ind : list) {
            ind->hovered = false;
        }
    };
    clear(leftIndicators_);
    clear(centerIndicators_);
    clear(rightIndicators_);
    host_.invalidate();
}

bool StatusBar::handleScroll(double x, double y, double dx, double dy) {
    if (popovers_.active() && popovers_.active()->contains(x, y)) {
        return popovers_.handleScroll(dx, dy);
    }
    
    // Check if scrolled directly on a scroll-to-adjust indicator
    auto checkScroll = [&](auto& list) {
        for (auto& ind : list) {
            if (ind->bounds.contains(x, y)) {
                if (ind->onScroll(dx, dy)) {
                    host_.invalidate();
                    return true;
                }
            }
        }
        return false;
    };

    if (checkScroll(rightIndicators_)) return true;
    return false;
}

bool StatusBar::handleKey(uint32_t keysym) {
    if (popovers_.active()) {
        return popovers_.handleKey(keysym);
    }
    return false;
}

void StatusBar::toggleQuickSettings() {
    if (popovers_.active() == &qsPanel_) {
        popovers_.closeActive();
    } else {
        popovers_.openBorrowed(&qsPanel_, bounds.x + bounds.w, bounds.y + bounds.h + 6.0);
    }
}

bool StatusBar::cycleFocus(bool reverse) {
    // Collect all indicators in order
    std::vector<StatusIndicator*> inds;
    for (auto& ind : leftIndicators_) inds.push_back(ind.get());
    for (auto& ind : centerIndicators_) inds.push_back(ind.get());
    for (auto& ind : rightIndicators_) inds.push_back(ind.get());

    if (inds.empty()) return false;

    // Find current focused
    int current = -1;
    for (size_t i = 0; i < inds.size(); ++i) {
        if (inds[i]->focused) {
            current = static_cast<int>(i);
            break;
        }
    }

    if (current == -1) {
        // Focus first or last
        int target = reverse ? static_cast<int>(inds.size()) - 1 : 0;
        inds[target]->focused = true;
        host_.invalidate();
        return true;
    }

    // Move focus
    inds[current]->focused = false;
    int next = current + (reverse ? -1 : 1);
    if (next >= 0 && next < static_cast<int>(inds.size())) {
        inds[next]->focused = true;
        host_.invalidate();
        return true;
    }

    // Off edge
    host_.invalidate();
    return false;
}

void StatusBar::clearFocus() {
    auto clear = [&](auto& list) {
        for (auto& ind : list) ind->focused = false;
    };
    clear(leftIndicators_);
    clear(centerIndicators_);
    clear(rightIndicators_);
    host_.invalidate();
}

bool StatusBar::hasFocusedChild() const {
    for (const auto& ind : leftIndicators_) if (ind->focused) return true;
    for (const auto& ind : centerIndicators_) if (ind->focused) return true;
    for (const auto& ind : rightIndicators_) if (ind->focused) return true;
    return false;
}

}  // namespace qypr
