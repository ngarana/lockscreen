// SNITrayHost.hpp - Status bar system-tray applet (StatusNotifierItem host).
//
// Renders one small icon per tracked SNI item and left-click-activates the
// item under the pointer. Icons come from the item's themed IconName (via the
// shared IconResolver) or, failing that, its IconPixmap.
#pragma once

#include <memory>

#include "ui/statusbar/StatusIndicator.hpp"

namespace qypr {

class SNIBackend;
class DbusMenuBackend;

class SNITrayHost : public StatusIndicator {
public:
    explicit SNITrayHost(const SystemBackends& backends);

    std::string icon() const override { return ""; }  // custom multi-icon draw
    std::string tooltip() const override;

    double measureWidth(Painter& p) override;
    void draw(Painter& p, int64_t now) override;

    void onBackendUpdate() override;
    bool onClick(double x, double y) override;               // left: Activate
    bool onSecondaryClick(double x, double y) override;      // right: dbusmenu
    bool onMiddleClick(double x, double y) override;         // middle: SecondaryActivate

    // A right-click on an item with a menu stashes it here and asks StatusBar to
    // open the detailed view; createDetailedView() then builds the MenuPopover.
    bool hasDetailedView() const override { return pendingMenu_ >= 0; }
    std::unique_ptr<DetailedPopover> createDetailedView() override;

private:
    int iconIndexAt(double x) const;  // which tray icon is under x, or -1

    SNIBackend* backend_ = nullptr;
    DbusMenuBackend* dbusMenu_ = nullptr;
    int pendingMenu_ = -1;  // item index whose menu a right-click requested
};

}  // namespace qypr
