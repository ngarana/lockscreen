#include "ui/LockScreen.hpp"

#include <xkbcommon/xkbcommon-keysyms.h>

#include <algorithm>

#include "core/EventLoop.hpp"
#include "power/PowerManager.hpp"
#include "render/Painter.hpp"
#include "video/VideoPlayer.hpp"
#include "ui/AudioController.hpp"
#include "ui/Theme.hpp"
#include "wayland/Seat.hpp"  // Mod flags

namespace qypr {

namespace {
constexpr uint32_t kBtnLeft = 0x110;
constexpr int kHideTimeoutMs = 15000;
constexpr double kButtonDiameter = 52;
constexpr int kDimMs = 1500;  // fade-to-black duration on entering/leaving idle

size_t utf8Count(const std::string& s) {
    size_t n = 0;
    for (unsigned char c : s)
        if ((c & 0xC0) != 0x80) ++n;
    return n;
}
void utf8PopBack(std::string& s) {
    if (s.empty()) return;
    size_t i = s.size();
    do { --i; } while (i > 0 && (static_cast<unsigned char>(s[i]) & 0xC0) == 0x80);
    s.erase(i);
}
}  // namespace

LockScreen::LockScreen(EventLoop& loop, RenderHost& host, PamAuthenticator& pam,
                       PowerManager& power)
    : loop_(loop), host_(host), pam_(pam), power_(power) {
    struct Cfg { const char* icon; const char* label; };
    const Cfg cfg[4] = {
        {"⏾", "Suspend"},    // ⏾
        {"⏻", "Hibernate"},  // ⏻
        {"↻", "Reboot"},     // ↻
        {"⏼", "Shutdown"},   // ⏼
    };
    for (int i = 0; i < 4; ++i) {
        auto& b = powerButtons_[i];
        b.icon = cfg[i].icon;
        b.label = cfg[i].label;
        b.diameter = kButtonDiameter;
    }
    powerButtons_[0].onClick = [this] { power_.suspend(); };
    powerButtons_[1].onClick = [this] { power_.hibernate(); };
    powerButtons_[2].onClick = [this] { power_.reboot(); };
    powerButtons_[3].onClick = [this] { power_.shutdown(); };

    alwaysPower_.icon = "⏻";  // ⏻
    alwaysPower_.label = "Power";
    alwaysPower_.diameter = kButtonDiameter;
    alwaysPower_.onClick = [this] { reveal(); };

    // Repaint once a second so the clock stays current; also re-poll MPRIS.
    // (Notifications are pushed via setNotifications, not polled.)
    clockTimer_ = loop_.addTimer(1000, true, [this] {
        if (audio_) audio_->refresh();
        host_.invalidate();
    });

    // Begin the idle countdown; any input resets it via reveal().
    restartIdleTimer();
}

// -----------------------------------------------------------------------------
// Reveal state machine
// -----------------------------------------------------------------------------
void LockScreen::reveal() {
    // Any activity wakes from deep idle: resume the video and fade the black
    // dim back out, then restart the idle countdown.
    if (idle_) {
        idle_ = false;
        if (video_) video_->resume();
        dimAnim_.animateTo(0.0, kDimMs, ease::inOutQuad);
    }
    restartIdleTimer();

    if (!revealed_) {
        revealed_ = true;
        revealAnim_.animateTo(1.0, theme::anim::reveal, ease::inOutQuad);
    }
    restartHideTimer();
    host_.invalidate();
}

void LockScreen::restartIdleTimer() {
    if (idleTimer_ >= 0) loop_.removeTimer(idleTimer_);
    idleTimer_ = loop_.addTimer(idleTimeoutMs_, false, [this] {
        idleTimer_ = -1;
        enterIdle();
    });
}

void LockScreen::enterIdle() {
    if (idle_) return;
    idle_ = true;
    if (video_) video_->pause();  // stop the decode cost; screen-off is the daemon's job
    dimAnim_.animateTo(1.0, kDimMs, ease::inOutQuad);
    host_.invalidate();
}

void LockScreen::collapse() {
    revealed_ = false;
    revealAnim_.animateTo(0.0, theme::anim::reveal, ease::inOutQuad);
    if (hideTimer_ >= 0) {
        loop_.removeTimer(hideTimer_);
        hideTimer_ = -1;
    }
    host_.invalidate();
}

void LockScreen::restartHideTimer() {
    if (hideTimer_ >= 0) loop_.removeTimer(hideTimer_);
    hideTimer_ = loop_.addTimer(kHideTimeoutMs, false, [this] {
        hideTimer_ = -1;
        if (password_.empty()) collapse();
    });
}

// -----------------------------------------------------------------------------
// Authentication
// -----------------------------------------------------------------------------
void LockScreen::submitPassword() {
    if (password_.empty() || pam_.busy()) return;
    statusMessage_ = "Authenticating...";
    hasError_ = false;
    std::string pw = password_;
    password_.clear();  // never keep the secret around longer than needed
    pam_.authenticate(pw, [this](PamAuthenticator::Result r, std::string m) { onAuthResult(r, m); });
    host_.invalidate();
}

void LockScreen::onAuthResult(PamAuthenticator::Result result, const std::string& message) {
    switch (result) {
        case PamAuthenticator::Result::Success:
            statusMessage_ = "Unlocking...";
            hasError_ = false;
            unlocking_ = true;
            host_.requestUnlock();
            break;
        case PamAuthenticator::Result::Failure:
            statusMessage_ = "Authentication failed";
            hasError_ = true;
            password_.clear();
            break;
        case PamAuthenticator::Result::Error:
            statusMessage_ = message.empty() ? "Authentication error" : ("Error: " + message);
            hasError_ = true;
            password_.clear();
            break;
    }
    host_.invalidate();
}

// -----------------------------------------------------------------------------
// Keyboard
// -----------------------------------------------------------------------------
void LockScreen::onTextInput(const std::string& utf8) {
    password_ += utf8;
    reveal();
}

void LockScreen::onSpecialKey(uint32_t sym, uint32_t modifiers) {
    switch (sym) {
        case XKB_KEY_Escape:
            password_.clear();
            collapse();
            return;
        case XKB_KEY_Return:
        case XKB_KEY_KP_Enter:
            submitPassword();
            return;
        case XKB_KEY_BackSpace:
            utf8PopBack(password_);
            reveal();
            return;
        default:
            break;
    }
    if ((modifiers & MOD_CTRL) && (sym == XKB_KEY_u || sym == XKB_KEY_U)) {
        password_.clear();  // Ctrl+U clears the field
        reveal();
        return;
    }
    reveal();  // any other key still counts as activity
}

// -----------------------------------------------------------------------------
// Pointer
// -----------------------------------------------------------------------------
void LockScreen::onPointerMotion(int w, int h, double x, double y) {
    reveal();
    if (pointerDown_ && audio_) audio_->handleDrag(x, y);
    if (notifications_.active()) notifications_.updateHover(x, y, nowMs());
    updateHover(w, h, x, y);
}

void LockScreen::onPointerButton(int w, int h, double x, double y, uint32_t button, bool pressed) {
    if (button != kBtnLeft) return;

    if (!pressed) {  // release ends any volume drag
        pointerDown_ = false;
        if (audio_) audio_->handleRelease();
        return;
    }

    pointerDown_ = true;
    bool wasRevealed = revealed_;
    reveal();

    // Notification cards are a foreground overlay; dismiss on click.
    if (notifications_.active() && notifications_.handlePress(x, y, nowMs())) {
        host_.invalidate();
        return;
    }

    if (!wasRevealed) return;  // first interaction only reveals

    // Audio panel sits above the power row; give it first refusal.
    if (audio_ && audio_->handlePress(x, y, nowMs())) {
        host_.invalidate();
        return;
    }

    for (int i = 0; i < 4; ++i) {
        powerButtons_[i].bounds = powerButtonRect(i, w, h);
        if (powerButtons_[i].contains(x, y)) {
            powerButtons_[i].click();
            return;
        }
    }
}

void LockScreen::onPointerLeave() {
    int64_t now = nowMs();
    for (auto& b : powerButtons_) b.setHovered(false, now);
    alwaysPower_.setHovered(false, now);
    if (audio_) audio_->clearHover(now);
    notifications_.clearHover(now);
    host_.invalidate();
}

void LockScreen::updateHover(int w, int h, double x, double y) {
    int64_t now = nowMs();
    for (int i = 0; i < 4; ++i) {
        powerButtons_[i].bounds = powerButtonRect(i, w, h);
        powerButtons_[i].setHovered(revealed_ && powerButtons_[i].contains(x, y), now);
    }
    alwaysPower_.bounds = alwaysPowerRect(w, h);
    alwaysPower_.setHovered(!revealed_ && alwaysPower_.contains(x, y), now);
    if (audio_ && revealed_ && audio_->active())
        audio_->updateHover(x, y, now);
    else if (audio_)
        audio_->clearHover(now);
    host_.invalidate();
}

// -----------------------------------------------------------------------------
// Layout geometry
// -----------------------------------------------------------------------------
Rect LockScreen::powerButtonRect(int index, int w, int h) const {
    const double d = kButtonDiameter;
    const double sp = theme::spacing::large;
    const double total = 4 * d + 3 * sp;
    const double startX = w / 2.0 - total / 2.0;
    const double rowY = h - theme::spacing::xxlarge - d;
    return {startX + index * (d + sp), rowY, d, d};
}

Rect LockScreen::alwaysPowerRect(int w, int h) const {
    const double d = kButtonDiameter;
    return {w - theme::spacing::xlarge - d, h - theme::spacing::xlarge - d, d, d};
}

// -----------------------------------------------------------------------------
// Render
// -----------------------------------------------------------------------------
void LockScreen::draw(cairo_t* cr, int width, int height, int) {
    Painter p(cr);
    const int64_t now = nowMs();
    const double r = clamp01(revealAnim_.value(now));
    const double cx = width / 2.0;

    // Video background (or fallback gradient if no video).
    if (video_ && video_->hasFrame()) {
        video_->draw(cr, width, height);
    } else {
        p.verticalGradient(width, height, Color::fromHex("#1e1e2e"),
                           Color::fromHex("#181825"), Color::fromHex("#11111b"));
    }
    p.fillRect({0, 0, static_cast<double>(width), static_cast<double>(height)},
               Color::rgba(0, 0, 0, lerp(0.15, 0.35, r)));

    // Clock (always visible) at 22% down.
    const double clockTop = height * 0.22;
    Size cs = clock_.measure(p);
    clock_.draw(p, cx, clockTop);
    const double clockBottom = clockTop + cs.h;

    // Password field.
    const double pwWidth = std::min(width * 0.35, 420.0);
    passwordField_.bounds = {cx - pwWidth / 2.0, clockBottom + theme::spacing::xxlarge, pwWidth,
                             PasswordField::kHeight};
    passwordField_.charCount = static_cast<int>(utf8Count(password_));

    const double statusTop =
        passwordField_.bounds.y + passwordField_.bounds.h + theme::spacing::medium;
    status_.message = statusMessage_;
    status_.isError = hasError_;
    Size statusSize = status_.measure(p);

    for (int i = 0; i < 4; ++i) powerButtons_[i].bounds = powerButtonRect(i, width, height);
    alwaysPower_.bounds = alwaysPowerRect(width, height);

    // Fade the revealed group in/out as one, and cross-fade the standby power button.
    auto withAlpha = [&](double a, auto&& fn) {
        if (a <= 0.01) return;
        if (a >= 0.999) { fn(); return; }
        p.pushGroup();
        fn();
        p.popGroupWithAlpha(a);
    };

    withAlpha(r, [&] { passwordField_.draw(p, now); });
    withAlpha(r, [&] { status_.draw(p, cx, statusTop); });
    if (audio_ && audio_->active()) {
        double audioTop = statusTop + statusSize.h + theme::spacing::medium;
        double audioWidth = std::min(width - theme::spacing::xlarge * 2.0,
                                     static_cast<double>(theme::audio::maxWidth));
        withAlpha(r, [&] { audio_->draw(p, now, cx, audioTop, audioWidth); });
    }
    withAlpha(r, [&] {
        for (auto& b : powerButtons_) b.draw(p, now);
    });
    withAlpha(lerp(0.7, 0.0, r), [&] { alwaysPower_.draw(p, now); });

    // Windows 11-style notification cards, always visible on the lock screen
    // (bottom-left), below the idle dim veil.
    {
        const double left = theme::spacing::xlarge;
        const double bottom = height - theme::spacing::xlarge;
        const double maxW = std::min(static_cast<double>(width) - 2 * theme::spacing::xlarge,
                                     static_cast<double>(theme::notification::cardWidth));
        notifications_.draw(p, now, left, bottom, maxW);
    }

    // Deep-idle dim: a black veil over everything, on top of all content.
    const double dim = clamp01(dimAnim_.value(now));
    if (dim > 0.001)
        p.fillRect({0, 0, static_cast<double>(width), static_cast<double>(height)},
                   Color::rgba(0, 0, 0, dim));
}

bool LockScreen::isAnimating() const {
    const int64_t now = nowMs();
    if (revealAnim_.active(now)) return true;
    if (dimAnim_.active(now)) return true;
    for (const auto& b : powerButtons_)
        if (b.animating(now)) return true;
    if (alwaysPower_.animating(now)) return true;
    if (notifications_.active() && notifications_.animating(now)) return true;
    return audio_ && audio_->animating(now);
}

}  // namespace qypr
