// BluetoothIndicator.cpp - Status bar Bluetooth indicator implementation.
#include "ui/indicators/BluetoothIndicator.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include "render/Painter.hpp"
#include "ui/Theme.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"
#include "ui/statusbar/QSTile.hpp"

namespace qypr {

namespace {
constexpr double kBW = 300.0;
constexpr double kBPad = 12.0;
constexpr double kBHdr = 24.0;
constexpr double kBRow = 42.0;

// A device glyph from BlueZ's freedesktop Icon category.
const char* btGlyph(const std::string& icon) {
    if (icon.find("headset") != std::string::npos) { return "󰋎"; }
    if (icon.find("headphone") != std::string::npos) { return "󰋋"; }
    if (icon.find("phone") != std::string::npos) { return "󰏳"; }
    if (icon.find("audio") != std::string::npos || icon.find("speaker") != std::string::npos) {
        return "󰓃";
    }
    if (icon.find("keyboard") != std::string::npos) { return "󰌌"; }
    if (icon.find("mouse") != std::string::npos) { return "󰦋"; }
    return "󰂯";  // generic bluetooth
}

// Device picker: paired devices, connected first; click a row to toggle.
class BluetoothPopover : public DetailedPopover {
public:
    explicit BluetoothPopover(BluetoothBackend* backend) : backend_(backend) {}

    [[nodiscard]] double contentWidth() const override { return kBW; }
    [[nodiscard]] double contentHeight() const override {
        const auto& s = backend_->snapshot();
        double const h = (kBPad * 2) + kBHdr;
        if (!s.powered) { return h + kBRow; }
        const size_t n = paired(s).size();
        return h + (n == 0 ? kBRow : static_cast<double>(n) * kBRow);
    }

    void draw(Painter& p, int64_t now) override {
        Rect b = getBounds();
        b.y -= (1.0 - openProgress_.value(now)) * 6.0;
        if (!drawSharedBackdrop(p, b, theme::statusbar::popoverRadius)) {
            p.fillRoundedRectSource(b, theme::statusbar::popoverRadius,
                                    theme::statusbar::panelSurface());
        }

        hits_.clear();
        const auto& s = backend_->snapshot();
        double y = b.y + kBPad;

        TextStyle const hdr{.family = theme::font::family,
                            .size = 11.0,
                            .weight = PANGO_WEIGHT_BOLD,
                            .color = theme::color::textSubtle};
        p.drawText(b.x + kBPad, y, "BLUETOOTH", hdr);
        TextStyle const st{.family = theme::font::family,
                           .size = 11.0,
                           .weight = PANGO_WEIGHT_NORMAL,
                           .color = s.powered ? theme::color::primary : theme::color::textSubtle};
        p.drawText(b.x + b.w - kBPad, y, s.powered ? "On" : "Off", st, HAlign::Right);
        y += kBHdr;

        if (!s.powered) {
            TextStyle const e{.family = theme::font::family,
                              .size = 12.0,
                              .weight = PANGO_WEIGHT_NORMAL,
                              .color = theme::color::textSubtle};
            p.drawText(b.x + kBPad, y + 10.0, "Turn on Bluetooth to see devices", e);
            return;
        }

        const std::vector<BtDevice> devs = paired(s);
        if (devs.empty()) {
            TextStyle const e{.family = theme::font::family,
                              .size = 12.0,
                              .weight = PANGO_WEIGHT_NORMAL,
                              .color = theme::color::textSubtle};
            p.drawText(b.x + kBPad, y + 10.0, "No paired devices", e);
            return;
        }

        for (const auto& d : devs) {
            const Rect row{.x = b.x + kBPad, .y = y, .w = b.w - (kBPad * 2), .h = kBRow};
            if (row.contains(hoverX_, hoverY_)) {
                p.fillRoundedRect(row, 8.0, theme::color::glassHover);
            }

            const Color accent = d.connected ? theme::color::primary : theme::color::text;
            TextStyle const gs{.family = theme::font::iconFamily,
                               .size = 18.0,
                               .weight = PANGO_WEIGHT_NORMAL,
                               .color = accent};
            p.drawText(row.x + 6.0, row.y + ((kBRow - 20.0) / 2.0), btGlyph(d.icon), gs);

            TextStyle const ns{.family = theme::font::family,
                               .size = 13.0,
                               .weight = PANGO_WEIGHT_NORMAL,
                               .color = theme::color::text};
            p.drawText(row.x + 34.0, row.y + 6.0, d.name.empty() ? "Device" : d.name, ns,
                       HAlign::Left, row.w - 40.0);

            std::string sub = d.connected ? "Connected" : "Disconnected";
            if (d.connected && d.battery >= 0) { sub += "  ·  " + std::to_string(d.battery) + "%"; }
            TextStyle const ss{.family = theme::font::family,
                               .size = 11.0,
                               .weight = PANGO_WEIGHT_NORMAL,
                               .color =
                                   d.connected ? theme::color::primary : theme::color::textSubtle};
            p.drawText(row.x + 34.0, row.y + 23.0, sub, ss);

            hits_.push_back({.r = row, .path = d.path, .connected = d.connected});
            y += kBRow;
        }
    }

