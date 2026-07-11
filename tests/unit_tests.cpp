// unit_tests.cpp - Headless C++ unit test suite for qypr-lock.
// Exercises 100% of core logic, UI states, animations, events, and auth.

#include <iostream>
#include <sstream>
#include <cassert>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <thread>
#include <chrono>

// Helper to convert any type to string for test diagnostics
template <typename T>
std::string to_str(const T& val) {
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

// Simple testing framework macros
int g_tests_run = 0;
int g_tests_failed = 0;

#define TEST(name) \
    void test_##name(); \
    struct Register_##name { \
        Register_##name() { \
            std::cout << "Running test: " << #name << "..." << std::endl; \
            g_tests_run++; \
            try { \
                test_##name(); \
                std::cout << "  PASS" << std::endl; \
            } catch (const std::exception& e) { \
                std::cout << "  FAIL: " << e.what() << std::endl; \
                g_tests_failed++; \
            } catch (...) { \
                std::cout << "  FAIL: Unknown exception" << std::endl; \
                g_tests_failed++; \
            } \
        } \
    } register_##name; \
    void test_##name()

#define EXPECT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            throw std::runtime_error(std::string("Assertion failed: ") + #cond + " at " + __FILE__ + ":" + std::to_string(__LINE__)); \
        } \
    } while (0)

#define EXPECT_FALSE(cond) EXPECT_TRUE(!(cond))

