// StatusBar.cpp - Container for zones, indicators, popovers, and layout implementation
#include "ui/statusbar/StatusBar.hpp"

#include <xkbcommon/xkbcommon-keysyms.h>

#include "core/EventLoop.hpp"
#include "core/Interfaces.hpp"
#include "core/Types.hpp"
#include "render/Painter.hpp"
#include "system/BatteryBackend.hpp"
#include "system/BluetoothBackend.hpp"
#include "system/BrightnessBackend.hpp"
#include "system/DndState.hpp"
#include "system/WifiBackend.hpp"
#include "ui/Theme.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"

namespace qypr {

namespace {
constexpr const char* kGearGlyph = "󰒓";  // nf-md-cog
constexpr double kGearWidth = 32.0;
}  // namespace

StatusBar::StatusBar(EventLoop& loop, Invalidator& host, const SystemBackends& backends)
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

    // Backends push; every push fans out to the indicators (they filter by
    // their own backend pointer) and triggers a repaint.
    if (backends.battery) {
        backends.battery->setOnChange([this] { notifyBackendUpdate(); });
    }
    if (backends.brightness) {
        backends.brightness->setOnChange([this] { notifyBackendUpdate(); });
    }
    if (backends.wifi) {
        backends.wifi->setOnChange([this] { notifyBackendUpdate(); });
    }
    if (backends.bluetooth) {
        backends.bluetooth->setOnChange([this] { notifyBackendUpdate(); });
    }
    if (backends.dnd) {
        backends.dnd->addListener([this] { notifyBackendUpdate(); });
    }

    // The bar's own 1s tick: drives poll() (clock text, animations that
    // depend on wall time) — repaint cadence matches the lockscreen clock.
    tickTimer_ = loop_.addTimer(1000, true, [this] {
        const int64_t now = nowMs();
        for (auto& ind : leftIndicators_) ind->poll(now);
        for (auto& ind : centerIndicators_) ind->poll(now);
        for (auto& ind : rightIndicators_) ind->poll(now);
        host_.invalidate();
    });

    // Offscreen measurement context for layout-time text measuring.
    measureSurface_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    measureCr_ = cairo_create(measureSurface_);
}

StatusBar::~StatusBar() {
    if (tickTimer_ >= 0) loop_.removeTimer(tickTimer_);
    if (measureCr_) cairo_destroy(measureCr_);
    if (measureSurface_) cairo_surface_destroy(measureSurface_);
}

void StatusBar::notifyBackendUpdate() {
    for (auto& ind : leftIndicators_) ind->onBackendUpdate();
    for (auto& ind : centerIndicators_) ind->onBackendUpdate();
    for (auto& ind : rightIndicators_) ind->onBackendUpdate();
    host_.invalidate();
}

void StatusBar::layout(int screenW, int screenH) {
    (void)screenH;
    double topMargin = theme::statusbar::topMargin;
    double sideMargin = theme::statusbar::sideMargin;
    double barH = theme::statusbar::height;
    double pad = theme::statusbar::padding;
    double sp = theme::statusbar::iconSpacing;

    bounds = {sideMargin, topMargin, screenW - 2 * sideMargin, barH};

    Painter meas(measureCr_);

    // 1. Layout Left Zone (flows right)
    double lx = bounds.x + pad;
    for (auto& ind : leftIndicators_) {
        if (!ind->visible) {
            ind->bounds = {0, 0, 0, 0};
            continue;
        }
        double w = ind->measureWidth(meas);
        ind->bounds = {lx, bounds.y, w, barH};
        lx += w + sp;
    }

    // 2. Layout rightmost gear button
    qsButtonBounds_ = {bounds.x + bounds.w - pad - kGearWidth, bounds.y, kGearWidth, barH};

    // 3. Layout Right Zone (flows left from gear button)
    double rx = qsButtonBounds_.x - sp;
    for (auto it = rightIndicators_.rbegin(); it != rightIndicators_.rend(); ++it) {
        if (!(*it)->visible) {
            (*it)->bounds = {0, 0, 0, 0};
            continue;
        }
        double w = (*it)->measureWidth(meas);
        rx -= w;
        (*it)->bounds = {rx, bounds.y, w, barH};
        rx -= sp;
    }

    // 4. Layout Center Zone
    double totalCenterW = 0;
    int visibleCenter = 0;
    for (auto& ind : centerIndicators_) {
        if (!ind->visible) continue;
        totalCenterW += ind->measureWidth(meas);
        ++visibleCenter;
    }
    if (visibleCenter > 1) totalCenterW += (visibleCenter - 1) * sp;
    double cx = bounds.x + (bounds.w - totalCenterW) / 2.0;
    for (auto& ind : centerIndicators_) {
        if (!ind->visible) {
            ind->bounds = {0, 0, 0, 0};
            continue;
        }
        double w = ind->measureWidth(meas);
        ind->bounds = {cx, bounds.y, w, barH};
        cx += w + sp;
    }

    // 5. Anchor the Quick Settings panel below the right edge of the bar.
    if (popovers_.active() == &qsPanel_) {
        qsPanel_.anchorX = bounds.x + bounds.w;
        qsPanel_.anchorY = bounds.y + barH + 6.0;
    }
}