    bool handleClick(double x, double y) override {
        const auto it =
            std::ranges::find_if(hits_, [&](const Hit& h) { return h.r.contains(x, y); });
        if (it == hits_.end()) { return false; }
        if (it->connected) {
            backend_->disconnectDevice(it->path);
        } else {
            backend_->connectDevice(it->path);
        }
        return true;
    }

    bool handleDrag(double x, double y) override {
        hoverX_ = x;
        hoverY_ = y;
        return false;
    }

private:
    struct Hit {
        Rect r;
        std::string path;
        bool connected;
    };

    // Paired devices, connected ones first, otherwise BlueZ order.
    static std::vector<BtDevice> paired(const BluetoothSnapshot& s) {
        std::vector<BtDevice> out;
        for (const auto& d : s.devices) {
            if (d.paired) { out.push_back(d); }
        }
        std::ranges::stable_sort(
            out, [](const BtDevice& a, const BtDevice& b) { return a.connected > b.connected; });
        return out;
    }

    BluetoothBackend* backend_ = nullptr;
    std::vector<Hit> hits_;
    double hoverX_ = -1, hoverY_ = -1;
};
}  // namespace

BluetoothIndicator::BluetoothIndicator(const SystemBackends& backends)
    : StatusIndicator("bluetooth", Zone::Right, 350),
      backend_(backends.bluetooth) {
    // Reserve the slot and show a neutral placeholder until the first push
    // (instant load, no reflow). Render defaults without a backend.
    loaded_ = (backend_ == nullptr);
}

std::string BluetoothIndicator::icon() const {
    if (!loaded_) {
        return "󰂯";  // generic bluetooth glyph while state is pending
    }
    if (!lastSnap_.powered) { return "󰂲"; }
    return lastSnap_.connectedCount > 0 ? "󰂱" : "󰂯";
}

std::string BluetoothIndicator::themedIcon() const {
    if (!loaded_) {
        return "";  // fall back to the neutral glyph until loaded
    }
    if (!lastSnap_.powered) { return "bluetooth-disabled-symbolic"; }
    return lastSnap_.connectedCount > 0 ? "bluetooth-active-symbolic" : "bluetooth-paired-symbolic";
}

std::string BluetoothIndicator::tooltip() const {
    if (!lastSnap_.powered) { return "Bluetooth off"; }
    if (lastSnap_.connectedCount == 0) { return "Bluetooth on"; }
    if (lastSnap_.connectedCount == 1) { return "Connected: " + lastSnap_.firstDevice; }
    return std::to_string(lastSnap_.connectedCount) + " devices connected";
}

Color BluetoothIndicator::iconColor() const {
    // Blue accent while something is connected.
    if (lastSnap_.powered && lastSnap_.connectedCount > 0) { return theme::color::primary; }
    return theme::color::text;
}

void BluetoothIndicator::onBackendUpdate() {
    if (backend_ == nullptr) { return; }
    lastSnap_ = backend_->snapshot();
    // Only *this* backend's readiness clears the placeholder — a push from an
    // unrelated backend must not mark us loaded with a still-empty snapshot.
    loaded_ = backend_->ready();
    visible = loaded_ ? lastSnap_.available : true;
}

std::unique_ptr<QSTile> BluetoothIndicator::createTile() {
    auto* snap = &lastSnap_;
    auto* backend = backend_;
    return std::make_unique<QSToggleTile>(
        "Bluetooth", "󰂯", [snap]() { return snap->powered; },
        [backend, snap]() {
            if (backend) { backend->setPowered(!snap->powered); }
        },
        [snap]() -> std::string {
            if (!snap->powered) { return "Off"; }
            if (snap->connectedCount == 0) { return "No devices"; }
            if (snap->connectedCount == 1) { return snap->firstDevice; }
            return std::to_string(snap->connectedCount) + " devices";
        });
}

std::unique_ptr<DetailedPopover> BluetoothIndicator::createDetailedView() {
    if (backend_ == nullptr) { return nullptr; }
    return std::make_unique<BluetoothPopover>(backend_);
}

REGISTER_INDICATOR("bluetooth", Zone::Right, 350, BluetoothIndicator)

}  // namespace qypr
