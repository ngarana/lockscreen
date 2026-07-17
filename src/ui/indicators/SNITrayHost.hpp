// SNITrayHost.hpp - Status bar system-tray applet (StatusNotifierItem host).
//
// Renders one small icon per tracked SNI item and left-click-activates the
// item under the pointer. Icons come from the item's themed IconName (via the
// shared IconResolver) or, failing that, its IconPixmap.
#pragma once

#include "ui/statusbar/StatusIndicator.hpp"

namespace qypr {

class SNIBackend;

class SNITrayHost : public StatusIndicator {
public:
    explicit SNITrayHost(const SystemBackends& backends);

    std::string icon() const override { return ""; }  // custom multi-icon draw
    std::string tooltip() const override;

    double measureWidth(Painter& p) override;
    void draw(Painter& p, int64_t now) override;

    void onBackendUpdate() override;
    bool onClick(double x, double y) override;

private:
    SNIBackend* backend_ = nullptr;
};

}  // namespace qypr