void StatusBar::draw(Painter& p, int64_t now) {
    if (!visible) return;

    // No chrome of its own — no strip, no border. Indicators sit directly on
    // the lockscreen background so bar and lockscreen read as one surface.
    auto drawZone = [&](auto& list) {
        for (auto& ind : list) {
            if (!ind->visible) continue;
            ind->hoverAlpha_.animateTo(ind->hovered ? 1.0 : 0.0, theme::anim::fast,
                                       ease::inOutQuad);
            ind->hoverScale_.animateTo(ind->hovered ? 1.05 : 1.0, theme::anim::fast,
                                       ease::inOutQuad);
            ind->draw(p, now);
        }
    };
    drawZone(leftIndicators_);
    drawZone(centerIndicators_);
    drawZone(rightIndicators_);

    // Gear button
    if (qsButtonHovered_) {
        p.fillRoundedRect(qsButtonBounds_, 8.0, theme::color::glassHover);
    }
    TextStyle gearStyle{theme::font::iconFamily, theme::statusbar::iconSize,
                        PANGO_WEIGHT_NORMAL, theme::color::text};
    Size gearSz = p.measureText(kGearGlyph, gearStyle);
    p.drawTextShadowed(qsButtonBounds_.x + (qsButtonBounds_.w - gearSz.w) / 2.0,
                       qsButtonBounds_.y + (qsButtonBounds_.h - gearSz.h) / 2.0, kGearGlyph,
                       gearStyle, HAlign::Left, theme::effects::shadowOpacity,
                       theme::effects::shadowOffset);

    // Popovers
    popovers_.draw(p, now);
}

bool StatusBar::animating(int64_t now) const {
    if (popovers_.animating(now)) return true;
    auto zoneAnimating = [&](const auto& list) {
        for (const auto& ind : list) {
            if (ind->hoverAlpha_.active(now) || ind->hoverScale_.active(now)) return true;
        }
        return false;
    };
    return zoneAnimating(leftIndicators_) || zoneAnimating(centerIndicators_) ||
           zoneAnimating(rightIndicators_);
}

bool StatusBar::handlePointerMotion(double x, double y, int64_t now) {
    (void)now;
    if (popovers_.active() == &qsPanel_ && qsPanel_.activeDragTile_) {
        popovers_.handleDrag(x, y);
        host_.invalidate();
        return true;
    }

    // Hit test gear button
    bool lastGearHover = qsButtonHovered_;
    qsButtonHovered_ = qsButtonBounds_.contains(x, y);
    if (qsButtonHovered_ != lastGearHover) host_.invalidate();

    // Hit test indicators
    auto checkHover = [&](auto& list) {
        for (auto& ind : list) {
            bool prev = ind->hovered;
            ind->hovered = ind->visible && ind->bounds.contains(x, y);
            if (ind->hovered != prev) host_.invalidate();
        }
    };
    checkHover(leftIndicators_);
    checkHover(centerIndicators_);
    checkHover(rightIndicators_);

    return bounds.contains(x, y) || (popovers_.active() && popovers_.active()->contains(x, y));
}

