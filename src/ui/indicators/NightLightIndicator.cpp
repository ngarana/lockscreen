// NightLightIndicator.cpp - Night Light applet implementation.
#include "ui/indicators/NightLightIndicator.hpp"

#include "core/Config.hpp"
#include "render/Painter.hpp"
#include "system/NightLightBackend.hpp"
#include "ui/Theme.hpp"
#include "ui/statusbar/DetailedPopover.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"
#include "ui/statusbar/QSTile.hpp"
#include <xkbcommon/xkbcommon-keysyms.h>

namespace qypr {

namespace {
constexpr const char* kMoonGlyph = "\uf186";  // nf-fa-moon_o
constexpr double kPopoverW = 280.0;
constexpr double kPopoverH = 100.0;
constexpr double kPad = 16.0;

// ─────────────────────────────────────────────────────────────────────────
// NightLightPopover — macOS-style temperature slider.
// ─────────────────────────────────────────────────────────────────────────
class NightLightPopover : public DetailedPopover {
public:
    explicit NightLightPopover(NightLightBackend* backend) : backend_(backend) {}

    double contentWidth() const override { return kPopoverW; }
    double contentHeight() const override { return kPopoverH; }

    void draw(Painter& p, int64_t now) override {
        Rect b = getBounds();
        b.y += (growUp ? 1.0 : -1.0) * (1.0 - openProgress_.value(now)) * 6.0;
        p.fillRoundedRect(b, theme::statusbar::popoverRadius, theme::color::surface);

        if (!backend_) return;

        // "Colour Temperature" title.
        TextStyle title{theme::font::family, 12.0, PANGO_WEIGHT_BOLD, theme::color::text};
        p.drawText(b.x + kPad, b.y + kPad, "Colour Temperature", title);

        // Slider track.
        const double trackY = b.y + kPad + 24.0;
        const double trackH = 4.0;
        const double trackX = b.x + kPad;
        const double trackW = b.w - kPad * 2;
        sliderBounds_ = {trackX, trackY, trackW, trackH};
        p.fillRoundedRect(sliderBounds_, trackH / 2.0, theme::color::surface);

        // Slider fill (from left = warm to right = off).
        double val = backend_->sliderValue();
        Rect fillBounds = sliderBounds_;
        fillBounds.w = trackW * val;
        p.fillRoundedRect(fillBounds, trackH / 2.0, theme::color::primary);

        // Thumb dot.
        double thumbX = trackX + trackW * val;
        double thumbY = trackY + trackH / 2.0;
        double thumbRadius = 6.0;
        p.fillCircle(thumbX, thumbY, thumbRadius, theme::color::primary);

        // Labels below the track.
        TextStyle label{theme::font::family, 10.0, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
        p.drawText(trackX, trackY + trackH + 8.0, "Less Warm", label);
        Size rightSz = p.measureText("More Warm", label);
        p.drawText(trackX + trackW - rightSz.w, trackY + trackH + 8.0, "More Warm", label);

        // Current temperature readout.
        if (backend_->enabled()) {
            uint32_t k = backend_->temperature();
            std::string tempStr = std::to_string(k) + " K";
            TextStyle tempStyle{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL, theme::color::primary};
            Size tempSz = p.measureText(tempStr, tempStyle);
            p.drawText(trackX + (trackW - tempSz.w) / 2.0, trackY + trackH + 8.0, tempStr, tempStyle);
        }
    }

    bool handleClick(double x, double y) override {
        if (sliderBounds_.contains(x, y)) {
            dragging_ = true;
            updateFromCoord(x);
            return true;
        }
        return false;
    }

    bool handleDrag(double x, double y) override {
        if (dragging_) {
            updateFromCoord(x);
            return true;
        }
        return false;
    }

    bool handleKey(uint32_t keysym) override {
        if (!backend_) return false;
        switch (keysym) {
            case XKB_KEY_Left:
            case XKB_KEY_Down:
                backend_->setSliderValue(backend_->sliderValue() - 0.05);
                return true;
            case XKB_KEY_Right:
            case XKB_KEY_Up:
                backend_->setSliderValue(backend_->sliderValue() + 0.05);
                return true;
        }
        return false;
    }

private:
    void updateFromCoord(double x) {
        if (!backend_ || sliderBounds_.w <= 0) return;
        double val = (x - sliderBounds_.x) / sliderBounds_.w;
        val = std::clamp(val, 0.0, 1.0);
        backend_->setSliderValue(val);
    }

    NightLightBackend* backend_ = nullptr;
    Rect sliderBounds_;
    bool dragging_ = false;
};

}  // namespace

// ─────────────────────────────────────────────────────────────────────────
// NightLightIndicator
// ─────────────────────────────────────────────────────────────────────────

NightLightIndicator::NightLightIndicator(const SystemBackends& backends)
    : StatusIndicator("night-light", Zone::Right, 230), backend_(backends.nightLight) {
    visible = false;
    // Read the configured colour temperature; applied on first enable.
    if (backends.config && backend_) {
        const int k = backends.config->getInt("night-light", "temperature",
                                              static_cast<int>(backend_->temperature()));
        backend_->setTemperature(static_cast<uint32_t>(k));
    }
}

std::string NightLightIndicator::icon() const {
    if (!backend_ || !backend_->enabled()) return kMoonGlyph;
    uint32_t k = backend_->temperature();
    if (k <= 3200) return "\uf186";  // warm
    return kMoonGlyph;
}

std::string NightLightIndicator::tooltip() const {
    if (!backend_ || !backend_->available()) return "Night Light";
    if (!backend_->enabled()) return "Night Light: off";
    return "Night Light: " + std::to_string(backend_->temperature()) + " K";
}

Color NightLightIndicator::iconColor() const {
    if (backend_ && backend_->enabled()) return theme::color::primary;
    return theme::color::textSubtle;
}

void NightLightIndicator::onBackendUpdate() {
    visible = backend_ && backend_->available();
}

bool NightLightIndicator::onClick(double, double) {
    if (!backend_ || !backend_->available()) return false;
    backend_->toggle();
    return true;
}

bool NightLightIndicator::onScroll(double dx, double dy, double, double) {
    if (!backend_ || !backend_->available()) return false;
    double d = dy != 0.0 ? dy : dx;
    double cur = backend_->sliderValue();
    backend_->setSliderValue(cur + (d < 0 ? 0.05 : -0.05));
    return true;
}

std::unique_ptr<QSTile> NightLightIndicator::createTile() {
    auto* backend = backend_;
    return std::make_unique<QSToggleTile>(
        "Night Light", kMoonGlyph,
        [backend]() { return backend && backend->enabled(); },
        [backend]() {
            if (backend && backend->available()) backend->toggle();
        },
        [backend]() -> std::string {
            if (!backend || !backend->available()) return "Unavailable";
            if (!backend->enabled()) return "Off";
            return std::to_string(backend->temperature()) + " K";
        });
}

std::unique_ptr<DetailedPopover> NightLightIndicator::createDetailedView() {
    if (!backend_) return nullptr;
    return std::make_unique<NightLightPopover>(backend_);
}

REGISTER_INDICATOR("night-light", Zone::Right, 230, NightLightIndicator)

}  // namespace qypr