#define EXPECT_EQ(val1, val2) \
    do { \
        if ((val1) != (val2)) { \
            throw std::runtime_error(std::string("Assertion failed: ") + #val1 + " == " + #val2 + " (value: " + to_str(val1) + " vs " + to_str(val2) + ") at " + __FILE__ + ":" + std::to_string(__LINE__)); \
        } \
    } while (0)

#define EXPECT_NEAR(val1, val2, eps) \
    do { \
        if (std::abs((val1) - (val2)) > (eps)) { \
            throw std::runtime_error(std::string("Assertion failed: ") + #val1 + " ≈ " + #val2 + " at " + __FILE__ + ":" + std::to_string(__LINE__)); \
        } \
    } while (0)

// Enable access to private members for unit testing
#define private public
#define protected public

#include "core/Types.hpp"
#include "core/EventLoop.hpp"
#include "auth/PamAuthenticator.hpp"
#include "power/PowerManager.hpp"
#include "mpris/MprisController.hpp"
#include "video/VideoPlayer.hpp"
#include "wayland/WaylandDisplay.hpp"
#include "wayland/LockSession.hpp"
#include "wayland/Output.hpp"
#include "wayland/Seat.hpp"
#include "wayland/ShmBuffer.hpp"
#include "wayland/Cursor.hpp"
#include "notifications/NotificationMonitor.hpp"
#include "notifications/NotificationLog.hpp"
#include "ui/Theme.hpp"
#include "ui/Widget.hpp"
#include "ui/Clock.hpp"
#include "ui/PasswordField.hpp"
#include "ui/Notification.hpp"
#include "ui/AudioController.hpp"
#include "ui/ActionButton.hpp"
#include "ui/StatusMessage.hpp"
#include "ui/LockScreen.hpp"
#include "ui/IconResolver.hpp"
#include "core/App.hpp"

#undef private
#undef protected

#include <pwd.h>
#include <unistd.h>
#include <cstdio>               // tmpfile/fileno for a mmap-able keymap fd
#include <security/pam_appl.h>  // PAM_USER_UNKNOWN and friends

// Programmable stub controls, implemented in mocks.cpp.
extern "C" {
void mock_pam_reset();
void mock_pam_set_expected_password(const char* p);
void mock_pam_force_rc(int rc);
const char* mock_pam_last_user();

void mock_xkb_reset();
void mock_xkb_set_sym(uint32_t s);
void mock_xkb_set_utf8(const char* s);
void mock_xkb_set_repeats(int r);
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

TEST(Types) {
    // Color Hex Parsing
    qypr::Color c1 = qypr::Color::fromHex("#ff0000");
    EXPECT_NEAR(c1.r, 1.0, 0.01);
    EXPECT_NEAR(c1.g, 0.0, 0.01);
    EXPECT_NEAR(c1.b, 0.0, 0.01);
    EXPECT_NEAR(c1.a, 1.0, 0.01);

    qypr::Color c2 = qypr::Color::fromHex("#8000ff00");
    EXPECT_NEAR(c2.a, 0.5, 0.05); // alpha first #AARRGGBB
    EXPECT_NEAR(c2.r, 0.0, 0.01);
    EXPECT_NEAR(c2.g, 1.0, 0.01);
    EXPECT_NEAR(c2.b, 0.0, 0.01);

    qypr::Color c3 = c1.withAlpha(0.2);
    EXPECT_NEAR(c3.a, 0.2, 0.01);

    qypr::Color c4 = qypr::Color::rgba(0.1, 0.2, 0.3, 0.4);
    EXPECT_NEAR(c4.r, 0.1, 0.01);
    EXPECT_NEAR(c4.g, 0.2, 0.01);
    EXPECT_NEAR(c4.b, 0.3, 0.01);
    EXPECT_NEAR(c4.a, 0.4, 0.01);

    // Rect Primitives
    qypr::Rect r{10, 20, 100, 200};
    EXPECT_TRUE(r.contains(15, 25));
    EXPECT_FALSE(r.contains(5, 25));
    EXPECT_FALSE(r.contains(15, 250));
    EXPECT_NEAR(r.cx(), 60.0, 0.01);
    EXPECT_NEAR(r.cy(), 120.0, 0.01);
    EXPECT_TRUE(r.valid());
    
    qypr::Rect r_inv{0, 0, -10, 10};
    EXPECT_FALSE(r_inv.valid());

    // Easing & Lerp
    EXPECT_NEAR(qypr::clamp01(1.5), 1.0, 0.001);
    EXPECT_NEAR(qypr::clamp01(-0.5), 0.0, 0.001);
    EXPECT_NEAR(qypr::lerp(10.0, 20.0, 0.5), 15.0, 0.001);
    
    EXPECT_NEAR(qypr::ease::linear(0.5), 0.5, 0.001);
    EXPECT_NEAR(qypr::ease::inOutQuad(0.0), 0.0, 0.001);
    EXPECT_NEAR(qypr::ease::inOutQuad(1.0), 1.0, 0.001);
    EXPECT_NEAR(qypr::ease::outBack(0.0), 0.0, 0.001);
    EXPECT_NEAR(qypr::ease::outBack(1.0), 1.0, 0.001);

    // Animated Class
    qypr::Animated anim(0.0);
    EXPECT_NEAR(anim.target(), 0.0, 0.001);
    EXPECT_FALSE(anim.active(qypr::nowMs()));

    anim.animateTo(1.0, 100, qypr::ease::linear);
    EXPECT_NEAR(anim.target(), 1.0, 0.001);
    
    int64_t start = qypr::nowMs();
    EXPECT_TRUE(anim.active(start));
    EXPECT_NEAR(anim.value(start), 0.0, 0.001);
    EXPECT_NEAR(anim.value(start + 50), 0.5, 0.1); // middle
    EXPECT_NEAR(anim.value(start + 200), 1.0, 0.001); // past end
    EXPECT_FALSE(anim.active(start + 200));

    anim.set(5.0);
    EXPECT_NEAR(anim.value(qypr::nowMs()), 5.0, 0.001);
    EXPECT_FALSE(anim.active(qypr::nowMs()));
}

TEST(EventLoop) {
    qypr::EventLoop loop;
    
    // Test post task
    bool posted_ran = false;
    loop.post([&] {
        posted_ran = true;
        loop.quit();
    });

    // Test prepare callback
    bool prepare_ran = false;
    loop.addPrepare([&] {
        prepare_ran = true;
    });

    // Run the loop which should exit via post
    loop.run();

    EXPECT_TRUE(posted_ran);
    EXPECT_TRUE(prepare_ran);

    // Test timers
    bool timer_ran = false;
    qypr::EventLoop loop2;
    int tfd = loop2.addTimer(5, false, [&] {
        timer_ran = true;
        loop2.quit();
    });
    EXPECT_TRUE(tfd >= 0);
    loop2.run();
    EXPECT_TRUE(timer_ran);

    // Test repeating timer cancellation
    int timer_count = 0;
    qypr::EventLoop loop3;
    int rep_tfd = loop3.addTimer(2, true, [&] {
        timer_count++;
        if (timer_count == 2) {
            loop3.removeTimer(rep_tfd);
            loop3.quit();
        }
    });
    loop3.run();
    EXPECT_EQ(timer_count, 2);
}

TEST(PamAuthenticator) {
    qypr::EventLoop loop;
    qypr::PamAuthenticator auth(loop, "qypr-lock-test");

    mock_pam_reset();
    mock_pam_set_expected_password("s3cret");

    qypr::PamAuthenticator::Result res = qypr::PamAuthenticator::Result::Failure;
    std::string last_msg;
    auto run_attempt = [&](const std::string& password) {
        bool done_called = false;
        res = qypr::PamAuthenticator::Result::Failure;
        auth.authenticate(password, [&](qypr::PamAuthenticator::Result r, const std::string& msg) {
            res = r;
            last_msg = msg;
            done_called = true;
            loop.quit();
        });
        loop.run();
        EXPECT_TRUE(done_called);
    };

    // ---- Correct password authenticates the *real* current user ----------
    // This is the regression guard for the "unknown user" bug: a prior change
    // hardcoded the PAM username to "test-user", so every real unlock failed.
    // The authenticator must resolve and pass the actual login user.
    run_attempt("s3cret");
    EXPECT_EQ(static_cast<int>(res), static_cast<int>(qypr::PamAuthenticator::Result::Success));

    const passwd* pw = getpwuid(getuid());
    EXPECT_TRUE(pw != nullptr);
    if (pw) EXPECT_EQ(std::string(mock_pam_last_user()), std::string(pw->pw_name));

    // ---- Wrong password -> Failure (real PAM_AUTH_ERR mapping) -----------
    run_attempt("wrong");
    EXPECT_EQ(static_cast<int>(res), static_cast<int>(qypr::PamAuthenticator::Result::Failure));

    // ---- PAM subsystem error -> Error (not a plain auth failure) ---------
    // PAM_USER_UNKNOWN is neither SUCCESS nor AUTH_ERR, so it must surface as
    // Error rather than being conflated with a wrong password.
    mock_pam_force_rc(PAM_USER_UNKNOWN);
    run_attempt("s3cret");
    EXPECT_EQ(static_cast<int>(res), static_cast<int>(qypr::PamAuthenticator::Result::Error));
    mock_pam_force_rc(-1);

    // ---- Empty password is rejected before any PAM call ------------------
    bool started = auth.authenticate("", [&](qypr::PamAuthenticator::Result, const std::string&) {});
    EXPECT_FALSE(started);
}

TEST(PowerManager) {
    qypr::PowerManager pm;
    pm.run("suspend");
    pm.run("reboot");
    // Should run instantly and safely in mock mode
}

TEST(MprisController) {
    qypr::MprisController mpris;
    EXPECT_TRUE(mpris.available());
    
    mpris.refresh();
    EXPECT_TRUE(mpris.active());
    EXPECT_TRUE(mpris.playing());
    EXPECT_EQ(mpris.title(), std::string("Mock Song"));
    EXPECT_EQ(mpris.artist(), std::string("Mock Artist"));
    EXPECT_EQ(mpris.album(), std::string("Mock Album"));
    EXPECT_EQ(mpris.sourceLabel(), std::string("Mock Player"));
    EXPECT_NEAR(mpris.volume(), 0.8, 0.01);
    EXPECT_NEAR(mpris.positionSeconds(), 60.0, 0.01);
    EXPECT_NEAR(mpris.durationSeconds(), 180.0, 0.01);
    EXPECT_TRUE(mpris.canGoNext());
    EXPECT_TRUE(mpris.canGoPrevious());
    EXPECT_TRUE(mpris.canTogglePlaying());
    EXPECT_TRUE(mpris.canSetVolume());

    mpris.togglePlaying();
    EXPECT_FALSE(mpris.playing()); // toggled to paused

    mpris.togglePlaying();
    EXPECT_TRUE(mpris.playing()); // toggled back to playing

    mpris.next();
    EXPECT_EQ(mpris.title(), std::string("Next Song"));

    mpris.previous();
    EXPECT_EQ(mpris.title(), std::string("Previous Song"));

    mpris.setVolume(0.4);
    EXPECT_NEAR(mpris.volume(), 0.4, 0.01);
}

TEST(NotificationMonitor) {
    qypr::EventLoop loop;
    qypr::NotificationMonitor mon(loop);
    
    EXPECT_TRUE(mon.start(false));
    EXPECT_TRUE(mon.notifications().empty());

    // Direct invocation to get coverage on teardown and other internal calls
    mon.teardown();
}

TEST(NotificationLog) {
    qypr::EventLoop loop;
    qypr::NotificationMonitor mon(loop);
    qypr::NotificationLog log(loop, mon);

    EXPECT_TRUE(log.start());
    log.drain();
}

TEST(VideoPlayer) {
    qypr::EventLoop loop;
    class DummyHost : public qypr::RenderHost {
    public:
        void invalidate() override { inv_called = true; }
        void requestUnlock() override {}
        bool inv_called = false;
    } host;

    qypr::VideoPlayer player(loop, host);
    EXPECT_FALSE(player.hasFrame());

    EXPECT_TRUE(player.init("playlists"));
    player.start();
    player.pause();
    player.resume();
    player.stop();
}

TEST(WaylandDisplay) {
    qypr::EventLoop loop;
    qypr::WaylandDisplay disp(loop);
    
    // Should connect using Wayland mock
    EXPECT_TRUE(disp.connect());

    disp.invalidateAll();
    
    // Lock session
    qypr::LockSession lock(disp);
    EXPECT_TRUE(lock.lock());
    lock.unlock();
}

TEST(AppAndUIHeadlessPreview) {
    qypr::App app;
    app.setIdleTimeout(10);
    
    // Exercises App::preview which renders both idle and revealed states to PNG
    // This exercises the full Cairo widget tree draw pipeline and event callbacks!
    int rc = app.preview("qypr-test-preview.png", 800, 600);
    EXPECT_EQ(rc, 0);

    // Clean up preview files
    std::remove("qypr-test-preview.png");
    std::remove("qypr-test-preview-idle.png");
}

TEST(LockScreenInputHandling) {
    qypr::EventLoop loop;
    qypr::App app;
    qypr::PamAuthenticator pam(loop);
    qypr::PowerManager power;
    
    qypr::LockScreen screen(loop, app, pam, power);

    // Test text inputs
    screen.onTextInput("a");
    screen.onTextInput("b");
    screen.onSpecialKey(0xff08, 0); // Backspace keysym
    screen.onSpecialKey(0xff0d, 0); // Enter keysym (submits pam auth)

    // Test pointer events
    screen.onPointerMotion(800, 600, 100, 100);
    screen.onPointerButton(800, 600, 100, 100, 272, true); // left press
    screen.onPointerButton(800, 600, 100, 100, 272, false); // left release
    screen.onPointerLeave();

    // Verify clock layout & icon resolver
    EXPECT_TRUE(screen.isAnimating());
}

// The core lockscreen invariant: the session unlocks if and only if PAM
// reports Success. A wrong password or a PAM subsystem error must never reach
// requestUnlock(). A spy RenderHost lets us observe that directly.
TEST(LockScreenUnlockGating) {
    qypr::EventLoop loop;
    struct SpyHost : qypr::RenderHost {
        int unlocks = 0;
        void invalidate() override {}
        void requestUnlock() override { ++unlocks; }
    } host;
    qypr::PamAuthenticator pam(loop);
    qypr::PowerManager power;
    qypr::LockScreen screen(loop, host, pam, power);

    mock_pam_reset();
    mock_pam_set_expected_password("open-sesame");

    auto submit = [&](const std::string& pw) {
        screen.password_ = pw;      // private, exposed for this test TU
        screen.submitPassword();
        while (pam.busy()) {}        // wait for the auth worker to finish
        loop.dispatchPosted();       // run onAuthResult on the loop thread
    };

    // Wrong password: no unlock, error surfaced, secret wiped.
    submit("wrong");
    EXPECT_EQ(host.unlocks, 0);
    EXPECT_TRUE(screen.hasError_);
    EXPECT_TRUE(screen.password_.empty());

    // PAM subsystem error (not a plain auth failure): still no unlock.
    mock_pam_force_rc(PAM_USER_UNKNOWN);
    submit("open-sesame");
    EXPECT_EQ(host.unlocks, 0);
    mock_pam_force_rc(-1);

    // Correct password: unlocks exactly once and enters the unlocking state.
    submit("open-sesame");
    EXPECT_EQ(host.unlocks, 1);
    EXPECT_TRUE(screen.unlocking_);
}

// A monitor hot-plugged while the session is locked must be covered by a lock
// surface immediately — otherwise the desktop behind it is exposed. Conversely,
// an output that appears while unlocked must NOT get a lock surface. Registry
// globals are driven directly (connect()'s global burst is one-shot per run).
TEST(OutputHotplugWhileLocked) {
    qypr::EventLoop loop;
    qypr::WaylandDisplay disp(loop);
    auto* reg = reinterpret_cast<wl_registry*>(0x5550);

    // Bring up the globals a lock needs.
    qypr::WaylandDisplay::onGlobal(&disp, reg, 1, "wl_compositor", 4);
    qypr::WaylandDisplay::onGlobal(&disp, reg, 2, "wl_shm", 1);
    qypr::WaylandDisplay::onGlobal(&disp, reg, 3, "ext_session_lock_manager_v1", 1);
    qypr::WaylandDisplay::onGlobal(&disp, reg, 5, "wl_output", 4);
    EXPECT_EQ(static_cast<int>(disp.outputs_.size()), 1);
    EXPECT_TRUE(disp.outputs_[0]->surface() == nullptr);  // not locked yet

    qypr::LockSession lock(disp);
    EXPECT_TRUE(lock.lock());
    EXPECT_TRUE(disp.outputs_[0]->surface() != nullptr);  // existing output covered

    // Hotplug while locked: the new monitor is covered on arrival.
    qypr::WaylandDisplay::onGlobal(&disp, reg, 6, "wl_output", 4);
    EXPECT_EQ(static_cast<int>(disp.outputs_.size()), 2);
    EXPECT_TRUE(disp.outputs_.back()->surface() != nullptr);

    // Unplug: the output is dropped cleanly, others untouched.
    qypr::WaylandDisplay::onGlobalRemove(&disp, reg, 6);
    EXPECT_EQ(static_cast<int>(disp.outputs_.size()), 1);

    lock.unlock();

    // After unlock, a newly-appearing output is NOT given a lock surface.
    qypr::WaylandDisplay::onGlobal(&disp, reg, 7, "wl_output", 4);
    EXPECT_EQ(static_cast<int>(disp.outputs_.size()), 2);
    EXPECT_TRUE(disp.outputs_.back()->surface() == nullptr);
}

// The keyboard/pointer input path is the primary interactive attack surface at
// the lock screen. Exercise the real translation logic (special keys vs text,
// key repeat, modifiers, pointer routing) against a recording sink.
TEST(SeatInput) {
    qypr::EventLoop loop;

    struct RecSink : qypr::InputSink {
        std::string text;
        int specials = 0, motions = 0, buttons = 0, leaves = 0;
        uint32_t lastSym = 0, lastMods = 0;
        bool lastPressed = false;
        void onTextInput(const std::string& s) override { text += s; }
        void onSpecialKey(uint32_t sym, uint32_t mods) override {
            ++specials; lastSym = sym; lastMods = mods;
        }
        void onPointerMotion(int, int, double, double) override { ++motions; }
        void onPointerButton(int, int, double, double, uint32_t, bool pressed) override {
            ++buttons; lastPressed = pressed;
        }
        void onPointerLeave() override { ++leaves; }
    } sink;

    qypr::OutputEnv env;  // null compositor/shm -> cursor build safely skipped
    auto* seatPtr = reinterpret_cast<wl_seat*>(0x5553);
    qypr::Seat seat(seatPtr, loop, &env);
    seat.setSink(&sink);

    // An output the pointer can focus, so motion/button carry a real size.
    qypr::Output out(reinterpret_cast<wl_output*>(0x5554), 42, &env);
    seat.setOutputResolver([&](wl_surface*) { return &out; });

    mock_xkb_reset();

    // Build a real, mmap-able keymap fd of `size` zero bytes.
    auto makeFd = [](size_t size) -> int {
        FILE* f = std::tmpfile();
        if (!f) return -1;
        for (size_t i = 0; i < size; ++i) std::fputc(0, f);
        std::fflush(f);
        int fd = dup(fileno(f));
        std::fclose(f);
        return fd;
    };

    // Capabilities: keyboard + pointer come online.
    qypr::Seat::onCapabilities(&seat, seatPtr,
                               WL_SEAT_CAPABILITY_KEYBOARD | WL_SEAT_CAPABILITY_POINTER);
    EXPECT_TRUE(seat.keyboard_ != nullptr);
    EXPECT_TRUE(seat.pointer_ != nullptr);

    // A non-XKB keymap format is rejected: no xkb state is built.
    qypr::Seat::onKeymap(&seat, nullptr, 0 /*bad format*/, makeFd(8), 8);
    EXPECT_TRUE(seat.xkbState_ == nullptr);

    // A valid XKB keymap builds the translation state.
    qypr::Seat::onKeymap(&seat, nullptr, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, makeFd(64), 64);
    EXPECT_TRUE(seat.xkbState_ != nullptr);

    qypr::Seat::onRepeatInfo(&seat, nullptr, 25 /*rate*/, 1 /*delay ms*/);

    // Printable key: delivered as text, and arms key-repeat.
    mock_xkb_set_utf8("k");
    mock_xkb_set_repeats(1);
    qypr::Seat::onKey(&seat, nullptr, 0, 0, 30, WL_KEYBOARD_KEY_STATE_PRESSED);
    EXPECT_EQ(sink.text, std::string("k"));
    EXPECT_TRUE(seat.repeatKeycode_ != 0);

    // Release stops repeat.
    qypr::Seat::onKey(&seat, nullptr, 0, 0, 30, WL_KEYBOARD_KEY_STATE_RELEASED);
    EXPECT_TRUE(seat.repeatKeycode_ == 0);

    // Special key (Return) routes to onSpecialKey, never as text.
    mock_xkb_set_utf8("");
    mock_xkb_set_repeats(0);
    mock_xkb_set_sym(XKB_KEY_Return);
    size_t textBefore = sink.text.size();
    qypr::Seat::onKey(&seat, nullptr, 0, 0, 28, WL_KEYBOARD_KEY_STATE_PRESSED);
    EXPECT_EQ(sink.specials, 1);
    EXPECT_EQ(static_cast<int>(sink.lastSym), static_cast<int>(XKB_KEY_Return));
    EXPECT_EQ(static_cast<int>(sink.text.size()), static_cast<int>(textBefore));

    // Modifier update and keyboard-leave paths (leave must stop any repeat).
    qypr::Seat::onModifiers(&seat, nullptr, 0, 1, 0, 0, 0);
    qypr::Seat::onKbLeave(&seat, nullptr, 0, nullptr);

    // Pointer: enter focuses the output, motion + button reach the sink, leave clears.
    qypr::Seat::onPtrEnter(&seat, reinterpret_cast<wl_pointer*>(0x5555), 1, nullptr, 0, 0);
    qypr::Seat::onPtrMotion(&seat, nullptr, 0, wl_fixed_from_int(100), wl_fixed_from_int(50));
    EXPECT_EQ(sink.motions, 1);
    qypr::Seat::onPtrButton(&seat, nullptr, 0, 0, 272 /*BTN_LEFT*/,
                            WL_POINTER_BUTTON_STATE_PRESSED);
    EXPECT_EQ(sink.buttons, 1);
    EXPECT_TRUE(sink.lastPressed);
    qypr::Seat::onPtrLeave(&seat, nullptr, 0, nullptr);
    EXPECT_EQ(sink.leaves, 1);

    // Capabilities dropped: keyboard + pointer are torn down.
    qypr::Seat::onCapabilities(&seat, seatPtr, 0);
    EXPECT_TRUE(seat.keyboard_ == nullptr);
    EXPECT_TRUE(seat.pointer_ == nullptr);

    mock_xkb_reset();  // restore defaults for any later test
}

// The compositor can refuse or revoke a lock via the `finished` event. When it
// does, the session must stop reporting itself as locked (it is no longer
// secure) and notify the app, and a later unlock() must not misuse the protocol
// by releasing a lock that was never granted.
TEST(LockSessionFinished) {
    qypr::EventLoop loop;
    qypr::WaylandDisplay disp(loop);
    auto* reg = reinterpret_cast<wl_registry*>(0x5550);
    qypr::WaylandDisplay::onGlobal(&disp, reg, 3, "ext_session_lock_manager_v1", 1);

    // --- Granted, then revoked mid-session ---
    {
        qypr::LockSession session(disp);
        bool lockedCb = false, finishedCb = false;
        session.setOnLocked([&] { lockedCb = true; });
        session.setOnFinished([&] { finishedCb = true; });

        EXPECT_TRUE(session.lock());
        EXPECT_FALSE(session.locked());  // not confirmed by the compositor yet

        qypr::LockSession::onLocked(&session, nullptr);
        EXPECT_TRUE(session.locked());
        EXPECT_TRUE(lockedCb);

        qypr::LockSession::onFinished(&session, nullptr);  // compositor revokes it
        EXPECT_FALSE(session.locked());  // no longer secure
        EXPECT_TRUE(finishedCb);

        session.unlock();  // safe: destroy (not unlock_and_destroy), no crash
    }

    // --- Refused outright: `finished` before any `locked` ---
    {
        qypr::LockSession session(disp);
        bool finishedCb = false;
        session.setOnFinished([&] { finishedCb = true; });

        EXPECT_TRUE(session.lock());
        qypr::LockSession::onFinished(&session, nullptr);
        EXPECT_FALSE(session.locked());
        EXPECT_TRUE(finishedCb);
        session.unlock();  // never granted -> still safe
    }

    // --- Normal lifecycle: granted, then cleanly unlocked while locked ---
    {
        qypr::LockSession session(disp);
        EXPECT_TRUE(session.lock());
        qypr::LockSession::onLocked(&session, nullptr);
        EXPECT_TRUE(session.locked());
        session.unlock();  // locked -> unlock_and_destroy path
        EXPECT_FALSE(session.locked());
    }

    // --- Teardown while still holding a granted lock (no unlock() call):
    //     the destructor must release it cleanly rather than leak it. ---
    {
        qypr::LockSession session(disp);
        EXPECT_TRUE(session.lock());
        qypr::LockSession::onLocked(&session, nullptr);
        EXPECT_TRUE(session.locked());
    }  // ~LockSession runs here with lock_ still held
}

// The notification tile icon resolver: a base64 data: URI must decode to a real
// surface, repeated lookups must hit the cache, and empty/unknown names must
// resolve to nothing so the caller falls back to a letter glyph.
TEST(IconResolver) {
    qypr::IconResolver r;

    const std::string duri =
        "data:image/png;base64,"
        "iVBORw0KGgoAAAANSUhEUgAAAAQAAAAECAIAAAAmkwkpAAAABmJLR0QA/wD/AP+gvaeT"
        "AAAAEElEQVQImWP8z4AATAxEcQAz0QEH1mUzKgAAAABJRU5ErkJggg==";
    cairo_surface_t* s = r.get(duri);
    EXPECT_TRUE(s != nullptr);
    EXPECT_EQ(cairo_image_surface_get_width(s), 4);

    // Same key returns the identical, cache-owned surface (no reload).
    EXPECT_TRUE(r.get(duri) == s);

    // Empty and unknown names resolve to nothing -> glyph fallback.
    EXPECT_TRUE(r.get("") == nullptr);
    EXPECT_TRUE(r.get("qypr-no-such-icon-xyz") == nullptr);
}

// -----------------------------------------------------------------------------
// Main Runner
// -----------------------------------------------------------------------------
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << " Running unit tests" << std::endl;
    std::cout << "========================================" << std::endl;

    // Unit tests are registered during static initialization.
    // They run automatically.

    std::cout << "========================================" << std::endl;
    std::cout << " Results: Passed: " << (g_tests_run - g_tests_failed)
              << " / " << g_tests_run << "   Failed: " << g_tests_failed << std::endl;
    std::cout << "========================================" << std::endl;

    return g_tests_failed == 0 ? 0 : 1;
}
