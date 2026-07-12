// Shell.hpp - Root UI compositor: coordinates StatusBar and LockScreen.
//
// Inspired by ChromeOS ash::Shell and GNOME Shell's ScreenShield.
// Implements InputSink to receive events from the platform layer,
// then routes them to the correct child (StatusBar or LockScreen)
// based on priority. Manages the shared idle/dim state.

#pragma once

#include "core/Interfaces.hpp"
#include "ui/LockScreen.hpp"
#include "core/Types.hpp"

namespace qypr {

class EventLoop;
class AudioController;
class VideoPlayer;

class Shell : public InputSink {
public:
    Shell(EventLoop& loop, RenderHost& host, PamAuthenticator& pam, PowerManager& power);

    // Inject optional subsystems (same API LockScreen had).
    void setAudioController(AudioController* audio);
    void setVideoPlayer(VideoPlayer* video);
    void setNotifications(std::vector<Notification> notes);
    void setIdleTimeout(int64_t ms);

    // Render the full compositor stack.
    void draw(cairo_t* cr, int width, int height, int scale);
    bool isAnimating() const;

    // InputSink — routes to StatusBar first, then LockScreen.
    void onTextInput(const std::string& utf8) override;
    void onSpecialKey(uint32_t keysym, uint32_t modifiers) override;
    void onPointerMotion(int w, int h, double x, double y) override;
    void onPointerButton(int w, int h, double x, double y, uint32_t button, bool pressed) override;
    void onPointerLeave() override;

    // Access for App wiring.
    LockScreen& lockScreen() { return lockScreen_; }

private:
    void wakeFromIdle();
    void restartIdleTimer();
    void enterIdle();

    EventLoop& loop_;
    RenderHost& host_;
    VideoPlayer* video_ = nullptr;

    // Children — peers, not parent-child.
    LockScreen lockScreen_;
    // StatusBar* statusBar_ = nullptr;  // Phase 2: will be added when StatusBar is implemented

    // Shared idle state (moved from LockScreen).
    bool idle_ = false;
    int64_t idleTimeoutMs_ = 60000;
    int idleTimer_ = -1;
    Animated dimAnim_{0};
};

}  // namespace qypr
