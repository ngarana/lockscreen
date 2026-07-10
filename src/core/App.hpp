// App.hpp - Wires the platform, auth, power, and UI layers together.
//
// Implements RenderHost so the UI can request repaints and unlock. Owns the
// single event loop everything runs on.

#pragma once

#include "auth/PamAuthenticator.hpp"
#include "core/EventLoop.hpp"
#include "core/Interfaces.hpp"
#include "mpris/MprisController.hpp"
#include "power/PowerManager.hpp"
#include "ui/AudioController.hpp"
#include "ui/LockScreen.hpp"
#include "video/VideoPlayer.hpp"
#include "wayland/LockSession.hpp"
#include "wayland/WaylandDisplay.hpp"

namespace qypr {

class App : public RenderHost {
public:
    App();

    int run();

    // Idle seconds before the video pauses and the screen dims (default 60).
    void setIdleTimeout(int seconds);

    // Render idle + revealed frames to PNGs (no Wayland lock) for visual
    // verification and previewing. Writes <path> and <path>-idle.png.
    int preview(const std::string& path, int width = 1920, int height = 1080);

    // Exercise the video pipeline (EGL + mpv) offscreen without locking the
    // session. Runs for `seconds` and reports whether frames were produced.
    int videoTest(int seconds = 6);

    // RenderHost
    void invalidate() override;
    void requestUnlock() override;

private:
    EventLoop loop_;
    WaylandDisplay display_;
    LockSession lock_;
    PamAuthenticator pam_;
    PowerManager power_;
    MprisController mpris_;
    AudioController audio_;
    VideoPlayer video_;
    LockScreen lockScreen_;
};

}  // namespace qypr
