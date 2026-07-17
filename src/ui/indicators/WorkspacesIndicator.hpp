// WorkspacesIndicator.hpp - Compositor workspaces widget (ext-workspace-v1).
//
// Session-sensitive: hidden while the screen is locked (see
// StatusIndicator::sensitive / StatusBar::setSessionContentVisible). Renders a
// pill per workspace, the active one accented, and switches workspace on click.
#pragma once

#include <string>
#include <vector>

#include "system/WorkspaceBackend.hpp"  // WorkspaceSnapshot
#include "ui/statusbar/StatusIndicator.hpp"

namespace qypr {

class WorkspacesIndicator : public StatusIndicator {
public:
    explicit WorkspacesIndicator(const SystemBackends& backends);

    std::string icon() const override { return ""; }  // custom pill draw
    std::string tooltip() const override;
    bool sensitive() const override { return true; }

    double measureWidth(Painter& p) override;
    void draw(Painter& p, int64_t now) override;
    void onBackendUpdate() override;
    bool onClick(double x, double y) override;

private:
    double pillWidth(Painter& p, const std::string& name) const;

    WorkspaceBackend* backend_ = nullptr;
    WorkspaceSnapshot snap_;

    // Per-pill x-extents from the last draw, for click hit-testing.
    struct Hit {
        double x0, x1;
        std::string name;
    };
    std::vector<Hit> hits_;
};

}  // namespace qypr
