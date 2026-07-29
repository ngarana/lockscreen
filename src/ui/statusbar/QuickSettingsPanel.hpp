// QuickSettingsPanel.hpp - Shared Quick Settings panel widget.
#pragma once

#include "ui/statusbar/DetailedPopover.hpp"
#include "ui/statusbar/QSTile.hpp"
#include <vector>
#include <memory>
#include <functional>

namespace qypr {

struct SystemBackends;
class EventLoop;

class QuickSettingsPanel : public DetailedPopover {
public:
    QuickSettingsPanel() = default;
    ~QuickSettingsPanel() override = default;

    // Build the panel's internal tiles from the given system backends.
    // `onPower` is a callback invoked when the power button is clicked.
    // The `backends` pointer is borrowed for the panel's lifetime.
    void buildTiles(EventLoop& loop, const SystemBackends& backends,
                    std::function<void()> onPower);

    void addTile(std::unique_ptr<QSTile> tile);
    void clearTiles() { tiles_.clear(); }

    void draw(Painter& p, int64_t now) override;
    double contentHeight() const override;
    double contentWidth() const override;

    bool handleClick(double x, double y) override;
    bool handleDrag(double x, double y) override;
    bool handleKey(uint32_t keysym) override;
    bool consumeCloseRequest() override;

    Rect findTileBounds(const std::string& tileTitle) const;

    QSTile* activeDragTile_ = nullptr;
    double curX_ = -1, curY_ = -1;

private:
    void layoutTiles();

    // Indicator-created tiles (toggle, slider, info) placed in the grid
    std::vector<std::unique_ptr<QSTile>> tiles_;

    // Internal panel tiles (owned, placed by the panel itself)
    std::unique_ptr<QSHeaderTile> header_;
    std::unique_ptr<QSPowerTile>  power_;
    std::unique_ptr<QSWifiComboTile> wifiCombo_;
    std::unique_ptr<QSVolumeTile> volume_;
    std::unique_ptr<QSMediaTile>  media_;

    // Bounds for the internal tiles (set during layout)
    Rect powerBounds_;
    Rect closeBounds_;
    bool closeRequested_ = false;

};

}  // namespace qypr
