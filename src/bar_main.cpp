// bar_main.cpp - qypr-bar entry point.
//
// A standalone, general-purpose Wayland status bar (wlr-layer-shell) for the
// unlocked desktop — a waybar replacement that shares the lock screen's
// StatusBar and indicators. Runs until the compositor or the user tears it down.

#include "core/BarApp.hpp"

int main(int, char**) {
    qypr::BarApp app;
    return app.run();
}
