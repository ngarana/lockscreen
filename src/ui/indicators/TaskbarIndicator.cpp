// TaskbarIndicator.cpp - Icons-only window list implementation.
#include "ui/indicators/TaskbarIndicator.hpp"

#include <algorithm>
#include <cctype>

#include "render/Painter.hpp"
#include "system/ToplevelBackend.hpp"
#include "ui/IconResolver.hpp"
#include "ui/Theme.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"

namespace qypr {

namespace {
constexpr double kBtnW = 32.0;    // per-window button width
constexpr double kIconPx = 20.0;  // icon glyph/surface size
constexpr double kGap = 4.0;      // between buttons
constexpr double kSidePad = 4.0;  // hover-zone padding each side
constexpr double kUnderlineH = 2.0;

// A themed initial-letter tile when the app id resolves to no icon (D5's
// fallback). The hue is derived from the app id so the same app is always the
// same colour, and different apps are distinguishable.
const Color& fallbackColor(const std::string& appId) {
    static const Color palette[] = {
        Color::fromHex("#89b4fa"), Color::fromHex("#a6e3a1"), Color::fromHex("#f9e2af"),
        Color::fromHex("#f38ba8"), Color::fromHex("#cba6f7"), Color::fromHex("#94e2d5"),
        Color::fromHex("#fab387"), Color::fromHex("#74c7ec"),
    };
    uint32_t h = 2166136261u;
    for (char c : appId) h = (h ^ static_cast<unsigned char>(c)) * 16777619u;
    return palette[h % (sizeof(palette) / sizeof(palette[0]))];
}

// The letter for the fallback tile: last dotted segment's first char, upper.
// "org.kde.dolphin" → 'D', "firefox" → 'F', "" → '?'.
std::string initialFor(const std::string& appId) {
    if (appId.empty()) return "?";
    size_t start = appId.find_last_of('.');
    start = (start == std::string::npos) ? 0 : start + 1;
    if (start >= appId.size()) start = 0;
    char c = appId[start];
    return std::string(1, static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
}

// Try the app id, then its lowercase form, as a freedesktop icon name.
cairo_surface_t* resolveIcon(const std::string& appId) {
    if (appId.empty()) return nullptr;
    if (auto* s = IconResolver::instance().get(appId)) return s;
    std::string lower = appId;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (lower != appId) {
        if (auto* s = IconResolver::instance().get(lower)) return s;
    }
    return nullptr;
}
}  // namespace

TaskbarIndicator::TaskbarIndicator(const SystemBackends& backends)
    : StatusIndicator("taskbar", Zone::Left, 100), backend_(backends.toplevel) {
    visible = false;  // hidden until at least one window exists
}

std::string TaskbarIndicator::tooltip() const {
    const size_t n = snap_.windows.size();
    if (n == 0) return "No open windows";
    return std::to_string(n) + (n == 1 ? " open window" : " open windows");
}

double TaskbarIndicator::measureWidth(Painter&) {
    const size_t n = snap_.windows.size();
    if (n == 0) return 0;
    return n * kBtnW + (n - 1) * kGap + 2 * kSidePad;
}

int TaskbarIndicator::hitTest(double x) const {
    const double localX = x - (bounds.x + kSidePad);
    if (localX < 0) return -1;
    const int idx = static_cast<int>(localX / (kBtnW + kGap));
    if (idx < 0 || idx >= static_cast<int>(snap_.windows.size())) return -1;
    // Reject the gap between buttons so a click there is a no-op, not a misfire.
    if (localX - idx * (kBtnW + kGap) > kBtnW) return -1;
    return idx;
}

void TaskbarIndicator::draw(Painter& p, int64_t now) {
    (void)now;
    if (!visible || snap_.windows.empty()) return;

    double x = bounds.x + kSidePad;
    for (const auto& w : snap_.windows) {
        const Rect btn{x, bounds.y + 2.0, kBtnW, bounds.h - 4.0};

        // Focused window: a filled pill + an accent underline. Others are bare.
        if (w.active) {
            p.fillRoundedRect(btn, 8.0, theme::color::glassHover);
            const Rect ul{btn.x + 6.0, btn.y + btn.h - kUnderlineH, btn.w - 12.0, kUnderlineH};
            p.fillRoundedRect(ul, kUnderlineH / 2.0, theme::color::primary);
        }

        const double iconX = x + (kBtnW - kIconPx) / 2.0;
        const double iconY = bounds.y + (bounds.h - kIconPx) / 2.0;

        cairo_surface_t* s = resolveIcon(w.appId);
        // Minimized windows are dimmed so the focused/visible set reads clearly.
        const double alpha = w.minimized ? 0.45 : 1.0;

        if (s) {
            if (alpha < 1.0) p.pushGroup();
            p.drawSurface(s, {iconX, iconY, kIconPx, kIconPx});
            if (alpha < 1.0) p.popGroupWithAlpha(alpha);
        } else {
            // Initial-letter tile.
            const Rect tile{iconX, iconY, kIconPx, kIconPx};
            p.fillRoundedRect(tile, 5.0, fallbackColor(w.appId).withAlpha(alpha));
            TextStyle st{theme::font::family, 12.0, PANGO_WEIGHT_BOLD,
                         Color::fromHex("#1e1e2e").withAlpha(alpha)};
            const std::string ch = initialFor(w.appId);
            const Size cs = p.measureText(ch, st);
            p.drawText(iconX + (kIconPx - cs.w) / 2.0, iconY + (kIconPx - cs.h) / 2.0, ch, st);
        }
        x += kBtnW + kGap;
    }
}

void TaskbarIndicator::onBackendUpdate() {
    if (!backend_) {
        visible = false;
        return;
    }
    snap_ = backend_->snapshot();
    visible = snap_.available && !snap_.windows.empty();
}

bool TaskbarIndicator::onClick(double x, double y) {
    (void)y;
    if (!backend_ || !visible) return false;
    const int idx = hitTest(x);
    if (idx < 0) return false;
    const ToplevelWindow& w = snap_.windows[idx];
    // Click the focused window to minimize it; click any other (or a minimized
    // one) to focus/restore it.
    if (w.active && !w.minimized) {
        backend_->toggleMinimize(w.id);
    } else {
        backend_->activate(w.id);
    }
    return true;
}

bool TaskbarIndicator::onMiddleClick(double x, double y) {
    (void)y;
    if (!backend_ || !visible) return false;
    const int idx = hitTest(x);
    if (idx < 0) return false;
    backend_->close(snap_.windows[idx].id);
    return true;
}

REGISTER_INDICATOR("taskbar", Zone::Left, 100, TaskbarIndicator)

}  // namespace qypr
