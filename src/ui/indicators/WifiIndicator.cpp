// WifiIndicator.cpp - Status bar WiFi indicator implementation.
#include "ui/indicators/WifiIndicator.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include "render/Painter.hpp"
#include "ui/Theme.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"
#include "ui/statusbar/QSTile.hpp"

namespace qypr {

namespace {
const char* wifiIcon(const WifiSnapshot& s) {
    if (!s.enabled) return "󰤮";
    if (!s.connected) return "󰤭";
    if (s.strength >= 75) return "󰤨";
    if (s.strength >= 50) return "󰤥";
    if (s.strength >= 25) return "󰤢";
    return "󰤯";
}

// freedesktop symbolic names — signal-bar icons with -secure variants.
const char* wifiThemedIcon(const WifiSnapshot& s) {
    if (!s.enabled) return "network-wireless-disabled-symbolic";
    if (!s.connected) return "network-wireless-disconnected-symbolic";
    if (s.strength >= 75) return "network-wireless-signal-excellent-symbolic";
    if (s.strength >= 50) return "network-wireless-signal-good-symbolic";
    if (s.strength >= 25) return "network-wireless-signal-ok-symbolic";
    return "network-wireless-signal-weak-symbolic";
}

// Signal glyph by strength for a picker row.
const char* apGlyph(int strength) {
    if (strength >= 75) return "󰤨";
    if (strength >= 50) return "󰤥";
    if (strength >= 25) return "󰤢";
    return "󰤟";
}

constexpr double kWW = 300.0;
constexpr double kWPad = 12.0;
constexpr double kWHdr = 24.0;
constexpr double kWRow = 34.0;
constexpr size_t kWMax = 8;  // one page; scroll reaches the rest
constexpr const char* kLock = "󰌾";
constexpr const char* kCheck = "󰄬";

// Network picker: nearby APs fetched once on open (scanNetworks reads the bus,
// so it is not called per frame). Joining is limited to saved networks — an
// unsaved secured AP needs a NetworkManager secret agent (out of scope).
class WifiPopover : public DetailedPopover {
public:
    WifiPopover(WifiBackend* backend, bool enabled)
        : backend_(backend), enabled_(enabled) {
        if (backend_ && enabled_) {
            networks_ = backend_->scanNetworks();
            backend_->requestScan();  // freshen for the next open
        }
    }

    double contentWidth() const override { return kWW; }
    double contentHeight() const override {
        double h = kWPad * 2 + kWHdr;
        if (!enabled_) return h + kWRow;
        const size_t n = std::min(networks_.size(), kWMax);
        h += (networks_.empty() ? kWRow : n * kWRow);
        if (networks_.size() > kWMax) h += 18.0;  // scroll hint
        return h;
    }

