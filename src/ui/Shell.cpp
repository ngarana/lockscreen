// Shell.cpp - Root UI compositor implementation
#include "ui/Shell.hpp"
#include "core/EventLoop.hpp"
#include "video/VideoPlayer.hpp"
#include "render/Painter.hpp"
#include "ui/Theme.hpp"

namespace qypr {

Shell::Shell(EventLoop& loop, RenderHost& host, PamAuthenticator& pam, PowerManager& power)
    : loop_(loop),
      host_(host),
      lockScreen_(loop, host, pam, power) {
    restartIdleTimer();
}

void Shell::setAudioController(AudioController* audio) {
    lockScreen_.setAudioController(audio);
}

void Shell::setVideoPlayer(VideoPlayer* video) {
    video_ = video;
}

void Shell::setNotifications(std::vector<Notification> notes) {
    lockScreen_.setNotifications(std::move(notes));
}

void Shell::setIdleTimeout(int64_t ms) {
    idleTimeoutMs_ = ms;
    restartIdleTimer();
}

void Shell::draw(cairo_t* cr, int width, int height, int scale) {
    Painter p(cr);
    const int64_t now = nowMs();
    const double r = lockScreen_.getReveal(now);

    // 1. Draw video background (or fallback gradient if no video).
    if (video_ && video_->hasFrame()) {
        video_->draw(cr, width, height);
    } else {
        p.verticalGradient(width, height, Color::fromHex("#1e1e2e"),
                           Color::fromHex("#181825"), Color::fromHex("#11111b"));
    }

    // Background darken overlay
    p.fillRect({0, 0, static_cast<double>(width), static_cast<double>(height)},
               Color::rgba(0, 0, 0, lerp(0.15, 0.35, r)));

    // 2. Draw lockscreen
    lockScreen_.draw(cr, width, height, scale);

    // 3. Draw deep-idle dim veil
    const double dim = clamp01(dimAnim_.value(now));
    if (dim > 0.001) {
        p.fillRect({0, 0, static_cast<double>(width), static_cast<double>(height)},
                   Color::rgba(0, 0, 0, dim));
    }
}

bool Shell::isAnimating() const {
    const int64_t now = nowMs();
    if (dimAnim_.active(now)) return true;
    return lockScreen_.isAnimating();
}

void Shell::onTextInput(const std::string& utf8) {
    wakeFromIdle();
    lockScreen_.handleTextInput(utf8);
}

void Shell::onSpecialKey(uint32_t keysym, uint32_t modifiers) {
    wakeFromIdle();
    lockScreen_.handleSpecialKey(keysym, modifiers);
}

void Shell::onPointerMotion(int w, int h, double x, double y) {
    wakeFromIdle();
    lockScreen_.handlePointerMotion(w, h, x, y);
}

void Shell::onPointerButton(int w, int h, double x, double y, uint32_t button, bool pressed) {
    wakeFromIdle();
    lockScreen_.handlePointerButton(w, h, x, y, button, pressed);
}

void Shell::onPointerLeave() {
    lockScreen_.handlePointerLeave();
}

void Shell::wakeFromIdle() {
    if (idle_) {
        idle_ = false;
        if (video_) video_->resume();
        dimAnim_.animateTo(0.0, 1500, ease::inOutQuad);
    }
    restartIdleTimer();
}

void Shell::restartIdleTimer() {
    if (idleTimer_ >= 0) loop_.removeTimer(idleTimer_);
    idleTimer_ = loop_.addTimer(idleTimeoutMs_, false, [this] {
        idleTimer_ = -1;
        enterIdle();
    });
}

void Shell::enterIdle() {
    if (idle_) return;
    idle_ = true;
    if (video_) video_->pause();
    dimAnim_.animateTo(1.0, 1500, ease::inOutQuad);
    host_.invalidate();
}

}  // namespace qypr
