// PowerMenuIndicator.cpp - Session/power menu implementation.
#include "ui/indicators/PowerMenuIndicator.hpp"

#include <array>
#include <vector>

#include "power/PowerManager.hpp"
#include "render/Painter.hpp"
#include "ui/Theme.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"

namespace qypr {

namespace {

constexpr double kRowH = 40.0;
constexpr double kPad = 8.0;
constexpr double kMenuW = 232.0;

enum class Action { Lock, Suspend, Hibernate, Reboot, Shutdown };

struct Item {
    Action action;
    const char* glyph;
    const char* label;
    bool destructive;  // needs a confirming second click
};

// Ordered least- to most-disruptive, so the dangerous rows are furthest from
// where the pointer lands.
constexpr std::array<Item, 5> kItems{{
    {Action::Lock, "󰌾", "Lock", false},
    {Action::Suspend, "󰒲", "Suspend", true},
    {Action::Hibernate, "󰢠", "Hibernate", true},
    {Action::Reboot, "󰜉", "Restart", true},
    {Action::Shutdown, "󰐥", "Shut Down", true},
}};

class PowerMenuPopover : public DetailedPopover {
public:
    explicit PowerMenuPopover(PowerManager* power) : power_(power) {}

    double contentWidth() const override { return kMenuW; }
    double contentHeight() const override { return kItems.size() * kRowH + kPad * 2.0; }

    void draw(Painter& p, int64_t now) override {
        Rect b = getBounds();
        // Slide in from the bar while opening (fade comes from PopoverManager).
        b.y += (growUp ? 1.0 : -1.0) * (1.0 - openProgress_.value(now)) * 6.0;

        p.fillRoundedRect(b, theme::statusbar::popoverRadius, theme::color::surface);

        TextStyle iconStyle{theme::font::iconFamily, 15.0, PANGO_WEIGHT_NORMAL,
                            theme::color::text};
        TextStyle labelStyle{theme::font::family, 13.0, PANGO_WEIGHT_NORMAL, theme::color::text};

        rows_.clear();
        double y = b.y + kPad;
        for (size_t i = 0; i < kItems.size(); ++i) {
            const Item& it = kItems[i];
            const Rect row{b.x + kPad, y, b.w - kPad * 2.0, kRowH};
            rows_.push_back(row);

            const bool armed = static_cast<int>(i) == armed_;
            if (armed) {
                // Armed rows read as a warning, not a hover.
                p.fillRoundedRect(row, 8.0, theme::color::warning.withAlpha(0.22));
            } else if (row.contains(hoverX_, hoverY_)) {
                p.fillRoundedRect(row, 8.0, theme::color::glassHover);
            }

            iconStyle.color = armed ? theme::color::warning
                                    : (it.destructive ? theme::color::text : theme::color::primary);
            const Size isz = p.measureText(it.glyph, iconStyle);
            p.drawText(row.x + 12.0, row.y + (row.h - isz.h) / 2.0, it.glyph, iconStyle);

            labelStyle.color = armed ? theme::color::warning : theme::color::text;
            const std::string label = armed ? std::string("Confirm ") + it.label : it.label;
            const Size lsz = p.measureText(label, labelStyle);
            p.drawText(row.x + 42.0, row.y + (row.h - lsz.h) / 2.0, label, labelStyle);

            y += kRowH;
        }
    }

    bool handleClick(double x, double y) override {
        for (size_t i = 0; i < rows_.size(); ++i) {
            if (!rows_[i].contains(x, y)) continue;
            const Item& it = kItems[i];

            if (!it.destructive) {
                fire(it.action);
                return true;
            }
            if (armed_ == static_cast<int>(i)) {  // second click on the armed row
                fire(it.action);
                return true;
            }
            armed_ = static_cast<int>(i);  // arm; a click elsewhere disarms
            return true;
        }
        armed_ = -1;
        return false;
    }

    bool handleDrag(double x, double y) override {
        hoverX_ = x;  // PopoverManager routes motion here as a drag
        hoverY_ = y;
        return false;
    }

private:
    void fire(Action a) {
        armed_ = -1;
        if (!power_) return;
        switch (a) {
            case Action::Lock: power_->lock(); break;
            case Action::Suspend: power_->suspend(); break;
            case Action::Hibernate: power_->hibernate(); break;
            case Action::Reboot: power_->reboot(); break;
            case Action::Shutdown: power_->shutdown(); break;
        }
    }

    PowerManager* power_ = nullptr;
    std::vector<Rect> rows_;  // rebuilt each draw; hit-tested on click
    int armed_ = -1;          // index awaiting a confirming second click
    double hoverX_ = -1, hoverY_ = -1;
};

}  // namespace

PowerMenuIndicator::PowerMenuIndicator(const SystemBackends& backends)
    : StatusIndicator("power", Zone::Right, 700), power_(backends.power) {
    visible = power_ != nullptr;
}

Color PowerMenuIndicator::iconColor() const { return theme::color::text; }

bool PowerMenuIndicator::onClick(double, double) {
    return false;
}

std::unique_ptr<DetailedPopover> PowerMenuIndicator::createDetailedView() {
    return std::make_unique<PowerMenuPopover>(power_);
}

REGISTER_INDICATOR("power", Zone::Right, 700, PowerMenuIndicator)

}  // namespace qypr
