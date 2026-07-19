// LauncherPopover.hpp - Application launcher search box + result list.
//
// A keyboard-driven fuzzy(-ish) app launcher over DesktopIndex: a search field
// at the top, matching .desktop entries below, keyboard (type / arrows / Enter)
// and pointer (click a row) selection. Firing a result is a user-initiated
// one-shot spawn (spawnDetached) and closes the popover. Bar-only — the lock
// screen never supplies a DesktopIndex, so this is never constructed there.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "system/DesktopIndex.hpp"  // DesktopEntry
#include "ui/statusbar/DetailedPopover.hpp"

namespace qypr {

class DesktopIndex;

class LauncherPopover : public DetailedPopover {
public:
    explicit LauncherPopover(DesktopIndex* index);

    double contentWidth() const override;
    double contentHeight() const override;
    void draw(Painter& p, int64_t now) override;

    // Keyboard-first: grab focus while open so the user can just start typing.
    bool wantsKeyboard() const override { return true; }
    bool handleText(const std::string& utf8) override;  // append to the query
    bool handleKey(uint32_t keysym) override;           // backspace / arrows / enter
    bool handleClick(double x, double y) override;       // click a row to launch
    bool handleDrag(double x, double y) override;        // hover highlight
    bool consumeCloseRequest() override;

private:
    void refresh();          // re-run the search for the current query
    void launch(size_t idx); // spawn results_[idx] and request close
    // Keep the selected row inside the visible window [scroll_, scroll_+kRows).
    void ensureVisible();

    DesktopIndex* index_ = nullptr;
    std::string query_;
    std::vector<const DesktopEntry*> results_;
    size_t sel_ = 0;      // selected result index
    size_t scroll_ = 0;   // first visible result index
    double hoverX_ = -1, hoverY_ = -1;
    bool closeRequested_ = false;
};

}  // namespace qypr