void StatusBar::activateIndicator(StatusIndicator& ind) {
    ind.onActivate();
    if (ind.hasDetailedView()) {
        if (auto view = ind.createDetailedView()) {
            popovers_.open(std::move(view), ind.bounds.x + ind.bounds.w,
                           ind.bounds.y + ind.bounds.h + 6.0);
        }
    } else {
        toggleQuickSettings();
    }
    host_.invalidate();
}

bool StatusBar::handlePointerButton(double x, double y, uint32_t button, bool pressed,
                                    int64_t now) {
    (void)button;
    (void)now;
    if (popovers_.active()) {
        if (popovers_.active()->contains(x, y)) {
            if (pressed) {
                popovers_.handleClick(x, y);
            } else if (popovers_.active() == &qsPanel_) {
                qsPanel_.activeDragTile_ = nullptr;  // release slider drag
            }
            host_.invalidate();
            return true;
        } else if (pressed) {
            // Clicked outside the active popover: dismiss it
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
            if (ind->visible && ind->bounds.contains(x, y)) {
                activateIndicator(*ind);
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
    (void)now;
    qsButtonHovered_ = false;
    auto clear = [&](auto& list) {
        for (auto& ind : list) ind->hovered = false;
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

    // Scroll-to-adjust indicators (volume, brightness)
    auto checkScroll = [&](auto& list) {
        for (auto& ind : list) {
            if (ind->visible && ind->bounds.contains(x, y) && ind->onScroll(dx, dy)) {
                host_.invalidate();
                return true;
            }
        }
        return false;
    };
    return checkScroll(rightIndicators_);
}

bool StatusBar::handleKey(uint32_t keysym) {
    if (popovers_.active()) {
        if (keysym == XKB_KEY_Escape) {
            popovers_.closeActive();
            host_.invalidate();
            return true;
        }
        return popovers_.handleKey(keysym);
    }

    if (!hasFocusedChild()) return false;
    switch (keysym) {
        case XKB_KEY_Escape:
            clearFocus();
            return true;
        case XKB_KEY_Return:
        case XKB_KEY_KP_Enter:
        case XKB_KEY_space: {
            auto findFocused = [&](auto& list) -> StatusIndicator* {
                for (auto& ind : list)
                    if (ind->focused) return ind.get();
                return nullptr;
            };
            StatusIndicator* focused = findFocused(leftIndicators_);
            if (!focused) focused = findFocused(centerIndicators_);
            if (!focused) focused = findFocused(rightIndicators_);
            if (focused) activateIndicator(*focused);
            return true;
        }
        case XKB_KEY_Left:
            return cycleFocus(true);
        case XKB_KEY_Right:
            return cycleFocus(false);
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
    // Collect visible indicators in order
    std::vector<StatusIndicator*> inds;
    auto collect = [&](auto& list) {
        for (auto& ind : list)
            if (ind->visible) inds.push_back(ind.get());
    };
    collect(leftIndicators_);
    collect(centerIndicators_);
    collect(rightIndicators_);

    if (inds.empty()) return false;

    int current = -1;
    for (size_t i = 0; i < inds.size(); ++i) {
        if (inds[i]->focused) {
            current = static_cast<int>(i);
            break;
        }
    }

    if (current == -1) {
        int target = reverse ? static_cast<int>(inds.size()) - 1 : 0;
        inds[target]->focused = true;
        host_.invalidate();
        return true;
    }

    inds[current]->focused = false;
    int next = current + (reverse ? -1 : 1);
    if (next >= 0 && next < static_cast<int>(inds.size())) {
        inds[next]->focused = true;
        host_.invalidate();
        return true;
    }

    // Fell off the edge — focus leaves the bar
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
    auto anyFocused = [](const auto& list) {
        for (const auto& ind : list)
            if (ind->focused) return true;
        return false;
    };
    return anyFocused(leftIndicators_) || anyFocused(centerIndicators_) ||
           anyFocused(rightIndicators_);
}

}  // namespace qypr
