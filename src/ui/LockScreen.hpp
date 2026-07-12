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
#include "ui/Notification.hpp"
#include "ui/PasswordField.hpp"
#include "ui/PowerDialog.hpp"
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

    // Windows 11-style notification cards (bottom-left). The owner pushes the
    // current set whenever it changes; the view reconciles by id.
    void setNotifications(std::vector<Notification> notes) {
        notifications_.update(std::move(notes));
    }

    // Idle period (ms) with no input before the video pauses and the screen
    // fades to black. Screen power-off itself is left to the idle daemon.
    void setIdleTimeout(int64_t ms) { idleTimeoutMs_ = ms; }

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
    void restartIdleTimer();
    void enterIdle();  // pause video + fade to black
    void submitPassword();
    void onAuthResult(PamAuthenticator::Result result, const std::string& message);
    void updateHover(int w, int h, double x, double y);
    void expandPower();   // open the pill (anchor clicked)
    void collapsePower(); // close the pill
    // Show the power confirmation popover for button index i, anchored to the pill.
    void showPowerConfirm(int index, int w, int h);

    // Pure geometry shared by draw() and hit-testing.
    // powerRowRect: the full expanded pill rect (all 5 buttons worth of space).
    // powerButtonRect(i): centre rect for action button i (0-3, left-to-right)
    //   inside the row — valid only when revealed.
    // powerAnchorRect: the single trigger/power button, always bottom-right.
    Rect powerRowRect(int w, int h) const;
    Rect powerButtonRect(int index, int w, int h) const;
    Rect powerAnchorRect(int w, int h) const;

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

    // Power pill expand state — driven exclusively by clicking the anchor button,
    // independent of the general reveal state machine.
    bool powerExpanded_ = false;
    Animated powerExpandAnim_{0};

    // Deep-idle: after idleTimeoutMs_ of no input, pause the video and fade the
    // whole screen to black (dimAnim_ 0 -> 1). Any input reverses both.
    bool idle_ = false;
    int64_t idleTimeoutMs_ = 60000;
    int idleTimer_ = -1;
    Animated dimAnim_{0};

    // Widgets
    Clock clock_;
    PasswordField passwordField_;
    StatusMessage status_;
    NotificationView notifications_;
    std::array<ActionButton, 4> powerButtons_;
    ActionButton alwaysPower_;
    PowerDialog powerDialog_;

    bool pointerDown_ = false;
    int hideTimer_ = -1;
    int clockTimer_ = -1;
    int lastW_ = 1920;  // last known output width  (updated every draw)
    int lastH_ = 1080;  // last known output height
};

}  // namespace qypr
