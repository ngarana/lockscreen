// LockScreen.hpp - Root UI: layout, reveal state machine, input, power menu.
//
// Port of LockScreen.qml. Implements InputSink (keyboard + pointer) and owns
// the shared lock state (revealed, password, status). It is drawn identically
// on every output at that output's size.

#pragma once

#include <array>
#include <string>

#include "auth/PamAuthenticator.hpp"
#include "core/Interfaces.hpp"
#include "ui/ActionButton.hpp"
#include "ui/Clock.hpp"
#include "ui/PasswordField.hpp"
#include "ui/StatusMessage.hpp"

namespace qypr {

class EventLoop;
class PowerManager;
class AudioController;
class VideoPlayer;

class LockScreen : public InputSink {
public:
    LockScreen(EventLoop& loop, RenderHost& host, PamAuthenticator& pam, PowerManager& power);

    // Optional audio panel, injected once MPRIS is available.
    void setAudioController(AudioController* audio) { audio_ = audio; }

    // Optional video background player.
    void setVideoPlayer(VideoPlayer* video) { video_ = video; }

    // Render the whole UI at a given output size.
    void draw(cairo_t* cr, int width, int height, int scale);
    bool isAnimating() const;

    // InputSink
    void onTextInput(const std::string& utf8) override;
    void onSpecialKey(uint32_t keysym, uint32_t modifiers) override;
    void onPointerMotion(int w, int h, double x, double y) override;
    void onPointerButton(int w, int h, double x, double y, uint32_t button, bool pressed) override;
    void onPointerLeave() override;

private:
    void reveal();
    void collapse();
    void restartHideTimer();
    void submitPassword();
    void onAuthResult(PamAuthenticator::Result result, const std::string& message);
    void updateHover(int w, int h, double x, double y);

    // Pure geometry shared by draw() and hit-testing.
    Rect powerButtonRect(int index, int w, int h) const;
    Rect alwaysPowerRect(int w, int h) const;

    EventLoop& loop_;
    RenderHost& host_;
    PamAuthenticator& pam_;
    PowerManager& power_;
    AudioController* audio_ = nullptr;
    VideoPlayer* video_ = nullptr;

    // State
    bool revealed_ = false;
    bool unlocking_ = false;
    std::string password_;
    std::string statusMessage_;
    bool hasError_ = false;
    Animated revealAnim_{0};

    // Widgets
    Clock clock_;
    PasswordField passwordField_;
    StatusMessage status_;
    std::array<ActionButton, 4> powerButtons_;
    ActionButton alwaysPower_;

    bool pointerDown_ = false;
    int hideTimer_ = -1;
    int clockTimer_ = -1;
};

}  // namespace qypr
