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

    return y + pad;
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
}

void QuickSettingsPanel::draw(Painter& p, int64_t now) {
    layoutTiles();

    Rect popBounds = getBounds();

    // Draw card popover background (glass-morphism)
    p.fillRoundedRect(popBounds, 16.0, theme::color::glass);
    p.strokeRoundedRect(popBounds, 16.0, theme::color::glassBorder, 1.0);

    // Draw popover arrow pointing up at top right of the card
    double arrowX = anchorX - 24.0;
    double arrowY = anchorY;

    cairo_t* cr = p.cr();
    cairo_save(cr);
    cairo_new_path(cr);
    cairo_move_to(cr, arrowX - 8.0, arrowY + 1.0); // slight bleed for overlap
    cairo_line_to(cr, arrowX, arrowY - 8.0);
    cairo_line_to(cr, arrowX + 8.0, arrowY + 1.0);
    cairo_close_path(cr);
    cairo_set_source_rgba(cr, theme::color::glass.r, theme::color::glass.g, theme::color::glass.b, theme::color::glass.a);
    cairo_fill(cr);

    // Stroke the arrow sides
    cairo_new_path(cr);
    cairo_move_to(cr, arrowX - 8.0, arrowY + 1.0);
    cairo_line_to(cr, arrowX, arrowY - 8.0);
    cairo_line_to(cr, arrowX + 8.0, arrowY + 1.0);
    cairo_set_source_rgba(cr, theme::color::glassBorder.r, theme::color::glassBorder.g, theme::color::glassBorder.b, theme::color::glassBorder.a);
    cairo_set_line_width(cr, 1.0);
    cairo_stroke(cr);
    cairo_restore(cr);

    // Draw all tiles
    for (const auto& tile : tiles_) {
        // Update tile hover state animation
        tile->hoverAnim_.animateTo(tile->hovered ? 1.0 : 0.0, theme::anim::fast, ease::inOutQuad);
        tile->draw(p, now);
    }
}

bool QuickSettingsPanel::handleClick(double x, double y) {
    activeDragTile_ = nullptr;
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
    if (activeDragTile_) {
        activeDragTile_->onDrag(x, y);
        return true;
    }
    return false;
}

}  // namespace qypr
