// SNITrayHost.cpp - Status bar system-tray applet implementation.
#include "ui/indicators/SNITrayHost.hpp"

#include "render/Painter.hpp"
#include "system/DbusMenuBackend.hpp"
#include "system/SNIBackend.hpp"
#include "ui/IconResolver.hpp"
#include "ui/Theme.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"
#include "ui/statusbar/MenuPopover.hpp"

namespace qypr {

namespace {
constexpr double kIconPx = 18.0;   // per-item icon size
constexpr double kGap = 6.0;       // between adjacent icons
constexpr double kSidePad = 8.0;   // hover-zone padding each side
constexpr const char* kFallbackGlyph = "󰀻";  // nf-md-application (no icon/pixmap)
}  // namespace

SNITrayHost::SNITrayHost(const SystemBackends& backends)
    : StatusIndicator("sni", Zone::Right, 600),
      backend_(backends.sni),
      dbusMenu_(backends.dbusMenu) {
    visible = false;  // hidden until the host mirrors at least one item
}

std::string SNITrayHost::tooltip() const {
    if (!backend_) return "System tray";
    const auto& items = backend_->items();
    if (items.empty()) return "System tray";
    if (items.size() == 1 && !items[0].title.empty()) return items[0].title;
    return std::to_string(items.size()) + " tray items";
}

double SNITrayHost::measureWidth(Painter&) {
    if (!backend_) return 0;
    size_t n = backend_->items().size();
    if (n == 0) return 0;
    return n * kIconPx + (n - 1) * kGap + 2 * kSidePad;
}

void SNITrayHost::draw(Painter& p, int64_t now) {
    if (!visible || !backend_) return;
    const auto& items = backend_->items();
    if (items.empty()) return;

    // Hover pill + focus ring (mirrors the base StatusIndicator chrome).
    double alpha = hoverAlpha_.value(now);
    if (alpha > 0.01) {
        Color bg = theme::color::glassHover.withAlpha(alpha * theme::color::glassHover.a);
        Rect hoverRect = bounds;
        hoverRect.y += 2.0;
        hoverRect.h -= 4.0;
        p.fillRoundedRect(hoverRect, 8.0, bg);
    }
    if (focused) {
        Rect focusRect = bounds;
        focusRect.y += 1.0;
        focusRect.h -= 2.0;
        p.strokeRoundedRect(focusRect, 8.0, theme::color::primary, 1.5);
    }

    double x = bounds.x + kSidePad;
    double y = bounds.y + (bounds.h - kIconPx) / 2.0;
    for (const auto& it : items) {
        // Prefer the themed IconName; fall back to the app-supplied pixmap.
        cairo_surface_t* s = nullptr;
        if (!it.iconName.empty()) s = IconResolver::instance().get(it.iconName);
        if (!s) s = it.pixmap;

        if (s) {
            p.drawSurface(s, {x, y, kIconPx, kIconPx});
        } else {
            TextStyle st{theme::font::iconFamily, kIconPx, PANGO_WEIGHT_NORMAL,
                         theme::color::text};
            p.drawTextShadowed(x, y, kFallbackGlyph, st, HAlign::Left,
                               theme::effects::shadowOpacity, theme::effects::shadowOffset);
        }
        x += kIconPx + kGap;
    }
}

void SNITrayHost::onBackendUpdate() {
    visible = backend_ && !backend_->items().empty();
}

int SNITrayHost::iconIndexAt(double x) const {
    if (!backend_ || !visible) return -1;
    const size_t n = backend_->items().size();
    if (n == 0) return -1;
    const double localX = x - (bounds.x + kSidePad);
    if (localX < 0) return -1;
    const int idx = static_cast<int>(localX / (kIconPx + kGap));
    return (idx >= 0 && idx < static_cast<int>(n)) ? idx : -1;
}

bool SNITrayHost::onClick(double x, double y) {
    const int idx = iconIndexAt(x);
    if (idx < 0) return false;
    backend_->activate(static_cast<size_t>(idx), static_cast<int>(x), static_cast<int>(y));
    return true;
}

bool SNITrayHost::onSecondaryClick(double x, double y) {
    (void)y;
    // Right-click opens the item's dbusmenu — but only on the unlocked bar
    // (dbusMenu_ is null on the lock screen, by design).
    if (!dbusMenu_) return false;
    const int idx = iconIndexAt(x);
    if (idx < 0) return false;
    if (backend_->items()[static_cast<size_t>(idx)].menuPath.empty()) return false;
    pendingMenu_ = idx;  // consumed by createDetailedView()
    return true;
}

bool SNITrayHost::onMiddleClick(double x, double y) {
    const int idx = iconIndexAt(x);
    if (idx < 0) return false;
    backend_->secondaryActivate(static_cast<size_t>(idx), static_cast<int>(x),
                                static_cast<int>(y));
    return true;
}

std::unique_ptr<DetailedPopover> SNITrayHost::createDetailedView() {
    const int idx = pendingMenu_;
    pendingMenu_ = -1;
    if (!dbusMenu_ || idx < 0 || idx >= static_cast<int>(backend_->items().size())) return nullptr;
    const SNIItem& it = backend_->items()[static_cast<size_t>(idx)];
    if (it.menuPath.empty()) return nullptr;
    auto root = dbusMenu_->fetch(it.service, it.menuPath, 0);
    return std::make_unique<MenuPopover>(dbusMenu_, it.service, it.menuPath, std::move(root),
                                         it.title);
}

REGISTER_INDICATOR("sni", Zone::Right, 600, SNITrayHost)

}  // namespace qypr
