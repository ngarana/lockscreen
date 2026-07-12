// QuickSettingsPanel.hpp - Shared Quick Settings panel widget.
#pragma once

#include "ui/statusbar/DetailedPopover.hpp"
#include "ui/statusbar/QSTile.hpp"
#include <vector>
#include <memory>

namespace qypr {

class QuickSettingsPanel : public DetailedPopover {
public:
    QuickSettingsPanel() = default;
    ~QuickSettingsPanel() override = default;

    void addTile(std::unique_ptr<QSTile> tile);
    void clearTiles() { tiles_.clear(); }

    void draw(Painter& p, int64_t now) override;
    double contentHeight() const override;
    double contentWidth() const override;

    bool handleClick(double x, double y) override;
    bool handleDrag(double x, double y) override;

    // Track active slider drag to route drag events properly
    QSTile* activeDragTile_ = nullptr;

private:
    void layoutTiles();

    std::vector<std::unique_ptr<QSTile>> tiles_;
};

}  // namespace qypr
