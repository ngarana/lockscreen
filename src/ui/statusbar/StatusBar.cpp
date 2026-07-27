// StatusBar.cpp - Container for zones, indicators, popovers, and layout implementation
#include "ui/statusbar/StatusBar.hpp"

#include <xkbcommon/xkbcommon-keysyms.h>

#include "core/EventLoop.hpp"
#include "core/Interfaces.hpp"
#include "core/Types.hpp"
#include "render/Painter.hpp"
#include "mpris/MprisController.hpp"
#include "system/BatteryBackend.hpp"
#include "system/BluetoothBackend.hpp"
#include "system/BrightnessBackend.hpp"
#include "system/DndState.hpp"
#include "system/IdleInhibitor.hpp"
#include "system/KeyboardLayout.hpp"
#include "system/SNIBackend.hpp"
#include "system/ToplevelBackend.hpp"
#include "system/VolumeBackend.hpp"
#include "system/WifiBackend.hpp"
#include "system/WorkspaceBackend.hpp"
#include "ui/Theme.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"

namespace qypr {

namespace {
// Translucent menu-bar backdrop. The tint colour and its alpha come from the
// theme at draw time (theme::statusbar::barTint / barTintAlpha, both derived
// from the active palette); a config-supplied `bar.backdrop` value overrides the
// alpha (used by qypr-bar's standalone config). The bar itself is a single
// translucent slab; each indicator is a separate interactive item (no
// right-group chip).
}  // namespace

StatusBar::StatusBar(EventLoop& loop, Invalidator& host, const SystemBackends& backends,
                     const IndicatorRegistry::ModuleSelection* sel)
    : loop_(loop), host_(host) {
    // Create indicators: the config-selected set when the host supplied one,
    // otherwise every registered indicator (the lock screen's behaviour).
    auto all = IndicatorRegistry::instance().createAll(backends, sel);
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

    // Build internal QS panel tiles (header, power, wifi combo, volume, media).
    // The power callback opens the PowerMenuIndicator's popover (if present).
    qsPanel_.buildTiles(loop_, backends, [this]() {
        for (auto& ind : rightIndicators_) {
            if (ind->id() == "power" && ind->hasDetailedView()) {
                activateIndicator(*ind);
                return;
            }
        }
    });

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
    if (backends.volume) {
        backends.volume->setOnChange([this] { notifyBackendUpdate(); });
    }
    if (backends.sni) {
        backends.sni->setOnChange([this] { notifyBackendUpdate(); });
    }
    if (backends.mpris) {
        backends.mpris->setOnChange([this] { notifyBackendUpdate(); });
    }
    if (backends.idleInhibitor) {
        backends.idleInhibitor->setOnChange([this] { notifyBackendUpdate(); });
    }
    if (backends.keyboardLayout) {
        backends.keyboardLayout->setOnChange([this] { notifyBackendUpdate(); });
    }
    // Session-sensitive WM widgets (only ever started by the unlocked bar).
    if (backends.workspace) {
        backends.workspace->setOnChange([this] { notifyBackendUpdate(); });
    }
    if (backends.toplevel) {
        backends.toplevel->setOnChange([this] { notifyBackendUpdate(); });
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

void StatusBar::setSessionContentVisible(bool v) {
    if (v == sessionContentVisible_) return;
    sessionContentVisible_ = v;
    host_.invalidate();
}

bool StatusBar::hasOpenOverlay() const {
    return popovers_.active() != nullptr || popovers_.isTransitioning();
}

int StatusBar::overlayHeight() const {
    // Height (from the anchored edge) needed to contain the strip and the open
    // popover. The popover sits a 6px gap past the strip; add a little breathing
    // room past its bottom. Uses the tallest *drawing* popover so the surface
    // stays big enough through the close fade, then returns 0 (idle strip) once
    // nothing is drawn. Symmetric for top/bottom bars: the popover always grows
    // away from the anchored edge, so the extent from that edge is the same.
    const double popH = popovers_.maxContentHeight();
    if (popH <= 0.0) return 0;
    return static_cast<int>(geom_.edgeMargin + geom_.height + 6.0 + popH + 8.0 + 0.5);
}

void StatusBar::setBackdrop(bool enabled, double alpha) {
    backdrop_ = enabled;
    backdropAlpha_ = alpha;
    host_.invalidate();
}

void StatusBar::setGeometry(const BarGeometry& g) {
    geom_ = g;
    host_.invalidate();  // layout() recomputes bounds on the next frame
}

void StatusBar::notifyBackendUpdate() {
    for (auto& ind : leftIndicators_) ind->onBackendUpdate();
    for (auto& ind : centerIndicators_) ind->onBackendUpdate();
    for (auto& ind : rightIndicators_) ind->onBackendUpdate();
    host_.invalidate();
}

void StatusBar::layout(int screenW, int screenH) {
    double sideMargin = geom_.sideMargin;
    double barH = geom_.height;
    double pad = theme::statusbar::padding;
    double sp = theme::statusbar::iconSpacing;

    // The strip sits `edgeMargin` from the anchored edge. Measuring the bottom
    // edge from screenH (rather than a fixed y) is what makes `position=bottom`
    // work in both host states: on the idle 66px layer-shell strip screenH is
    // the strip itself, and when the surface grows for an overlay screenH is the
    // whole output — the bar stays pinned to the same physical edge either way.
    const double y = geom_.bottom ? screenH - geom_.edgeMargin - barH : geom_.edgeMargin;
    bounds = {sideMargin, y, screenW - 2 * sideMargin, barH};

    Painter meas(measureCr_);

    // 1. Layout Left Zone (flows right)
    double lx = bounds.x + pad;
    for (auto& ind : leftIndicators_) {
        if (!isShown(*ind)) {
            ind->bounds = {0, 0, 0, 0};
            continue;
        }
        double w = ind->measureWidth(meas);
        ind->bounds = {lx, bounds.y, w, barH};
        lx += w + sp;
    }

    // 2. Layout Right Zone (flows left from the bar's right edge).
    //    Each indicator is an independent interactive item.
    //    Clicking an indicator opens its own popover or toggles its state.
    double rx = bounds.x + bounds.w - pad;
    for (auto it = rightIndicators_.rbegin(); it != rightIndicators_.rend(); ++it) {
        if (!isShown(**it)) {
            (*it)->bounds = {0, 0, 0, 0};
            continue;
        }
        double w = (*it)->measureWidth(meas);
        rx -= w;
        (*it)->bounds = {rx, bounds.y, w, barH};
        rx -= sp;
    }

    // Compute right-group bounds for the backdrop chip (subtle, not a solid filled tile)
    rightGroupBounds_ = {0, 0, 0, 0};
    double rightMinX = bounds.x + bounds.w;
    double rightMaxX = 0;
    for (const auto& ind : rightIndicators_) {
        if (!isShown(*ind)) continue;
        if (ind->bounds.x < rightMinX) rightMinX = ind->bounds.x;
        if (ind->bounds.x + ind->bounds.w > rightMaxX) rightMaxX = ind->bounds.x + ind->bounds.w;
    }
    if (rightMinX < rightMaxX) {
        rightGroupBounds_ = {rightMinX, bounds.y, rightMaxX - rightMinX, barH};
    }

    // 3. Layout Center Zone
    double totalCenterW = 0;
    int visibleCenter = 0;
    for (auto& ind : centerIndicators_) {
        if (!isShown(*ind)) continue;
        totalCenterW += ind->measureWidth(meas);
        ++visibleCenter;
    }
    if (visibleCenter > 1) totalCenterW += (visibleCenter - 1) * sp;
    double cx = bounds.x + (bounds.w - totalCenterW) / 2.0;
    for (auto& ind : centerIndicators_) {
        if (!isShown(*ind)) {
            ind->bounds = {0, 0, 0, 0};
            continue;
        }
        double w = ind->measureWidth(meas);
        ind->bounds = {cx, bounds.y, w, barH};
        cx += w + sp;
    }

    // 4. Anchor active popovers
    //    right edge of the bar, opening away from the anchored screen edge.
    if (popovers_.active()) {
        DetailedPopover* p = popovers_.active();
        if (p == &qsPanel_ || p->contentWidth() >= 300.0) {
            p->anchorX = bounds.x + bounds.w;
        }
        anchorPopoverY(*p);
    }
}

void StatusBar::anchorPopoverY(DetailedPopover& pop) const {
    // growUp makes getBounds() extend upward from the anchor instead of down.
    pop.growUp = geom_.bottom;
    pop.anchorY = geom_.bottom ? bounds.y - 6.0 : bounds.y + bounds.h + 6.0;
}

void StatusBar::draw(Painter& p, int64_t now) {
    if (!visible) return;

    // Translucent menu-bar backdrop: a single frosted-glass slab spans the
    // entire bar surface, with a subtle hairline bottom (or top) border.
    // The lock screen stays chromeless against its dark background.
    if (backdrop_ && bounds.w > 0) {
        const double tintAlpha = backdropAlpha_ >= 0.0
                                     ? backdropAlpha_
                                     : theme::statusbar::barTintAlpha;
        p.fillRoundedRect(bounds, theme::statusbar::cornerRadius,
                          theme::statusbar::barTint.withAlpha(tintAlpha));
        // Hairline separator along the anchored edge
        if (theme::statusbar::barBorderEnabled && theme::statusbar::barBorderAlpha > 0.0) {
            const double borderY = geom_.bottom ? bounds.y : bounds.y + bounds.h;
            p.fillRect({bounds.x, borderY - 0.5, bounds.w, 1.0},
                        theme::statusbar::barBorder.withAlpha(theme::statusbar::barBorderAlpha));
        }
    }

    auto drawZone = [&](auto& list) {
        for (auto& ind : list) {
            if (!isShown(*ind)) continue;
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

    // Popovers
    popovers_.draw(p, now);

    // Hover tooltip — last so it floats over the bar but below nothing else.
    drawTooltip(p, now);
}

bool StatusBar::animating(int64_t now) const {
    if (popovers_.animating(now)) return true;
    if (tooltipAlpha_.active(now)) return true;
    auto zoneAnimating = [&](const auto& list) {
        for (const auto& ind : list) {
            if (ind->animating(now)) return true;
        }
        return false;
    };
    return zoneAnimating(leftIndicators_) || zoneAnimating(centerIndicators_) ||
           zoneAnimating(rightIndicators_);
}

bool StatusBar::handlePointerMotion(double x, double y, int64_t now) {
    if (popovers_.active() == &qsPanel_ && qsPanel_.activeDragTile_) {
        popovers_.handleDrag(x, y);
        host_.invalidate();
        return true;
    }
    // An open popover hides the hover-tooltip (the popover's own title serves).
    if (popovers_.active()) tooltipTarget_ = nullptr;

    // Hit test indicators. The hovered indicator also drives the hover-tooltip
    // (Phase 6 polish): we record the one pointer is over, and the timestamp
    // at which it *first* became so; the draw pass reveals its tooltip once
    // the dwell hits kTooltipDelayMs.
    StatusIndicator* newTooltipTarget = nullptr;
    auto checkHover = [&](auto& list) {
        for (auto& ind : list) {
            bool prev = ind->hovered;
            ind->hovered = isShown(*ind) && ind->bounds.contains(x, y);
            if (ind->hovered) newTooltipTarget = ind.get();
            if (ind->hovered != prev) host_.invalidate();
        }
    };
    checkHover(leftIndicators_);
    checkHover(centerIndicators_);
    checkHover(rightIndicators_);

    if (newTooltipTarget != tooltipTarget_) {
        tooltipTarget_ = newTooltipTarget;
        tooltipHoverStartMs_ = now;
        tooltipAlpha_.set(0.0);
        host_.invalidate();
    } else if (tooltipTarget_) {
        host_.invalidate();  // keep ticking so the fade can run
    }

    return bounds.contains(x, y) || (popovers_.active() && popovers_.active()->contains(x, y));
}

void StatusBar::activateIndicator(StatusIndicator& ind) {
    ind.onActivate();
    if (ind.id() == "power") {
        toggleQuickSettings();
        host_.invalidate();
        return;
    }
    if (ind.hasDetailedView()) {
        if (auto view = ind.createDetailedView()) {
            DetailedPopover* p = view.get();
            const double anchorX = ind.zone() == Zone::Left
                                       ? ind.bounds.x + p->contentWidth()
                                       : (ind.zone() == Zone::Right ? bounds.x + bounds.w
                                                                     : ind.bounds.x + ind.bounds.w);
            popovers_.open(std::move(view), anchorX, 0);
            anchorPopoverY(*p);
        }
    }
    host_.invalidate();
}

bool StatusBar::handlePointerButton(double x, double y, uint32_t button, bool pressed,
                                    int64_t now) {
    (void)now;
    // Linux evdev button codes as delivered by wl_pointer.
    constexpr uint32_t kBtnLeft = 0x110, kBtnRight = 0x111, kBtnMiddle = 0x112;
    // A button press always cancels the hover-tooltip: the click either
    // activates an indicator (and opens its popover) or dismisses the open
    // popover, and the lingering label would otherwise sit under the new
    // surface. It re-pops after kTooltipDelayMs of fresh dwell if the cursor
    // stays put.
    if (pressed) {
        tooltipTarget_ = nullptr;
        tooltipAlpha_.set(0.0);
    }
    if (popovers_.active()) {
        if (popovers_.active()->contains(x, y)) {
            if (pressed) {
                popovers_.handleClick(x, y);
                // A menu item may ask to close its popover after firing.
                if (popovers_.active() && popovers_.active()->consumeCloseRequest()) {
                    popovers_.closeActive();
                }
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

    // Clicking anywhere on the right-group chip (or a right-zone indicator that
    // does not have its own popover) opens Quick Settings.
    // Right-zone indicators with detailed views (WiFi AP picker, battery popover,
    // etc.) open their own popover first; a second click on the same indicator or
    // a click on the open space inside the right chip opens QS.
    if (button == kBtnLeft) {
        // Each indicator handles its own click. Right-zone indicators with
        // detailed views open their own popover; toggle indicators fire directly.
        // No automatic fallback to Quick Settings.
        for (const auto& ind : rightIndicators_) {
            if (!isShown(*ind) || !ind->bounds.contains(x, y)) continue;
            if (ind->hasDetailedView()) {
                activateIndicator(*ind);
                host_.invalidate();
                return true;
            }
        }
        for (const auto& ind : rightIndicators_) {
            if (!isShown(*ind) || !ind->bounds.contains(x, y)) continue;
            if (ind->onClick(x, y)) {
                host_.invalidate();
                return true;
            }
            return true;
        }
    }

    // Indicator click. Right/middle go to their handlers and never fall through
    // to activate; a left click gets onClick() first refusal (e.g. the tray host
    // maps it to a sub-icon), else the default activate (tile/popover) runs.
    auto checkClick = [&](auto& list) {
        for (auto& ind : list) {
            if (!isShown(*ind) || !ind->bounds.contains(x, y)) continue;
            if (button == kBtnRight) {
                ind->onSecondaryClick(x, y);
            } else if (button == kBtnMiddle) {
                ind->onMiddleClick(x, y);
            } else if (!ind->onClick(x, y)) {
                activateIndicator(*ind);
            }
            host_.invalidate();
            return true;
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
    auto clear = [&](auto& list) {
        for (auto& ind : list) ind->hovered = false;
    };
    clear(leftIndicators_);
    clear(centerIndicators_);
    clear(rightIndicators_);
    tooltipTarget_ = nullptr;
    tooltipAlpha_.set(0.0);
    host_.invalidate();
}

bool StatusBar::handleScroll(double x, double y, double dx, double dy) {
    if (popovers_.active() && popovers_.active()->contains(x, y)) {
        return popovers_.handleScroll(dx, dy);
    }

    // Scroll-to-adjust indicators (volume, brightness)
    auto checkScroll = [&](auto& list) {
        for (auto& ind : list) {
            if (isShown(*ind) && ind->bounds.contains(x, y) && ind->onScroll(dx, dy, x, y)) {
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
        const bool handled = popovers_.handleKey(keysym);
        // A keyboard action may fire an item (launcher Enter) and ask to close.
        if (popovers_.active() && popovers_.active()->consumeCloseRequest()) {
            popovers_.closeActive();
        }
        host_.invalidate();
        return handled;
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

bool StatusBar::handleTextInput(const std::string& utf8) {
    return popovers_.handleText(utf8);
}

bool StatusBar::wantsKeyboard() const {
    return popovers_.activeWantsKeyboard();
}

void StatusBar::toggleQuickSettings() {
    if (popovers_.active() == &qsPanel_) {
        popovers_.closeActive();
    } else {
        popovers_.openBorrowed(&qsPanel_, bounds.x + bounds.w, 0);
        anchorPopoverY(qsPanel_);
    }
}

bool StatusBar::cycleFocus(bool reverse) {
    // Collect visible indicators in order
    std::vector<StatusIndicator*> inds;
    auto collect = [&](auto& list) {
        for (auto& ind : list)
            if (isShown(*ind)) inds.push_back(ind.get());
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

// indicator for kTooltipDelayMs the bar fades in a small glass-card label
// completing the spec's "per-indicator tooltips" entry. It opens *away from
// the anchored edge* (down on a top bar, up on a bottom bar) — the same direction
// every other popover opens — and is anchored under the indicator's centre.
void StatusBar::drawTooltip(Painter& p, int64_t now) const {
    if (!tooltipTarget_) {
        if (tooltipAlpha_.target() > 0.01) {
            const_cast<StatusBar*>(this)->tooltipAlpha_.animateTo(0.0, theme::anim::fast,
                                                                  ease::inOutQuad);
        }
        return;
    }
    const std::string text = tooltipTarget_->tooltip();
    if (text.empty()) return;

    const int64_t dwell = now - tooltipHoverStartMs_;
    if (dwell >= kTooltipDelayMs && tooltipAlpha_.target() < 0.99) {
        const_cast<StatusBar*>(this)->tooltipAlpha_.animateTo(1.0, theme::anim::fast,
                                                               ease::inOutQuad);
    }

    const double alpha = tooltipAlpha_.value(now);
    if (alpha < 0.01) return;

    // Geometry: measure once, place below (or above on a bottom bar) the
    // indicator. We mirror the popover screen-edge behaviour so the label
    // never gets cropped at the anchored edge.
    constexpr double kPadX = 10.0;
    constexpr double kPadY = 6.0;
    constexpr double kRadius = 8.0;
    constexpr double kGap = 8.0;  // gap from the indicator bounds

    TextStyle style{theme::font::family, 12.0, PANGO_WEIGHT_NORMAL, theme::color::text};
    Size ts = p.measureText(text, style);
    double w = ts.w + kPadX * 2.0;
    double h = ts.h + kPadY * 2.0;

    double cx = tooltipTarget_->bounds.cx();
    double x = cx - w / 2.0;
    // Open away from the anchored edge.
    double y = geom_.bottom ? tooltipTarget_->bounds.y - kGap - h
                            : tooltipTarget_->bounds.y + tooltipTarget_->bounds.h + kGap;

    // Clamp horizontal to the bar's slab so a near-edge indicator never clips.
    x = std::clamp(x, bounds.x + 2.0, bounds.x + bounds.w - w - 2.0);

    Rect r{x, y, w, h};
    p.pushGroup();
    p.fillRoundedRect(r, kRadius, theme::color::glass);
    p.strokeRoundedRect(r, kRadius, theme::color::glassBorder, 1.0);
    p.drawText(r.x + kPadX, r.y + kPadY, text, style);
    p.popGroupWithAlpha(alpha);
}

}  // namespace qypr