    void draw(Painter& p, int64_t now) override {
        Rect b = getBounds();
        b.y -= (1.0 - openProgress_.value(now)) * 6.0;
        if (!drawSharedBackdrop(p, b, theme::statusbar::popoverRadius))
            p.fillRoundedRectSource(b, theme::statusbar::popoverRadius, theme::statusbar::panelSurface());

        hits_.clear();
        double y = b.y + kWPad;
        TextStyle hdr{theme::font::family, 11.0, PANGO_WEIGHT_BOLD, theme::color::textSubtle};
        p.drawText(b.x + kWPad, y, "WI-FI", hdr);
        TextStyle st{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL,
                     enabled_ ? theme::color::primary : theme::color::textSubtle};
        p.drawText(b.x + b.w - kWPad, y, enabled_ ? "On" : "Off", st, HAlign::Right);
        y += kWHdr;

        if (!enabled_) {
            TextStyle e{theme::font::family, 12.0, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
            p.drawText(b.x + kWPad, y + 8.0, "Turn on Wi-Fi to see networks", e);
            return;
        }
        if (networks_.empty()) {
            TextStyle e{theme::font::family, 12.0, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
            p.drawText(b.x + kWPad, y + 8.0, "No networks found", e);
            return;
        }

        const size_t total = networks_.size();
        const size_t first = std::min(scroll_, total > kWMax ? total - kWMax : size_t{0});
        for (size_t i = first; i < std::min(first + kWMax, total); ++i) {
            const WifiAp& a = networks_[i];
            const Rect row{b.x + kWPad, y, b.w - kWPad * 2, kWRow};
            // Joinable = active (to disconnect) or saved (to connect).
            const bool joinable = a.active || a.saved;
            if (row.contains(hoverX_, hoverY_) && joinable)
                p.fillRoundedRect(row, 8.0, theme::color::glassHover);

            const Color fg = joinable ? theme::color::text : theme::color::textSubtle.withAlpha(0.6);
            TextStyle gs{theme::font::iconFamily, 15.0, PANGO_WEIGHT_NORMAL,
                         a.active ? theme::color::primary : fg};
            p.drawText(row.x + 4.0, row.y + (kWRow - 16.0) / 2.0, apGlyph(a.strength), gs);

            TextStyle ns{theme::font::family, 13.0,
                         a.active ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL, fg};
            p.drawText(row.x + 30.0, row.y + (kWRow - 15.0) / 2.0, a.ssid, ns, HAlign::Left,
                       row.w - 90.0);

            // Right cluster: saved hint · lock · active check.
            double rx = row.x + row.w - 6.0;
            if (a.active) {
                TextStyle cs{theme::font::iconFamily, 14.0, PANGO_WEIGHT_NORMAL,
                             theme::color::primary};
                const Size cz = p.measureText(kCheck, cs);
                rx -= cz.w;
                p.drawText(rx, row.y + (kWRow - 14.0) / 2.0, kCheck, cs);
                rx -= 6.0;
            }
            if (a.secured) {
                TextStyle ls{theme::font::iconFamily, 12.0, PANGO_WEIGHT_NORMAL,
                             theme::color::textSubtle};
                const Size lz = p.measureText(kLock, ls);
                rx -= lz.w;
                p.drawText(rx, row.y + (kWRow - 12.0) / 2.0, kLock, ls);
                rx -= 6.0;
            }
            if (a.saved && !a.active) {
                TextStyle ss{theme::font::family, 10.0, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
                const Size sz = p.measureText("saved", ss);
                rx -= sz.w;
                p.drawText(rx, row.y + (kWRow - 12.0) / 2.0, "saved", ss);
            }

            if (joinable) hits_.push_back({row, a.ssid, a.active});
            y += kWRow;
        }

        if (total > kWMax) {
            TextStyle m{theme::font::family, 10.0, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
            p.drawText(b.x + kWPad, y + 1.0,
                       std::to_string(first + 1) + "–" +
                           std::to_string(std::min(first + kWMax, total)) + " of " +
                           std::to_string(total) + "  ·  scroll for more",
                       m);
        }
    }

    bool handleClick(double x, double y) override {
        for (const auto& h : hits_) {
            if (!h.r.contains(x, y)) continue;
            if (h.active) backend_->disconnect();
            else backend_->connectSsid(h.ssid);
            closeRequested_ = true;
            return true;
        }
        return false;
    }

    bool handleDrag(double x, double y) override {
        hoverX_ = x;
        hoverY_ = y;
        return false;
    }

    bool handleScroll(double, double dy) override {
        const size_t total = networks_.size();
        if (total <= kWMax) return false;
        const size_t maxScroll = total - kWMax;
        if (dy > 0 && scroll_ > 0) --scroll_;
        else if (dy < 0 && scroll_ < maxScroll) ++scroll_;
        return true;
    }

    bool consumeCloseRequest() override {
        const bool c = closeRequested_;
        closeRequested_ = false;
        return c;
    }

private:
    struct Hit {
        Rect r;
        std::string ssid;
        bool active;
    };

    WifiBackend* backend_ = nullptr;
    bool enabled_ = false;
    std::vector<WifiAp> networks_;
    std::vector<Hit> hits_;
    size_t scroll_ = 0;
    double hoverX_ = -1, hoverY_ = -1;
    bool closeRequested_ = false;
};
}  // namespace

WifiIndicator::WifiIndicator(const SystemBackends& backends)
    : StatusIndicator("wifi", Zone::Right, 300), backend_(backends.wifi) {
    // Hidden until the backend pushes real data; render defaults without one.
    if (backend_) visible = false;
}

std::string WifiIndicator::icon() const {
    return wifiIcon(lastSnap_);
}

std::string WifiIndicator::themedIcon() const {
    return wifiThemedIcon(lastSnap_);
}

std::string WifiIndicator::tooltip() const {
    if (!lastSnap_.enabled) return "WiFi off";
    if (!lastSnap_.connected) return "WiFi disconnected";
    return lastSnap_.ssid + " (" + std::to_string(lastSnap_.strength) + "%)";
}

void WifiIndicator::onBackendUpdate() {
    if (!backend_) return;
    lastSnap_ = backend_->snapshot();
    visible = lastSnap_.available;
}

std::unique_ptr<QSTile> WifiIndicator::createTile() {
    auto snap = &lastSnap_;
    auto backend = backend_;
    return std::make_unique<QSToggleTile>(
        "WiFi", "󰤨",
        [snap]() { return snap->enabled; },
        [backend, snap]() {
            if (backend) backend->setEnabled(!snap->enabled);
        },
        [snap]() -> std::string {
            if (!snap->enabled) return "Off";
            if (!snap->connected) return "Not connected";
            return snap->ssid;
        });
}

std::unique_ptr<DetailedPopover> WifiIndicator::createDetailedView() {
    if (!backend_) return nullptr;
    return std::make_unique<WifiPopover>(backend_, lastSnap_.enabled);
}

REGISTER_INDICATOR("wifi", Zone::Right, 300, WifiIndicator)

}  // namespace qypr
