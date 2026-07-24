// QuickSettingsPanel.cpp - Shared Quick Settings panel widget implementation
#include "ui/statusbar/QuickSettingsPanel.hpp"
#include "render/Painter.hpp"
#include "ui/Theme.hpp"
#include <vector>

namespace qypr {

void QuickSettingsPanel::addTile(std::unique_ptr<QSTile> tile) {
    tiles_.push_back(std::move(tile));
}

double QuickSettingsPanel::contentWidth() const {
    return 380.0;
}

double QuickSettingsPanel::contentHeight() const {
    // Dynamic height calculation
    double pad = 16.0;
    double gap = 10.0;
    double y = pad;

    // Toggle tiles (2 columns)
    int togglesCount = 0;
    for (const auto& tile : tiles_) {
        if (tile->type() == QSTile::Type::Toggle) {
            togglesCount++;
        }
    }
    int toggleRows = (togglesCount + 1) / 2;
    if (toggleRows > 0) {
        y += toggleRows * 64.0 + (toggleRows - 1) * gap;
        y += gap;
    }

    // Slider tiles
    for (const auto& tile : tiles_) {
        if (tile->type() == QSTile::Type::Slider) {
            y += 40.0 + gap;
        }
    }

    // Info tiles (status summary rows, full width)
    for (const auto& tile : tiles_) {
        if (tile->type() == QSTile::Type::Info) {
            y += 64.0 + gap;
        }
    }

    return y + pad - gap;
}

void QuickSettingsPanel::layoutTiles() {
    Rect popBounds = getBounds();
    double pad = 16.0;
    double gap = 10.0;

    double contentW = popBounds.w - 2 * pad;
    double y = popBounds.y + pad;

    // 1. Layout Toggle tiles in 2 columns
    std::vector<QSTile*> toggles;
    for (const auto& tile : tiles_) {
        if (tile->type() == QSTile::Type::Toggle) {
            toggles.push_back(tile.get());
        }
    }

    double toggleW = (contentW - gap) / 2.0;
    double toggleH = 64.0;

    for (size_t i = 0; i < toggles.size(); i += 2) {
        toggles[i]->bounds = {popBounds.x + pad, y, toggleW, toggleH};
        if (i + 1 < toggles.size()) {
            toggles[i + 1]->bounds = {popBounds.x + pad + toggleW + gap, y, toggleW, toggleH};
        }
        y += toggleH + gap;
    }

    // 2. Layout Sliders below toggles (span full width)
    for (const auto& tile : tiles_) {
        if (tile->type() == QSTile::Type::Slider) {
            tile->bounds = {popBounds.x + pad, y, contentW, 40.0};
            y += 40.0 + gap;
        }
    }

    // 3. Layout Info tiles below sliders (status summary, full width)
    for (const auto& tile : tiles_) {
        if (tile->type() == QSTile::Type::Info) {
            tile->bounds = {popBounds.x + pad, y, contentW, 64.0};
            y += 64.0 + gap;
        }
    }
}

void QuickSettingsPanel::draw(Painter& p, int64_t now) {
    layoutTiles();

    Rect popBounds = getBounds();

    // Draw popover background: filled surface card, no outline stroke or
    // glass sheen. The popover arrow (formerly glassmorphism detail) is
    // removed — the card reads clean without it in the filled-surface style.
    p.fillRoundedRect(popBounds, 16.0, theme::color::surface);

    // Draw all tiles
    for (const auto& tile : tiles_) {
        // Update tile hover state animation
        tile->hoverAnim_.animateTo(tile->hovered ? 1.0 : 0.0, theme::anim::fast, ease::inOutQuad);
        tile->draw(p, now);
    }
}

bool QuickSettingsPanel::handleClick(double x, double y) {
    activeDragTile_ = nullptr;
    curX_ = x; curY_ = y;
    for (const auto& tile : tiles_) {
        if (tile->bounds.contains(x, y)) {
            tile->onClick(x, y);
            if (tile->type() == QSTile::Type::Slider) {
                activeDragTile_ = tile.get();
            }
            return true;
        }
    }
    return false;
}

bool QuickSettingsPanel::handleDrag(double x, double y) {
    curX_ = x; curY_ = y;
    if (activeDragTile_) {
        activeDragTile_->onDrag(x, y);
        return true;
    }
    return false;
}

bool QuickSettingsPanel::handleKey(uint32_t keysym) {
    // Up/Down (and arrows in general) fine-adjust the slider the cursor is
    // currently over — or the active drag slider if a drag is in progress.
    // Anything else is not ours; the host lets it bubble to Escape handling.
    QSSliderTile* target = nullptr;
    if (activeDragTile_ && activeDragTile_->type() == QSTile::Type::Slider) {
        target = static_cast<QSSliderTile*>(activeDragTile_);
    } else {
        for (const auto& tile : tiles_) {
            if (tile->type() == QSTile::Type::Slider && tile->bounds.contains(curX_, curY_)) {
                target = static_cast<QSSliderTile*>(tile.get());
                break;
            }
        }
    }
    if (target && target->handleKey(keysym)) return true;
    return false;
}

}  // namespace qypr
