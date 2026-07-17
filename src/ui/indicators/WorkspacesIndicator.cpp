// WorkspacesIndicator.cpp - Workspaces bar widget implementation.
#include "ui/indicators/WorkspacesIndicator.hpp"

#include "render/Painter.hpp"
#include "system/WorkspaceBackend.hpp"
#include "ui/Theme.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"

namespace qypr {

namespace {
constexpr double kPillPadX = 9.0;  // horizontal padding inside each pill
constexpr double kPillGap = 5.0;   // between pills
constexpr double kSidePad = 6.0;   // indicator edge padding
constexpr double kPillH = 24.0;    // pill height (within the 36px bar)
constexpr double kFont = 13.0;
}  // namespace

WorkspacesIndicator::WorkspacesIndicator(const SystemBackends& backends)
    : StatusIndicator("workspaces", Zone::Left, -100), backend_(backends.workspace) {
    visible = false;  // hidden until the backend reports workspaces
}

std::string WorkspacesIndicator::tooltip() const {
    for (const auto& w : snap_.workspaces) {
        if (w.active) return "Workspace " + w.name;
    }
    return "Workspaces";
}

double WorkspacesIndicator::pillWidth(Painter& p, const std::string& name) const {
    TextStyle st{theme::font::family, kFont, PANGO_WEIGHT_MEDIUM, theme::color::text};
    return p.measureText(name, st).w + 2 * kPillPadX;
}

double WorkspacesIndicator::measureWidth(Painter& p) {
    if (snap_.workspaces.empty()) return 0;
    double w = 2 * kSidePad;
    for (size_t i = 0; i < snap_.workspaces.size(); ++i) {
        if (i) w += kPillGap;
        w += pillWidth(p, snap_.workspaces[i].name);
    }
    return w;
}

void WorkspacesIndicator::draw(Painter& p, int64_t now) {
    (void)now;
    if (!visible) return;
    hits_.clear();

    double x = bounds.x + kSidePad;
    double pillY = bounds.y + (bounds.h - kPillH) / 2.0;
    for (const auto& w : snap_.workspaces) {
        double pw = pillWidth(p, w.name);

        Color txtColor;
        bool onFill = false;
        if (w.active) {
            p.fillRoundedRect({x, pillY, pw, kPillH}, kPillH / 2.0, theme::color::primary);
            txtColor = theme::color::background;
            onFill = true;
        } else if (w.urgent) {
            p.fillRoundedRect({x, pillY, pw, kPillH}, kPillH / 2.0,
                              theme::color::warning.withAlpha(0.28));
            txtColor = theme::color::warning;
        } else {
            txtColor = theme::color::textSubtle;
        }

        TextStyle st{theme::font::family, kFont, PANGO_WEIGHT_MEDIUM, txtColor};
        Size ts = p.measureText(w.name, st);
        double tx = x + (pw - ts.w) / 2.0;
        double ty = bounds.y + (bounds.h - ts.h) / 2.0;
        if (onFill) {
            p.drawText(tx, ty, w.name, st, HAlign::Left);
        } else {
            // Chromeless: shadow keeps unfilled pill text readable over video.
            p.drawTextShadowed(tx, ty, w.name, st, HAlign::Left, theme::effects::shadowOpacity,
                               theme::effects::shadowOffset);
        }

        hits_.push_back({x, x + pw, w.name});
        x += pw + kPillGap;
    }
}

void WorkspacesIndicator::onBackendUpdate() {
    if (!backend_) {
        visible = false;
        return;
    }
    snap_ = backend_->snapshot();
    visible = snap_.available && !snap_.workspaces.empty();
}

bool WorkspacesIndicator::onClick(double x, double y) {
    (void)y;
    if (!backend_) return false;
    for (const auto& h : hits_) {
        if (x >= h.x0 && x <= h.x1) {
            backend_->activate(h.name);
            return true;
        }
    }
    return false;
}

REGISTER_INDICATOR("workspaces", Zone::Left, -100, WorkspacesIndicator)

}  // namespace qypr
