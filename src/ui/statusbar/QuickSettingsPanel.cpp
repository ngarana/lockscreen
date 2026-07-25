// QuickSettingsPanel.cpp - Shared Quick Settings panel widget implementation
#include "ui/statusbar/QuickSettingsPanel.hpp"
#include "render/Painter.hpp"
#include "ui/Theme.hpp"
#include "ui/statusbar/StatusIndicator.hpp"
#include "core/Config.hpp"
#include "system/WifiBackend.hpp"
#include "system/BluetoothBackend.hpp"
#include "system/BrightnessBackend.hpp"
#include "system/VolumeBackend.hpp"
#include "system/DndState.hpp"
#include "mpris/MprisController.hpp"
#include "power/PowerManager.hpp"
#include <cstdlib>
#include <unistd.h>
#include <vector>

#include "system/NightLightBackend.hpp"
#include "system/IdleInhibitor.hpp"

namespace qypr {

namespace {
constexpr double kPanelW = 380.0;
constexpr double kPad = 14.0;
constexpr double kGap = 8.0;

// Header
constexpr double kHeaderH = 52.0;

// Toggle grid: 3 equal columns
constexpr int    kGridCols = 3;
constexpr double kGridRowH = 76.0;

// Volume section
constexpr double kVolumeH = 68.0;

// Media card
constexpr double kMediaH = 60.0;

// Config section for QS commands
constexpr const char* kQsSection = "quick-settings";

// Run a shell command in the background (fire-and-forget).
void runCommand(const std::string& cmd) {
    if (cmd.empty()) return;
    std::string detached = cmd + " &";
    std::system(detached.c_str());
}

std::string getUserName() {
    const char* u = std::getenv("USER");
    return u ? std::string(u) : "user";
}

std::string getHostName() {
    char buf[256]{};
    if (gethostname(buf, sizeof(buf)) == 0) {
        buf[sizeof(buf) - 1] = '\0';
        return std::string(buf);
    }
    return "localhost";
}
}  // namespace

// ─── Tile building ────────────────────────────────────────────────────────

void QuickSettingsPanel::buildTiles(EventLoop&, const SystemBackends& backends,
                                   std::function<void()> onPower) {
    // Header
    std::string user = getUserName();
    std::string host = getHostName();
    header_ = std::make_unique<QSHeaderTile>(user, user + "@" + host, theme::color::primary);

    // Power button (right of header)
    std::string powerCmd = "waylaunch --power";
    if (backends.config) {
        powerCmd = backends.config->getString(kQsSection, "power-command", powerCmd);
    }
    power_ = std::make_unique<QSPowerTile>([powerCmd]() {
        runCommand(powerCmd);
    });

    // Wi-Fi combo (if backend present)
    if (backends.wifi) {
        auto snap = backends.wifi;
        wifiCombo_ = std::make_unique<QSWifiComboTile>(
            "Wired connection", 100, true, theme::color::primary);
        wifiCombo_->setEnabled(snap->snapshot().enabled);
        wifiCombo_->setSsid(snap->snapshot().ssid);
        wifiCombo_->setStrength(snap->snapshot().strength);
        backends.wifi->setOnChange([this, snap]() {
            wifiCombo_->setEnabled(snap->snapshot().enabled);
            wifiCombo_->setSsid(snap->snapshot().ssid);
            wifiCombo_->setStrength(snap->snapshot().strength);
        });
    } else if (!wifiCombo_) {
        wifiCombo_ = std::make_unique<QSWifiComboTile>(
            "Wired connection", 100, true, theme::color::primary);
    }

    // Remove any indicator-created WiFi/Network tiles — the combo tile replaces them.
    tiles_.erase(std::remove_if(tiles_.begin(), tiles_.end(), [](const std::unique_ptr<QSTile>& t) {
        if (!t) return false;
        std::string title = t->title();
        return title == "WiFi" || title == "Network" || title == "Wired";
    }), tiles_.end());

    // Purge any volume tiles from grid (volume is handled in dedicated QSVolumeTile card at bottom)
    tiles_.erase(std::remove_if(tiles_.begin(), tiles_.end(), [](const std::unique_ptr<QSTile>& t) {
        return t && (t->type() == QSTile::Type::Volume || t->title() == "Volume");
    }), tiles_.end());

    bool hasBt = false, hasBr = false, hasDnd = false, hasKa = false, hasSs = false;
    for (const auto& t : tiles_) {
        if (!t) continue;
        std::string title = t->title();
        if (title == "Bluetooth") hasBt = true;
        else if (t->type() == QSTile::Type::Slider || title == "Q27G41ZDF" || title == "Brightness") hasBr = true;
        else if (title == "Do Not Disturb") hasDnd = true;
        else if (title == "Keep awake" || title == "Idle Inhibitor") hasKa = true;
        else if (title == "Screenshot") hasSs = true;
    }

    if (!hasBt) {
        if (backends.bluetooth) {
            auto bt = backends.bluetooth;
            tiles_.push_back(std::make_unique<QSToggleTile>(
                "Bluetooth", "󰂯",
                std::function<bool()>([bt]() { return bt->snapshot().powered; }),
                std::function<void()>([bt]() { bt->setPowered(!bt->snapshot().powered); }),
                std::function<std::string()>([bt]() -> std::string {
                    if (!bt->snapshot().powered) return "Off";
                    if (bt->snapshot().connectedCount == 0) return "Not Connected";
                    return bt->snapshot().firstDevice;
                })));
        } else {
            tiles_.push_back(std::make_unique<QSToggleTile>(
                "Bluetooth", "󰂯", std::function<bool()>([]() { return false; }), std::function<void()>([]() {}),
                std::function<std::string()>([]() -> std::string { return "Not Connected"; })));
        }
    }

    if (!hasBr) {
        if (backends.brightness) {
            auto br = backends.brightness;
            tiles_.push_back(std::make_unique<QSSliderTile>(
                "󰃟",
                std::function<double()>([br]() { return br->snapshot().fraction(); }),
                std::function<void(double)>([br](double v) { br->setFraction(v); }),
                nullptr, nullptr, nullptr, "Q27G41ZDF"));
        } else {
            tiles_.push_back(std::make_unique<QSSliderTile>(
                "󰃟", std::function<double()>([]() { return 0.8; }), std::function<void(double)>([](double) {}),
                nullptr, nullptr, nullptr, "Q27G41ZDF"));
        }
    }

    if (!hasDnd) {
        if (backends.dnd) {
            auto dnd = backends.dnd;
            tiles_.push_back(std::make_unique<QSToggleTile>(
                "Do Not Disturb", "󰂜",
                std::function<bool()>([dnd]() { return dnd->enabled(); }),
                std::function<void()>([dnd]() { dnd->toggle(); }),
                std::function<std::string()>([dnd]() -> std::string {
                    return dnd->enabled() ? "On" : "Off";
                })));
        } else {
            tiles_.push_back(std::make_unique<QSToggleTile>(
                "Do Not Disturb", "󰂜", []() { return false; }, []() {},
                []() -> std::string { return "Off"; }));
        }
    }

    // Night Light: always present. Remove any indicator-created tile and
    // re-create from the backend (or a static fallback) so the wiring is
    // consistent regardless of how the indicator was set up.
    {
        tiles_.erase(std::remove_if(tiles_.begin(), tiles_.end(), [](const std::unique_ptr<QSTile>& t) {
            return t && t->title() == "Night Light";
        }), tiles_.end());

        if (backends.nightLight) {
            auto nl = backends.nightLight;
            tiles_.push_back(std::make_unique<QSToggleTile>(
                "Night Light", "󰋌",
                std::function<bool()>([nl]() { return nl && nl->enabled(); }),
                std::function<void()>([nl]() { if (nl && nl->available()) nl->toggle(); }),
                std::function<std::string()>([nl]() -> std::string {
                    if (!nl || !nl->available()) return "Off";
                    return nl->enabled() ? "On" : "Off";
                })));
        } else {
            tiles_.push_back(std::make_unique<QSToggleTile>(
                "Night Light", "󰋌", []() { return false; }, []() {},
                []() -> std::string { return "Off"; }));
        }
    }

    if (!hasKa) {
        if (backends.idleInhibitor) {
            auto ii = backends.idleInhibitor;
            tiles_.push_back(std::make_unique<QSToggleTile>(
                "Keep awake", "󰅶",
                std::function<bool()>([ii]() { return ii && ii->active(); }),
                std::function<void()>([ii]() { if (ii && ii->available()) ii->toggle(); }),
                std::function<std::string()>([ii]() -> std::string {
                    if (!ii || !ii->available()) return "Off";
                    return ii->active() ? "On" : "Off";
                })));
        } else {
            tiles_.push_back(std::make_unique<QSToggleTile>(
                "Keep awake", "󰅶", []() { return false; }, []() {},
                []() -> std::string { return "Off"; }));
        }
    }

    if (!hasSs) {
        std::string ssCmd = "flameshot gui";
        if (backends.config) {
            ssCmd = backends.config->getString(kQsSection, "screenshot-command", ssCmd);
        }
        tiles_.push_back(std::make_unique<QSToggleTile>(
            "Screenshot", "󰄄", []() { return false; }, [ssCmd]() {
                runCommand(ssCmd);
            }, []() -> std::string { return "Screenshot"; }));
    }

    // Volume section
    if (backends.volume) {
        auto vsnap = backends.volume;
        volume_ = std::make_unique<QSVolumeTile>(
            [vsnap]() { return vsnap->snapshot().level; },
            [vsnap](double v) { vsnap->setLevel(v); },
            [vsnap]() -> std::string {
                if (vsnap->snapshot().muted) return "󰝟";
                return "󰕾";
            },
            [vsnap]() { vsnap->toggleMute(); },
            [vsnap]() { return vsnap->snapshot().muted; });
    } else {
        volume_ = std::make_unique<QSVolumeTile>(
            []() { return 0.75; }, [](double) {}, []() -> std::string { return "󰕾"; },
            []() {}, []() { return false; });
    }

    // Media card
    media_ = std::make_unique<QSMediaTile>(backends.mpris);
}

void QuickSettingsPanel::addTile(std::unique_ptr<QSTile> tile) {
    tiles_.push_back(std::move(tile));
}

// ─── Geometry ─────────────────────────────────────────────────────────────

double QuickSettingsPanel::contentWidth() const { return kPanelW; }

double QuickSettingsPanel::contentHeight() const {
    double h = kPad; // top padding

    // Header row
    h += kHeaderH;

    // Grid row 1 & row 2
    int totalCells = static_cast<int>(tiles_.size()) + (wifiCombo_ ? 1 : 0);
    int rows = (totalCells + kGridCols - 1) / kGridCols;
    if (rows > 0) {
        h += kGap;
        h += rows * kGridRowH + (rows - 1) * kGap;
    }

    // Volume section
    if (volume_) {
        h += kGap;
        h += kVolumeH;
    }

    // Media card
    if (media_) {
        h += kGap;
        h += kMediaH;
    }

    h += kPad; // bottom padding
    return h;
}

// ─── Layout ───────────────────────────────────────────────────────────────

void QuickSettingsPanel::layoutTiles() {
    Rect popBounds = getBounds();
    double contentW = popBounds.w - 2 * kPad;
    double colW = (contentW - (kGridCols - 1) * kGap) / kGridCols;
    double y = popBounds.y + kPad;

    // 1. Header (left side) & Power button (right side)
    double powerW = 52.0;
    double headerW = contentW - powerW - kGap;
    if (header_) {
        header_->bounds = {popBounds.x + kPad, y, headerW, kHeaderH};
    }

    if (power_) {
        double px = popBounds.x + popBounds.w - kPad - powerW;
        powerBounds_ = {px, y, powerW, kHeaderH};
        power_->bounds = powerBounds_;
    }

    y += kHeaderH + kGap;

    // 2. 3-Column Toggle & Slider Grid
    std::vector<QSTile*> gridTiles;
    if (wifiCombo_) gridTiles.push_back(wifiCombo_.get());
    for (auto& t : tiles_) {
        gridTiles.push_back(t.get());
    }

    int col = 0;
    double rowY = y;
    for (size_t i = 0; i < gridTiles.size(); ++i) {
        auto* tile = gridTiles[i];
        if (col >= kGridCols) {
            col = 0;
            rowY += kGridRowH + kGap;
        }
        double tx = popBounds.x + kPad + col * (colW + kGap);
        tile->bounds = {tx, rowY, colW, kGridRowH};
        col++;
    }

    if (!gridTiles.empty()) {
        y = rowY + kGridRowH;
    }

    // 3. Volume section
    if (volume_) {
        y += kGap;
        volume_->bounds = {popBounds.x + kPad, y, contentW, kVolumeH};
        y += kVolumeH;
    }

    // 4. Media card
    if (media_) {
        y += kGap;
        media_->bounds = {popBounds.x + kPad, y, contentW, kMediaH};
    }
}

// ─── Draw ─────────────────────────────────────────────────────────────────

void QuickSettingsPanel::draw(Painter& p, int64_t now) {
    layoutTiles();
    Rect popBounds = getBounds();

    // Main panel background: dark slate rounded rect
    p.fillRoundedRect(popBounds, 16.0, theme::color::background);
    p.strokeRoundedRect(popBounds, 16.0, theme::color::surface, 1.5);

    // Close button (X) at top right of the panel canvas
    closeBounds_ = {popBounds.x + popBounds.w - kPad - 24.0, popBounds.y + 8.0, 24.0, 24.0};
    p.fillCircle(closeBounds_.x + 12.0, closeBounds_.y + 12.0, 12.0, theme::color::surface);
    p.strokeCircle(closeBounds_.x + 12.0, closeBounds_.y + 12.0, 12.0, theme::color::surfaceHover, 1.0);

    TextStyle closeStyle{theme::font::family, 12.0, PANGO_WEIGHT_BOLD, theme::color::text};
    Size closeSz = p.measureText("✕", closeStyle);
    p.drawText(closeBounds_.x + 12.0 - closeSz.w / 2.0, closeBounds_.y + 12.0 - closeSz.h / 2.0, "✕", closeStyle);

    auto drawOrSkip = [&](auto& tile) {
        if (!tile) return;
        tile->hoverAnim_.animateTo(tile->hovered ? 1.0 : 0.0,
                                   theme::anim::fast, ease::inOutQuad);
        tile->draw(p, now);
    };

    drawOrSkip(header_);
    drawOrSkip(power_);
    drawOrSkip(wifiCombo_);
    for (auto& t : tiles_) drawOrSkip(t);
    drawOrSkip(volume_);
    drawOrSkip(media_);
}

// ─── Input ────────────────────────────────────────────────────────────────

bool QuickSettingsPanel::consumeCloseRequest() {
    if (closeRequested_) {
        closeRequested_ = false;
        return true;
    }
    return false;
}

bool QuickSettingsPanel::handleClick(double x, double y) {
    activeDragTile_ = nullptr;
    curX_ = x;
    curY_ = y;

    if (closeBounds_.contains(x, y)) {
        closeRequested_ = true;
        return true;
    }

    if (power_ && power_->bounds.contains(x, y)) {
        power_->onClick(x, y);
        return true;
    }

    if (volume_ && volume_->bounds.contains(x, y)) {
        volume_->onClick(x, y);
        activeDragTile_ = volume_.get();
        return true;
    }

    for (auto& t : tiles_) {
        if (t->bounds.contains(x, y)) {
            t->onClick(x, y);
            if (t->type() == QSTile::Type::Slider) activeDragTile_ = t.get();
            return true;
        }
    }

    if (media_ && media_->bounds.contains(x, y)) {
        media_->onClick(x, y);
        return true;
    }

    if (wifiCombo_ && wifiCombo_->bounds.contains(x, y)) {
        return true;
    }

    return false;
}

bool QuickSettingsPanel::handleDrag(double x, double y) {
    curX_ = x;
    curY_ = y;
    if (activeDragTile_) {
        activeDragTile_->onDrag(x, y);
        return true;
    }
    return false;
}

bool QuickSettingsPanel::handleKey(uint32_t keysym) {
    QSTile* target = nullptr;
    if (activeDragTile_ &&
        (activeDragTile_->type() == QSTile::Type::Slider ||
         activeDragTile_->type() == QSTile::Type::Volume)) {
        target = activeDragTile_;
    } else {
        for (auto& t : tiles_) {
            if (t->type() == QSTile::Type::Slider && t->bounds.contains(curX_, curY_)) {
                target = t.get();
                break;
            }
        }
    }
    if (target && target->handleKey(keysym)) return true;
    return false;
}

}  // namespace qypr
