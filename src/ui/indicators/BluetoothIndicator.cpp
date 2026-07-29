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
    if (icon.find("headset") != std::string::npos) return "󰋎";
    if (icon.find("headphone") != std::string::npos) return "󰋋";
    if (icon.find("phone") != std::string::npos) return "󰏳";
    if (icon.find("audio") != std::string::npos || icon.find("speaker") != std::string::npos)
        return "󰓃";
    if (icon.find("keyboard") != std::string::npos) return "󰌌";
    if (icon.find("mouse") != std::string::npos) return "󰦋";
    return "󰂯";  // generic bluetooth
}

// Device picker: paired devices, connected first; click a row to toggle.
class BluetoothPopover : public DetailedPopover {
public:
    explicit BluetoothPopover(BluetoothBackend* backend) : backend_(backend) {}

    double contentWidth() const override { return kBW; }
    double contentHeight() const override {
        const auto& s = backend_->snapshot();
        double h = kBPad * 2 + kBHdr;
        if (!s.powered) return h + kBRow;
        const size_t n = paired(s).size();
        return h + (n == 0 ? kBRow : n * kBRow);
    }

    void draw(Painter& p, int64_t now) override {
        Rect b = getBounds();
        b.y -= (1.0 - openProgress_.value(now)) * 6.0;
        if (!drawSharedBackdrop(p, b, theme::statusbar::popoverRadius))
            p.fillRoundedRectSource(b, theme::statusbar::popoverRadius, theme::statusbar::panelSurface());

        hits_.clear();
        const auto& s = backend_->snapshot();
        double y = b.y + kBPad;

        TextStyle hdr{theme::font::family, 11.0, PANGO_WEIGHT_BOLD, theme::color::textSubtle};
        p.drawText(b.x + kBPad, y, "BLUETOOTH", hdr);
        TextStyle st{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL,
                     s.powered ? theme::color::primary : theme::color::textSubtle};
        p.drawText(b.x + b.w - kBPad, y, s.powered ? "On" : "Off", st, HAlign::Right);
        y += kBHdr;

        if (!s.powered) {
            TextStyle e{theme::font::family, 12.0, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
            p.drawText(b.x + kBPad, y + 10.0, "Turn on Bluetooth to see devices", e);
            return;
        }

        const std::vector<BtDevice> devs = paired(s);
        if (devs.empty()) {
            TextStyle e{theme::font::family, 12.0, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
            p.drawText(b.x + kBPad, y + 10.0, "No paired devices", e);
            return;
        }

        for (const auto& d : devs) {
            const Rect row{b.x + kBPad, y, b.w - kBPad * 2, kBRow};
            if (row.contains(hoverX_, hoverY_))
                p.fillRoundedRect(row, 8.0, theme::color::glassHover);

            const Color accent = d.connected ? theme::color::primary : theme::color::text;
            TextStyle gs{theme::font::iconFamily, 18.0, PANGO_WEIGHT_NORMAL, accent};
            p.drawText(row.x + 6.0, row.y + (kBRow - 20.0) / 2.0, btGlyph(d.icon), gs);

            TextStyle ns{theme::font::family, 13.0, PANGO_WEIGHT_NORMAL, theme::color::text};
            p.drawText(row.x + 34.0, row.y + 6.0, d.name.empty() ? "Device" : d.name, ns,
                       HAlign::Left, row.w - 40.0);

            std::string sub = d.connected ? "Connected" : "Disconnected";
            if (d.connected && d.battery >= 0) sub += "  ·  " + std::to_string(d.battery) + "%";
            TextStyle ss{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL,
                         d.connected ? theme::color::primary : theme::color::textSubtle};
            p.drawText(row.x + 34.0, row.y + 23.0, sub, ss);

            hits_.push_back({row, d.path, d.connected});
            y += kBRow;
        }
    }

    bool handleClick(double x, double y) override {
        for (const auto& h : hits_) {
            if (!h.r.contains(x, y)) continue;
            if (h.connected) backend_->disconnectDevice(h.path);
            else backend_->connectDevice(h.path);
            return true;
        }
        return false;
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
        for (const auto& d : s.devices)
            if (d.paired) out.push_back(d);
        std::stable_sort(out.begin(), out.end(),
                         [](const BtDevice& a, const BtDevice& b) { return a.connected > b.connected; });
        return out;
    }

    BluetoothBackend* backend_ = nullptr;
    std::vector<Hit> hits_;
    double hoverX_ = -1, hoverY_ = -1;
};
}  // namespace

BluetoothIndicator::BluetoothIndicator(const SystemBackends& backends)
    : StatusIndicator("bluetooth", Zone::Right, 350), backend_(backends.bluetooth) {
    // Hidden until the backend pushes real data; render defaults without one.
    if (backend_) visible = false;
}

std::string BluetoothIndicator::icon() const {
    if (!lastSnap_.powered) return "󰂲";
    return lastSnap_.connectedCount > 0 ? "󰂱" : "󰂯";
}

std::string BluetoothIndicator::themedIcon() const {
    if (!lastSnap_.powered) return "bluetooth-disabled-symbolic";
    return lastSnap_.connectedCount > 0 ? "bluetooth-active-symbolic" : "bluetooth-paired-symbolic";
}

std::string BluetoothIndicator::tooltip() const {
    if (!lastSnap_.powered) return "Bluetooth off";
    if (lastSnap_.connectedCount == 0) return "Bluetooth on";
    if (lastSnap_.connectedCount == 1) return "Connected: " + lastSnap_.firstDevice;
    return std::to_string(lastSnap_.connectedCount) + " devices connected";
}

Color BluetoothIndicator::iconColor() const {
    // Blue accent while something is connected.
    if (lastSnap_.powered && lastSnap_.connectedCount > 0) return theme::color::primary;
    return theme::color::text;
}

void BluetoothIndicator::onBackendUpdate() {
    if (!backend_) return;
    lastSnap_ = backend_->snapshot();
    visible = lastSnap_.available;
}

std::unique_ptr<QSTile> BluetoothIndicator::createTile() {
    auto snap = &lastSnap_;
    auto backend = backend_;
    return std::make_unique<QSToggleTile>(
        "Bluetooth", "󰂯",
        [snap]() { return snap->powered; },
        [backend, snap]() {
            if (backend) backend->setPowered(!snap->powered);
        },
        [snap]() -> std::string {
            if (!snap->powered) return "Off";
            if (snap->connectedCount == 0) return "No devices";
            if (snap->connectedCount == 1) return snap->firstDevice;
            return std::to_string(snap->connectedCount) + " devices";
        });
}

std::unique_ptr<DetailedPopover> BluetoothIndicator::createDetailedView() {
    if (!backend_) return nullptr;
    return std::make_unique<BluetoothPopover>(backend_);
}

REGISTER_INDICATOR("bluetooth", Zone::Right, 350, BluetoothIndicator)

}  // namespace qypr
