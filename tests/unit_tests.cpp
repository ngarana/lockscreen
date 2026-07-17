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
#include <fstream>
#include <cstdlib>

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
#include "ui/statusbar/StatusBar.hpp"
#include "ui/statusbar/StatusIndicator.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"
#include "ui/statusbar/QSTile.hpp"
#include "ui/statusbar/QuickSettingsPanel.hpp"
#include "ui/statusbar/PopoverManager.hpp"
#include "ui/statusbar/DetailedPopover.hpp"
#include "core/Config.hpp"
#include "ui/indicators/ClockIndicator.hpp"
#include "ui/indicators/NotificationIndicator.hpp"
#include "ui/indicators/PowerMenuIndicator.hpp"
#include "power/PowerManager.hpp"
#include "ui/indicators/BatteryIndicator.hpp"
#include "system/BatteryBackend.hpp"
#include "system/BrightnessBackend.hpp"
#include "system/SystemBus.hpp"
#include "system/BluetoothBackend.hpp"
#include "system/WifiBackend.hpp"
#include "system/DndState.hpp"
#include "system/VolumeBackend.hpp"
#include "system/SNIBackend.hpp"
#include "system/WorkspaceBackend.hpp"
#include "system/ToplevelBackend.hpp"
#include "ui/indicators/BluetoothIndicator.hpp"
#include "ui/indicators/BrightnessIndicator.hpp"
#include "ui/indicators/DNDIndicator.hpp"
#include "ui/indicators/VolumeIndicator.hpp"
#include "ui/indicators/WifiIndicator.hpp"
#include "ui/indicators/SNITrayHost.hpp"
#include "ui/indicators/WorkspacesIndicator.hpp"
#include "ui/indicators/ActiveWindowIndicator.hpp"
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
    screen.handleTextInput("a");
    screen.handleTextInput("b");
    screen.handleSpecialKey(0xff08, 0); // Backspace keysym
    screen.handleSpecialKey(0xff0d, 0); // Enter keysym (submits pam auth)

    // Test pointer events
    screen.handlePointerMotion(800, 600, 100, 100);
    screen.handlePointerButton(800, 600, 100, 100, 272, true); // left press
    screen.handlePointerButton(800, 600, 100, 100, 272, false); // left release
    screen.handlePointerLeave();

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
    seat.setSurfaceSizer([&](wl_surface*, int& w, int& h) {
        w = out.logicalWidth();
        h = out.logicalHeight();
        return true;
    });

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

    // Pointer: enter focuses a surface (the sizer resolves it), motion + button
    // reach the sink, leave clears.
    qypr::Seat::onPtrEnter(&seat, reinterpret_cast<wl_pointer*>(0x5555), 1,
                           reinterpret_cast<wl_surface*>(0x5556), 0, 0);
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

// =============================================================================
// Phase 1: Status Bar Framework Tests
// =============================================================================

// A concrete test indicator for exercising the base class interface.
class TestIndicator : public qypr::StatusIndicator {
public:
    TestIndicator(const std::string& id, qypr::Zone zone, int priority)
        : StatusIndicator(id, zone, priority) {}

    std::string icon() const override { return icon_; }
    std::string tooltip() const override { return tooltip_; }
    qypr::Color iconColor() const override { return qypr::theme::color::primary; }

    std::string icon_ = "T";
    std::string tooltip_ = "Test Indicator";
};

// Test that the IndicatorRegistry can register and create indicators.
TEST(IndicatorRegistryRegisterAndCreate) {
    auto& reg = qypr::IndicatorRegistry::instance();
    size_t before = reg.entries_.size();

    reg.registerIndicator("test-indicator", qypr::Zone::Right, 999,
        [](const qypr::SystemBackends&) {
            return std::make_unique<TestIndicator>("test-indicator", qypr::Zone::Right, 999);
        });

    EXPECT_EQ(reg.entries_.size(), before + 1);

    qypr::SystemBackends backends{};
    auto items = reg.createAll(backends);
    bool found = false;
    for (auto& item : items) {
        if (item->id() == "test-indicator") {
            found = true;
            EXPECT_TRUE(static_cast<int>(item->zone()) == static_cast<int>(qypr::Zone::Right));
            EXPECT_EQ(item->priority(), 999);
        }
    }
    EXPECT_TRUE(found);
}

// Test StatusIndicator base class getters and state.
TEST(StatusIndicatorBaseClass) {
    TestIndicator ind("my-ind", qypr::Zone::Left, 100);

    EXPECT_EQ(ind.id(), std::string("my-ind"));
    EXPECT_EQ(static_cast<int>(ind.zone()), static_cast<int>(qypr::Zone::Left));
    EXPECT_EQ(ind.priority(), 100);
    EXPECT_FALSE(ind.hovered);
    EXPECT_FALSE(ind.focused);
    EXPECT_FALSE(ind.hasDetailedView());
    EXPECT_TRUE(ind.createTile() == nullptr);
    EXPECT_TRUE(ind.createDetailedView() == nullptr);
}

// Test QSToggleTile renders without crashing on a null Cairo context.
TEST(QSToggleTileDraw) {
    qypr::QSToggleTile tile("WiFi", "󰤨",
        []() { return true; },
        []() {},
        []() { return std::string("MyHome"); });

    EXPECT_TRUE(tile.type() == qypr::QSTile::Type::Toggle);
    EXPECT_TRUE(tile.bounds.valid() == false);  // not yet laid out

    // Draw to a null painter — exercises the code path without a real surface.
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 200, 100);
    cairo_t* cr = cairo_create(surf);
    qypr::Painter p(cr);
    tile.bounds = {0, 0, 110, 64};
    tile.draw(p, 1000);

    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}

// Test QSSliderTile renders and handles drag input.
TEST(QSSliderTileDrawAndDrag) {
    double vol = 0.5;
    qypr::QSSliderTile tile("󰕾",
        [&]() { return vol; },
        [&](double v) { vol = v; });

    EXPECT_TRUE(tile.type() == qypr::QSTile::Type::Slider);

    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 400, 60);
    cairo_t* cr = cairo_create(surf);
    qypr::Painter p(cr);
    tile.bounds = {0, 0, 360, 40};
    tile.draw(p, 2000);

    // Simulate click/drag on the slider track to change value.
    tile.onClick(tile.bounds.x + tile.bounds.w * 0.75, tile.bounds.y + tile.bounds.h / 2.0);
    EXPECT_NEAR(vol, 0.75, 0.1);

    tile.onDrag(tile.bounds.x + tile.bounds.w * 0.25, tile.bounds.y + tile.bounds.h / 2.0);
    EXPECT_NEAR(vol, 0.25, 0.1);

    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}

// Test QuickSettingsPanel layout and tile aggregation.
TEST(QuickSettingsPanelLayout) {
    qypr::QuickSettingsPanel panel;

    panel.addTile(std::make_unique<qypr::QSToggleTile>("WiFi", "󰤨",
        []() { return true; }, []() {}));
    panel.addTile(std::make_unique<qypr::QSToggleTile>("BT", "󰂯",
        []() { return false; }, []() {}));
    panel.addTile(std::make_unique<qypr::QSSliderTile>("󰕾",
        []() { return 0.6; }, [](double) {}));

    panel.anchorX = 800;
    panel.anchorY = 50;

    EXPECT_TRUE(!panel.isOpen());

    // Draw the panel to exercise layout logic and populate tile bounds.
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 400, 300);
    cairo_t* cr = cairo_create(surf);
    qypr::Painter p(cr);
    panel.draw(p, 3000);
    double h = panel.contentHeight();
    EXPECT_TRUE(h > 0);

    // Click inside the first toggle tile (bounds set by draw/layoutTiles).
    qypr::Rect pb = panel.getBounds();
    double firstTileX = pb.x + 16.0 + 5.0;  // pad + small offset inside tile
    double firstTileY = pb.y + 16.0 + 5.0;
    EXPECT_TRUE(panel.handleClick(firstTileX, firstTileY));

    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}

// Test PopoverManager open/close lifecycle.
TEST(PopoverManagerLifecycle) {
    qypr::PopoverManager pm;
    EXPECT_TRUE(pm.active() == nullptr);

    // DetailedPopover is abstract; use a minimal concrete subclass.
    struct TestPopover : qypr::DetailedPopover {
        void draw(qypr::Painter&, int64_t) override {}
        double contentHeight() const override { return 100.0; }
    };

    auto pop = std::make_unique<TestPopover>();
    qypr::DetailedPopover* raw = pop.get();

    pm.open(std::move(pop), 100, 50);
    EXPECT_TRUE(pm.active() == raw);
    EXPECT_TRUE(pm.active()->isOpen());

    pm.closeActive();
    EXPECT_TRUE(pm.active() == nullptr);
}

// Test that StatusBar constructs, lays out, and draws without crashing.
TEST(StatusBarConstructionAndDraw) {
    qypr::EventLoop loop;

    struct DummyHost : qypr::RenderHost {
        int invalidations = 0;
        void invalidate() override { ++invalidations; }
        void requestUnlock() override {}
    } host;

    qypr::SystemBackends backends{};
    qypr::StatusBar bar(loop, host, backends);

    // Layout at 1920x1080
    bar.layout(1920, 1080);
    EXPECT_TRUE(bar.bounds.w > 0);
    EXPECT_TRUE(bar.bounds.h > 0);

    // Draw to a real Cairo surface
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1920, 1080);
    cairo_t* cr = cairo_create(surf);
    qypr::Painter p(cr);
    bar.draw(p, 4000);

    // Second draw to verify no state corruption
    bar.draw(p, 5000);

    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}

// Test StatusBar pointer input routing (hover, click, scroll).
TEST(StatusBarPointerInput) {
    qypr::EventLoop loop;
    struct DummyHost : qypr::RenderHost {
        void invalidate() override {}
        void requestUnlock() override {}
    } host;

    qypr::SystemBackends backends{};
    qypr::StatusBar bar(loop, host, backends);
    bar.layout(1920, 1080);

    // Motion inside the bar
    bool handled = bar.handlePointerMotion(bar.bounds.x + 50, bar.bounds.y + 10, 1000);
    EXPECT_TRUE(handled);

    // Motion outside the bar
    handled = bar.handlePointerMotion(0, 0, 1001);
    EXPECT_FALSE(handled);

    // Click on the gear button area
    double gearX = bar.bounds.x + bar.bounds.w - 30;
    double gearY = bar.bounds.y + bar.bounds.h / 2.0;
    handled = bar.handlePointerButton(gearX, gearY, 272, true, 1002);
    EXPECT_TRUE(handled);

    // Leave clears hover
    bar.handlePointerLeave(1003);
    EXPECT_FALSE(bar.qsButtonHovered_);
}

// Test StatusBar keyboard focus cycling.
TEST(StatusBarFocusCycling) {
    qypr::EventLoop loop;
    struct DummyHost : qypr::RenderHost {
        void invalidate() override {}
        void requestUnlock() override {}
    } host;

    qypr::SystemBackends backends{};
    qypr::StatusBar bar(loop, host, backends);
    bar.layout(1920, 1080);

    // cycleFocus on a bar with registered indicators should succeed
    // (the TestIndicator from IndicatorRegistryRegisterAndCreate persists in the singleton)
    bool changed = bar.cycleFocus(false);
    // Focus should have moved to the first indicator
    EXPECT_TRUE(changed);

    // Clear and verify no crash
    bar.clearFocus();
}

// Test StatusBar animating() detects active animations.
TEST(StatusBarAnimating) {
    qypr::EventLoop loop;
    struct DummyHost : qypr::RenderHost {
        void invalidate() override {}
        void requestUnlock() override {}
    } host;

    qypr::SystemBackends backends{};
    qypr::StatusBar bar(loop, host, backends);
    bar.layout(1920, 1080);

    // Initially not animating
    EXPECT_TRUE(!bar.animating(10000));

    // Hover over an indicator and draw to trigger its hover animation
    if (!bar.rightIndicators_.empty()) {
        auto& ind = bar.rightIndicators_.front();
        bar.handlePointerMotion(ind->bounds.x + 5, ind->bounds.y + 5, 10001);
        cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1920, 1080);
        cairo_t* cr = cairo_create(surf);
        qypr::Painter p(cr);
        bar.draw(p, 10001);  // draw triggers the hover animation on indicators
        EXPECT_TRUE(bar.animating(10001));
        cairo_destroy(cr);
        cairo_surface_destroy(surf);
    }
}

// Test theme::statusbar constants are accessible.
TEST(StatusBarThemeConstants) {
    using namespace qypr::theme::statusbar;
    EXPECT_TRUE(height > 0);
    EXPECT_TRUE(topMargin >= 0);
    EXPECT_TRUE(sideMargin >= 0);
    EXPECT_TRUE(cornerRadius > 0);
    EXPECT_TRUE(iconSize > 0);
    EXPECT_TRUE(iconSpacing > 0);
    EXPECT_TRUE(padding > 0);
    EXPECT_TRUE(qsPanelWidth > 0);
    EXPECT_TRUE(qsTileHeight > 0);
}

// =============================================================================
// Phase 2: Clock + Battery Indicator Tests
// =============================================================================

TEST(ClockIndicatorConstruction) {
    qypr::SystemBackends backends{};
    qypr::ClockIndicator clock(backends);

    EXPECT_EQ(clock.id(), std::string("clock"));
    EXPECT_TRUE(static_cast<int>(clock.zone()) == static_cast<int>(qypr::Zone::Left));
    EXPECT_EQ(clock.priority(), 0);
    EXPECT_TRUE(clock.createTile() == nullptr);  // no Quick Settings tile
    // The clock drops a calendar popover (Phase 12).
    EXPECT_TRUE(clock.hasDetailedView());
    EXPECT_TRUE(clock.createDetailedView() != nullptr);
}

TEST(ClockIndicatorRenders) {
    qypr::SystemBackends backends{};
    qypr::ClockIndicator clock(backends);

    // Poll to populate cached time
    clock.poll(qypr::nowMs());

    // Text-only indicator: label carries the formatted time, icon is empty
    std::string t = clock.label();
    EXPECT_TRUE(!t.empty());
    EXPECT_TRUE(clock.icon().empty());

    // Tooltip returns date string
    std::string d = clock.tooltip();
    EXPECT_TRUE(!d.empty());

    // Measure width against a real painter
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 400, 60);
    cairo_t* cr = cairo_create(surf);
    qypr::Painter p(cr);
    double w = clock.measureWidth(p);
    EXPECT_TRUE(w > 0);

    // Draw
    clock.bounds = {0, 0, w, 36};
    clock.draw(p, qypr::nowMs());

    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}

TEST(BatteryIndicatorConstruction) {
    qypr::SystemBackends backends{};
    qypr::BatteryIndicator batt(backends);

    EXPECT_EQ(batt.id(), std::string("battery"));
    EXPECT_TRUE(static_cast<int>(batt.zone()) == static_cast<int>(qypr::Zone::Right));
    EXPECT_EQ(batt.priority(), 500);
    EXPECT_TRUE(batt.hasDetailedView());
}

TEST(BatteryIndicatorCreatesInfoTile) {
    qypr::SystemBackends backends{};
    qypr::BatteryIndicator batt(backends);

    auto tile = batt.createTile();
    EXPECT_TRUE(tile != nullptr);
    EXPECT_TRUE(tile->type() == qypr::QSTile::Type::Info);

    // Draw the tile
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 200, 80);
    cairo_t* cr = cairo_create(surf);
    qypr::Painter p(cr);
    tile->bounds = {0, 0, 180, 64};
    tile->draw(p, qypr::nowMs());
    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}

TEST(BatteryIndicatorColorCoding) {
    qypr::SystemBackends backends{};
    qypr::BatteryIndicator batt(backends);

    // Default state: 0%, Unknown
    qypr::Color c = batt.iconColor();
    // Should be red for 0%
    EXPECT_TRUE(c.r > 0.5);  // red channel dominant

    // Backend update with no backend attached must be a no-op, not a crash
    batt.onBackendUpdate();

    // Measure should return positive width
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 200, 60);
    cairo_t* cr = cairo_create(surf);
    qypr::Painter p(cr);
    double w = batt.measureWidth(p);
    EXPECT_TRUE(w > 0);
    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}

TEST(BatteryBackendConstruction) {
    qypr::EventLoop loop;
    qypr::SystemBus bus(loop);
    qypr::BatteryBackend backend(bus);

    // start() must not crash whether or not UPower/a battery is available;
    // on failure the snapshot stays absent so the indicator hides.
    bool started = backend.start();
    const auto& s = backend.snapshot();
    if (started) {
        EXPECT_TRUE(s.present);
        EXPECT_TRUE(s.percentage >= 0 && s.percentage <= 100);
    } else {
        EXPECT_FALSE(s.present);
    }
}

// =============================================================================
// Phase 3a: Brightness Indicator Tests
// =============================================================================

TEST(BrightnessIndicatorConstruction) {
    qypr::SystemBackends backends{};
    qypr::BrightnessIndicator bright(backends);

    EXPECT_EQ(bright.id(), std::string("brightness"));
    EXPECT_TRUE(static_cast<int>(bright.zone()) == static_cast<int>(qypr::Zone::Right));
    EXPECT_EQ(bright.priority(), 100);
    EXPECT_FALSE(bright.hasDetailedView());

    // Icon levels track the snapshot fraction
    bright.lastSnap_.max = 100;
    bright.lastSnap_.current = 80;
    EXPECT_EQ(bright.icon(), std::string("󰃠"));
    bright.lastSnap_.current = 50;
    EXPECT_EQ(bright.icon(), std::string("󰃟"));
    bright.lastSnap_.current = 10;
    EXPECT_EQ(bright.icon(), std::string("󰃞"));

    // Scroll without a backend must be a no-op, not a crash
    EXPECT_FALSE(bright.onScroll(0, -1.0));
}

TEST(BrightnessIndicatorCreatesSliderTile) {
    qypr::SystemBackends backends{};
    qypr::BrightnessIndicator bright(backends);
    bright.lastSnap_.max = 100;
    bright.lastSnap_.current = 75;

    auto tile = bright.createTile();
    EXPECT_TRUE(tile != nullptr);
    EXPECT_TRUE(tile->type() == qypr::QSTile::Type::Slider);

    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 400, 60);
    cairo_t* cr = cairo_create(surf);
    qypr::Painter p(cr);
    tile->bounds = {0, 0, 348, 40};
    tile->draw(p, qypr::nowMs());
    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}

TEST(BrightnessBackendConstruction) {
    qypr::EventLoop loop;
    qypr::SystemBus bus(loop);
    qypr::BrightnessBackend backend(loop, bus);

    // start() must not crash whether or not a backlight exists; on failure
    // the snapshot stays unavailable so the indicator hides.
    bool started = backend.start();
    const auto& s = backend.snapshot();
    if (started) {
        EXPECT_TRUE(s.available);
        EXPECT_TRUE(s.max > 0);
        EXPECT_TRUE(s.current >= 0 && s.current <= s.max);
    } else {
        EXPECT_FALSE(s.available);
    }
}

// =============================================================================
// Phase 3b: WiFi Indicator Tests
// =============================================================================

TEST(WifiIndicatorConstruction) {
    qypr::SystemBackends backends{};
    qypr::WifiIndicator wifi(backends);

    EXPECT_EQ(wifi.id(), std::string("wifi"));
    EXPECT_TRUE(static_cast<int>(wifi.zone()) == static_cast<int>(qypr::Zone::Right));
    EXPECT_EQ(wifi.priority(), 300);

    // Icon tracks radio/connection/strength states
    wifi.lastSnap_.enabled = false;
    EXPECT_EQ(wifi.icon(), std::string("󰤮"));
    wifi.lastSnap_.enabled = true;
    wifi.lastSnap_.connected = false;
    EXPECT_EQ(wifi.icon(), std::string("󰤭"));
    wifi.lastSnap_.connected = true;
    wifi.lastSnap_.strength = 80;
    EXPECT_EQ(wifi.icon(), std::string("󰤨"));
    wifi.lastSnap_.strength = 60;
    EXPECT_EQ(wifi.icon(), std::string("󰤥"));
    wifi.lastSnap_.strength = 30;
    EXPECT_EQ(wifi.icon(), std::string("󰤢"));
    wifi.lastSnap_.strength = 10;
    EXPECT_EQ(wifi.icon(), std::string("󰤯"));
}

TEST(WifiIndicatorCreatesToggleTile) {
    qypr::SystemBackends backends{};
    qypr::WifiIndicator wifi(backends);
    wifi.lastSnap_.enabled = true;
    wifi.lastSnap_.connected = true;
    wifi.lastSnap_.ssid = "TestNet";

    auto tile = wifi.createTile();
    EXPECT_TRUE(tile != nullptr);
    EXPECT_TRUE(tile->type() == qypr::QSTile::Type::Toggle);

    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 200, 80);
    cairo_t* cr = cairo_create(surf);
    qypr::Painter p(cr);
    tile->bounds = {0, 0, 160, 64};
    tile->draw(p, qypr::nowMs());
    cairo_destroy(cr);
    cairo_surface_destroy(surf);

    // Toggling without a backend must be a no-op, not a crash
    tile->onClick(10, 10);
}

// =============================================================================
// Phase 3c: Bluetooth Indicator Tests
// =============================================================================

TEST(BluetoothIndicatorConstruction) {
    qypr::SystemBackends backends{};
    qypr::BluetoothIndicator bt(backends);

    EXPECT_EQ(bt.id(), std::string("bluetooth"));
    EXPECT_TRUE(static_cast<int>(bt.zone()) == static_cast<int>(qypr::Zone::Right));
    EXPECT_EQ(bt.priority(), 350);

    // Icon and accent track power/connection state
    bt.lastSnap_.powered = false;
    EXPECT_EQ(bt.icon(), std::string("󰂲"));
    bt.lastSnap_.powered = true;
    bt.lastSnap_.connectedCount = 0;
    EXPECT_EQ(bt.icon(), std::string("󰂯"));
    bt.lastSnap_.connectedCount = 1;
    bt.lastSnap_.firstDevice = "Headphones";
    EXPECT_EQ(bt.icon(), std::string("󰂱"));
    EXPECT_TRUE(bt.tooltip().find("Headphones") != std::string::npos);
    // Connected: blue accent, not the plain text color
    qypr::Color accent = bt.iconColor();
    EXPECT_TRUE(accent.b > accent.r);
}

TEST(BluetoothIndicatorCreatesToggleTile) {
    qypr::SystemBackends backends{};
    qypr::BluetoothIndicator bt(backends);
    bt.lastSnap_.powered = true;
    bt.lastSnap_.connectedCount = 2;

    auto tile = bt.createTile();
    EXPECT_TRUE(tile != nullptr);
    EXPECT_TRUE(tile->type() == qypr::QSTile::Type::Toggle);

    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 200, 80);
    cairo_t* cr = cairo_create(surf);
    qypr::Painter p(cr);
    tile->bounds = {0, 0, 160, 64};
    tile->draw(p, qypr::nowMs());
    cairo_destroy(cr);
    cairo_surface_destroy(surf);

    // Toggling without a backend must be a no-op, not a crash
    tile->onClick(10, 10);
}

// =============================================================================
// Phase 3d: DND Tests
// =============================================================================

TEST(DndStateToggleNotifiesAllListeners) {
    qypr::DndState dnd;
    int a = 0, b = 0;
    dnd.addListener([&] { ++a; });
    dnd.addListener([&] { ++b; });

    EXPECT_FALSE(dnd.enabled());
    dnd.toggle();
    EXPECT_TRUE(dnd.enabled());
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 1);

    // Setting the same value must not re-notify
    dnd.setEnabled(true);
    EXPECT_EQ(a, 1);

    dnd.setEnabled(false);
    EXPECT_FALSE(dnd.enabled());
    EXPECT_EQ(a, 2);
}

TEST(DNDIndicatorVisibilityAndTile) {
    qypr::DndState dnd;
    qypr::SystemBackends backends{};
    backends.dnd = &dnd;
    qypr::DNDIndicator ind(backends);

    // Moon icon hidden until DND is active
    EXPECT_FALSE(ind.visible);
    dnd.setEnabled(true);
    ind.onBackendUpdate();
    EXPECT_TRUE(ind.visible);

    auto tile = ind.createTile();
    EXPECT_TRUE(tile != nullptr);
    EXPECT_TRUE(tile->type() == qypr::QSTile::Type::Toggle);

    // The tile toggles the shared state directly
    tile->onClick(10, 10);
    EXPECT_FALSE(dnd.enabled());
}

// =============================================================================
// Phase 4: Volume Tests
// =============================================================================

TEST(VolumeIndicatorConstruction) {
    qypr::SystemBackends backends{};
    qypr::VolumeIndicator vol(backends);

    EXPECT_EQ(vol.id(), std::string("volume"));
    EXPECT_TRUE(static_cast<int>(vol.zone()) == static_cast<int>(qypr::Zone::Right));
    EXPECT_EQ(vol.priority(), 200);

    // Icon tracks level and mute
    vol.lastSnap_.level = 0.8;
    EXPECT_EQ(vol.icon(), std::string("󰕾"));
    vol.lastSnap_.level = 0.5;
    EXPECT_EQ(vol.icon(), std::string("󰖀"));
    vol.lastSnap_.level = 0.1;
    EXPECT_EQ(vol.icon(), std::string("󰕿"));
    vol.lastSnap_.muted = true;
    EXPECT_EQ(vol.icon(), std::string("󰝟"));

    // Scroll without a backend must be a no-op, not a crash
    EXPECT_FALSE(vol.onScroll(0, -1.0));
}

TEST(VolumeIndicatorCreatesSliderTile) {
    qypr::SystemBackends backends{};
    qypr::VolumeIndicator vol(backends);
    vol.lastSnap_.level = 0.55;

    auto tile = vol.createTile();
    EXPECT_TRUE(tile != nullptr);
    EXPECT_TRUE(tile->type() == qypr::QSTile::Type::Slider);

    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 400, 60);
    cairo_t* cr = cairo_create(surf);
    qypr::Painter p(cr);
    tile->bounds = {0, 0, 348, 40};
    tile->draw(p, qypr::nowMs());
    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}

TEST(VolumeBackendConstructAndTeardown) {
    // Exercises the PulseLoop adapter lifecycle: async connect begins, then
    // the destructor disconnects with callbacks silenced. Must not crash or
    // touch freed memory whether or not a pulse server exists.
    qypr::EventLoop loop;
    qypr::VolumeBackend backend(loop);
    backend.setOnChange([] {});
    backend.start();
    // Snapshot stays unavailable until the (never-run) loop delivers READY.
    EXPECT_FALSE(backend.snapshot().available);
}

// -----------------------------------------------------------------------------
// SNI tray host (Phase 5)
// -----------------------------------------------------------------------------
TEST(SNIParseItemRef) {
    std::string service, path;

    // "service/path" form (as reported by the watcher for real items).
    qypr::SNIBackend::parseItemRef(":1.51/org/blueman/sni", service, path);
    EXPECT_EQ(service, std::string(":1.51"));
    EXPECT_EQ(path, std::string("/org/blueman/sni"));

    qypr::SNIBackend::parseItemRef(":1.17/org/ayatana/NotificationItem/nm_applet", service, path);
    EXPECT_EQ(service, std::string(":1.17"));
    EXPECT_EQ(path, std::string("/org/ayatana/NotificationItem/nm_applet"));

    // Bare service name: default object path per the spec.
    qypr::SNIBackend::parseItemRef(":1.42", service, path);
    EXPECT_EQ(service, std::string(":1.42"));
    EXPECT_EQ(path, std::string("/StatusNotifierItem"));
}

TEST(SNITrayHostConstruction) {
    qypr::SystemBackends backends{};  // no backend
    qypr::SNITrayHost host(backends);

    EXPECT_EQ(host.id(), std::string("sni"));
    EXPECT_TRUE(static_cast<int>(host.zone()) == static_cast<int>(qypr::Zone::Right));
    EXPECT_EQ(host.priority(), 600);
    EXPECT_EQ(host.icon(), std::string(""));  // custom multi-icon draw

    // No backend → hidden, zero width, click is a no-op (not a crash).
    host.onBackendUpdate();
    EXPECT_FALSE(host.visible);

    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    cairo_t* cr = cairo_create(surf);
    qypr::Painter p(cr);
    EXPECT_EQ(host.measureWidth(p), 0.0);
    EXPECT_FALSE(host.onClick(10, 10));
    cairo_destroy(cr);
    cairo_surface_destroy(surf);

    EXPECT_EQ(host.tooltip(), std::string("System tray"));
}

TEST(SNITrayHostVisibleWithItems) {
    // Drive the indicator from a backend whose item list we populate directly
    // (no bus needed): the tray host mirrors item count for visibility/width.
    qypr::EventLoop loop;
    qypr::SystemBus session(loop, qypr::BusKind::Session);
    qypr::SNIBackend sni(session);
    sni.items_.push_back(qypr::SNIItem{":1.51", "/org/blueman/sni", "blueman", "blueman", "Active",
                                       "/org/blueman/sni/menu", nullptr});
    sni.items_.push_back(qypr::SNIItem{":1.17", "/org/ayatana/NotificationItem/nm_applet",
                                       "nm-signal-75", "Network", "Active", "", nullptr});

    qypr::SystemBackends backends{};
    backends.sni = &sni;
    qypr::SNITrayHost host(backends);

    host.onBackendUpdate();
    EXPECT_TRUE(host.visible);

    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    cairo_t* cr = cairo_create(surf);
    qypr::Painter p(cr);
    // Two 18px icons + one 6px gap + 2*8px side pad = 68px.
    EXPECT_NEAR(host.measureWidth(p), 2 * 18.0 + 6.0 + 2 * 8.0, 0.01);
    cairo_destroy(cr);
    cairo_surface_destroy(surf);

    // Tooltip reflects the count when more than one item.
    EXPECT_EQ(host.tooltip(), std::string("2 tray items"));

    // A click within the first icon's slot resolves without a bus (the mock's
    // async call is a no-op) and is consumed.
    host.bounds = {100, 0, host.measureWidth(p), 36};
    EXPECT_TRUE(host.onClick(100 + 8 + 2, 18));
}

// -----------------------------------------------------------------------------
// Workspaces + Active window (WM widgets — session-sensitive, hidden while locked)
// -----------------------------------------------------------------------------
TEST(WorkspacesIndicatorConstruction) {
    qypr::SystemBackends backends{};  // no backend
    qypr::WorkspacesIndicator ws(backends);

    EXPECT_EQ(ws.id(), std::string("workspaces"));
    EXPECT_TRUE(static_cast<int>(ws.zone()) == static_cast<int>(qypr::Zone::Left));
    EXPECT_EQ(ws.priority(), -100);
    EXPECT_EQ(ws.icon(), std::string(""));
    EXPECT_TRUE(ws.sensitive());  // must be gated off while locked

    ws.onBackendUpdate();
    EXPECT_FALSE(ws.visible);  // no backend → hidden
    EXPECT_EQ(ws.tooltip(), std::string("Workspaces"));
}

TEST(WorkspacesIndicatorRendersAndActivates) {
    qypr::WorkspaceBackend backend;
    backend.snap_.available = true;
    backend.snap_.workspaces = {{"1", true, false}, {"2", false, false}, {"3", false, true}};

    qypr::SystemBackends backends{};
    backends.workspace = &backend;
    qypr::WorkspacesIndicator ws(backends);

    ws.onBackendUpdate();
    EXPECT_TRUE(ws.visible);
    EXPECT_EQ(ws.tooltip(), std::string("Workspace 1"));

    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 600, 40);
    cairo_t* cr = cairo_create(surf);
    qypr::Painter p(cr);
    EXPECT_TRUE(ws.measureWidth(p) > 0);

    // draw populates per-pill hit rects; a click inside a pill is consumed
    // (activate is a no-op without a live compositor, but must not crash).
    ws.bounds = {0, 0, ws.measureWidth(p), 36};
    ws.draw(p, qypr::nowMs());
    EXPECT_TRUE(ws.onClick(ws.bounds.x + 10, 18));
    EXPECT_FALSE(ws.onClick(9000, 18));  // outside all pills
    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}

TEST(ActiveWindowIndicatorConstruction) {
    qypr::SystemBackends backends{};
    qypr::ActiveWindowIndicator aw(backends);

    EXPECT_EQ(aw.id(), std::string("active-window"));
    EXPECT_TRUE(static_cast<int>(aw.zone()) == static_cast<int>(qypr::Zone::Center));
    EXPECT_TRUE(aw.sensitive());
    EXPECT_EQ(aw.icon(), std::string(""));

    aw.onBackendUpdate();
    EXPECT_FALSE(aw.visible);
    EXPECT_EQ(aw.label(), std::string(""));
}

TEST(ActiveWindowIndicatorShowsFocused) {
    qypr::ToplevelBackend backend;
    backend.snap_.available = true;
    backend.snap_.hasActive = true;
    backend.snap_.appId = "kitty";
    backend.snap_.title = "vim — file.cpp";

    qypr::SystemBackends backends{};
    backends.toplevel = &backend;
    qypr::ActiveWindowIndicator aw(backends);

    aw.onBackendUpdate();
    EXPECT_TRUE(aw.visible);
    EXPECT_EQ(aw.label(), std::string("vim — file.cpp"));  // title preferred
    EXPECT_EQ(aw.tooltip(), std::string("kitty — vim — file.cpp"));

    // Falls back to app id when the title is empty.
    backend.snap_.title = "";
    aw.onBackendUpdate();
    EXPECT_EQ(aw.label(), std::string("kitty"));
}

TEST(ActiveWindowIndicatorTruncatesUtf8) {
    qypr::ToplevelBackend backend;
    backend.snap_.available = true;
    backend.snap_.hasActive = true;
    // 70 multibyte codepoints (each "→" is 3 bytes): must cut on a codepoint
    // boundary and append the ellipsis — never split a character.
    std::string title;
    for (int i = 0; i < 70; ++i) title += "\xE2\x86\x92";  // U+2192
    backend.snap_.title = title;

    qypr::SystemBackends backends{};
    backends.toplevel = &backend;
    qypr::ActiveWindowIndicator aw(backends);
    aw.onBackendUpdate();

    std::string shown = aw.label();
    // 60 codepoints kept (60*3 bytes) + "…" (3 bytes).
    EXPECT_EQ(shown.size(), static_cast<size_t>(60 * 3 + 3));
    EXPECT_TRUE(shown.size() < title.size());
}

TEST(SensitiveIndicatorsGatedByDefault) {
    // The privacy contract: workspace/active-window are sensitive; the common
    // indicators are not. StatusBar hides sensitive ones unless session content
    // is explicitly enabled (never on the lock screen).
    qypr::SystemBackends b{};
    qypr::WorkspacesIndicator ws(b);
    qypr::ActiveWindowIndicator aw(b);
    qypr::BatteryIndicator bat(b);
    EXPECT_TRUE(ws.sensitive());
    EXPECT_TRUE(aw.sensitive());
    EXPECT_FALSE(bat.sensitive());  // default: not sensitive
}

// -----------------------------------------------------------------------------
// Phase 9 — Config
// -----------------------------------------------------------------------------

// Write a temp config and return its path.
static std::string writeTempConfig(const std::string& body) {
    std::string path = "/tmp/qypr-test-" + std::to_string(::getpid()) + "-" +
                       std::to_string(::rand()) + ".conf";
    std::ofstream f(path);
    f << body;
    f.close();
    return path;
}

TEST(ConfigParsing) {
    const std::string path = writeTempConfig(
        "# a comment\n"
        "// another comment\n"
        "\n"
        "[bar]\n"
        "position = bottom\n"
        "height = 40\n"
        "backdrop = 0.5\n"
        "auto-hide = yes\n"
        "modules-left = workspaces, clock\n"
        "  spaced-key   =   value with inner spaces  \n"
        "junk line without equals\n"
        "\n"
        "[clock]\n"
        "format = %H:%M\n");

    qypr::Config c;
    EXPECT_TRUE(c.load(path));
    EXPECT_TRUE(c.loaded());

    // Typed accessors + section scoping.
    EXPECT_EQ(c.getString("bar", "position", "top"), std::string("bottom"));
    EXPECT_EQ(c.getInt("bar", "height", 36), 40);
    EXPECT_TRUE(std::fabs(c.getDouble("bar", "backdrop", 0.8) - 0.5) < 1e-9);
    EXPECT_TRUE(c.getBool("bar", "auto-hide", false));
    EXPECT_EQ(c.getString("clock", "format", "x"), std::string("%H:%M"));

    // Ends trimmed, inner spaces kept.
    EXPECT_EQ(c.getString("bar", "spaced-key", ""), std::string("value with inner spaces"));

    // Lists split + trim.
    auto mods = c.getList("bar", "modules-left");
    EXPECT_EQ(static_cast<int>(mods.size()), 2);
    EXPECT_EQ(mods[0], std::string("workspaces"));
    EXPECT_EQ(mods[1], std::string("clock"));

    // Absent keys fall back; a key in the wrong section is absent.
    EXPECT_EQ(c.getInt("bar", "nope", 7), 7);
    EXPECT_EQ(c.getString("bar", "format", "def"), std::string("def"));  // clock's, not bar's
    EXPECT_FALSE(c.has("bar", "format"));
    EXPECT_TRUE(c.has("clock", "format"));

    ::unlink(path.c_str());
}

TEST(ConfigMissingFileIsNotAnError) {
    // The compiled-in bar must still run with no config at all.
    qypr::Config c;
    EXPECT_FALSE(c.load("/tmp/qypr-definitely-does-not-exist-9182.conf"));
    EXPECT_FALSE(c.loaded());
    EXPECT_EQ(c.getInt("bar", "height", 36), 36);       // default survives
    EXPECT_EQ(static_cast<int>(c.getList("bar", "modules-left").size()), 0);
}

TEST(ConfigMalformedValuesKeepDefaults) {
    const std::string path = writeTempConfig(
        "[bar]\nheight = not-a-number\nbackdrop = \nflag = maybe\n");
    qypr::Config c;
    EXPECT_TRUE(c.load(path));
    EXPECT_EQ(c.getInt("bar", "height", 36), 36);                       // stoi throws → default
    EXPECT_TRUE(std::fabs(c.getDouble("bar", "backdrop", 0.8) - 0.8) < 1e-9);
    EXPECT_TRUE(c.getBool("bar", "flag", true));                        // unparseable → default
    ::unlink(path.c_str());
}

TEST(ConfigEmptyListEmptiesZone) {
    // An explicitly empty value means "this zone is empty", which must be
    // distinguishable from "key absent" (= use defaults).
    const std::string path = writeTempConfig("[bar]\nmodules-center =\n");
    qypr::Config c;
    EXPECT_TRUE(c.load(path));
    EXPECT_TRUE(c.has("bar", "modules-center"));
    std::vector<std::string> fallback{"active-window"};
    EXPECT_EQ(static_cast<int>(c.getList("bar", "modules-center", fallback).size()), 0);
    // Absent key → caller's fallback.
    EXPECT_EQ(static_cast<int>(c.getList("bar", "modules-left", fallback).size()), 1);
    ::unlink(path.c_str());
}

TEST(ConfigDirRespectsXdg) {
    const char* old = ::getenv("XDG_CONFIG_HOME");
    const std::string saved = old ? old : "";
    ::setenv("XDG_CONFIG_HOME", "/tmp/xdg-probe", 1);
    EXPECT_EQ(qypr::Config::configDir(), std::string("/tmp/xdg-probe/qypr"));
    EXPECT_EQ(qypr::Config::defaultPath(), std::string("/tmp/xdg-probe/qypr/bar.conf"));

    // Without XDG_CONFIG_HOME it falls back to $HOME/.config/qypr.
    ::unsetenv("XDG_CONFIG_HOME");
    ::setenv("HOME", "/tmp/home-probe", 1);
    EXPECT_EQ(qypr::Config::configDir(), std::string("/tmp/home-probe/.config/qypr"));

    if (!saved.empty()) ::setenv("XDG_CONFIG_HOME", saved.c_str(), 1);
}

// These use a LOCAL registry rather than instance(). The TEST macro runs bodies
// during static initialisation, so the compiled-in REGISTER_INDICATOR set is not
// guaranteed to exist yet (cross-TU static init order is unspecified) — a test
// leaning on it would pass or fail by link order. A local registry is hermetic
// and also keeps fake indicators out of the global one.
static qypr::IndicatorRegistry::Factory testFactory(const std::string& id, qypr::Zone z, int p) {
    return [id, z, p](const qypr::SystemBackends&) {
        return std::make_unique<TestIndicator>(id, z, p);
    };
}

TEST(RegistryModuleSelection) {
    qypr::IndicatorRegistry reg;  // ctor reachable via `#define private public`
    reg.registerIndicator("clock", qypr::Zone::Left, 0, testFactory("clock", qypr::Zone::Left, 0));
    reg.registerIndicator("battery", qypr::Zone::Right, 500,
                          testFactory("battery", qypr::Zone::Right, 500));
    reg.registerIndicator("volume", qypr::Zone::Right, 200,
                          testFactory("volume", qypr::Zone::Right, 200));
    reg.registerIndicator("brightness", qypr::Zone::Right, 100,
                          testFactory("brightness", qypr::Zone::Right, 100));

    // Config-driven selection: only the named ids, in the listed order, re-homed
    // to the zone they were listed under.
    qypr::SystemBackends b{};
    qypr::IndicatorRegistry::ModuleSelection sel;
    sel.left = {"clock"};                  // compiled Left, stays Left
    sel.center = {"battery"};              // compiled Right → re-homed to Center
    sel.right = {"volume", "brightness"};  // reversed vs compiled priority

    auto made = reg.createAll(b, &sel);
    EXPECT_EQ(static_cast<int>(made.size()), 4);
    EXPECT_EQ(made[0]->id(), std::string("clock"));
    EXPECT_TRUE(made[0]->zone() == qypr::Zone::Left);
    // Re-homed: a module lands in the zone it was listed under.
    EXPECT_EQ(made[1]->id(), std::string("battery"));
    EXPECT_TRUE(made[1]->zone() == qypr::Zone::Center);
    // Listed order wins over compiled priority (brightness=100 < volume=200
    // would otherwise sort first).
    EXPECT_EQ(made[2]->id(), std::string("volume"));
    EXPECT_EQ(made[3]->id(), std::string("brightness"));
    EXPECT_TRUE(made[2]->zone() == qypr::Zone::Right);
}

TEST(RegistryUnknownModuleIsSkipped) {
    // A typo must drop that module, never crash the bar.
    qypr::IndicatorRegistry reg;
    reg.registerIndicator("clock", qypr::Zone::Left, 0, testFactory("clock", qypr::Zone::Left, 0));
    reg.registerIndicator("battery", qypr::Zone::Right, 500,
                          testFactory("battery", qypr::Zone::Right, 500));

    qypr::SystemBackends b{};
    qypr::IndicatorRegistry::ModuleSelection sel;
    sel.left = {"clock", "no-such-module", "battery"};
    auto made = reg.createAll(b, &sel);
    EXPECT_EQ(static_cast<int>(made.size()), 2);
    EXPECT_EQ(made[0]->id(), std::string("clock"));
    EXPECT_EQ(made[1]->id(), std::string("battery"));
}

TEST(RegistryNullSelectionKeepsCompiledDefaults) {
    // No selection (the lock screen's path): every registered indicator, grouped
    // by compiled zone, priority ascending — unchanged behaviour.
    qypr::IndicatorRegistry reg;
    reg.registerIndicator("battery", qypr::Zone::Right, 500,
                          testFactory("battery", qypr::Zone::Right, 500));
    reg.registerIndicator("brightness", qypr::Zone::Right, 100,
                          testFactory("brightness", qypr::Zone::Right, 100));
    reg.registerIndicator("clock", qypr::Zone::Left, 0, testFactory("clock", qypr::Zone::Left, 0));

    qypr::SystemBackends b{};
    auto all = reg.createAll(b, nullptr);
    EXPECT_EQ(static_cast<int>(all.size()), 3);
    EXPECT_EQ(static_cast<int>(reg.registeredIds().size()), 3);
    // Left zone first, then Right by ascending priority.
    EXPECT_EQ(all[0]->id(), std::string("clock"));
    EXPECT_EQ(all[1]->id(), std::string("brightness"));  // 100 before 500
    EXPECT_EQ(all[2]->id(), std::string("battery"));
    for (size_t i = 1; i < all.size(); ++i) {
        if (all[i - 1]->zone() == all[i]->zone()) {
            EXPECT_TRUE(all[i - 1]->priority() <= all[i]->priority());
        }
    }
}

TEST(ClockFormatFromConfig) {
    const std::string path = writeTempConfig("[clock]\nformat = %Y\n");
    qypr::Config c;
    EXPECT_TRUE(c.load(path));

    qypr::SystemBackends b{};
    b.config = &c;
    qypr::ClockIndicator clk(b);
    clk.poll(1'000'000);  // force a refresh past the 1s gate

    // %Y renders a 4-digit year — proves the config format is in effect.
    const std::string label = clk.label();
    EXPECT_EQ(static_cast<int>(label.size()), 4);
    EXPECT_TRUE(label[0] == '2');

    // No config → compiled default (contains ":" from %-I:%M).
    qypr::SystemBackends plain{};
    qypr::ClockIndicator def(plain);
    def.poll(1'000'000);
    EXPECT_TRUE(def.label().find(':') != std::string::npos);

    ::unlink(path.c_str());
}

TEST(StatusBarGeometryTopAndBottom) {
    qypr::EventLoop loop;
    struct Inv : qypr::Invalidator { void invalidate() override {} } inv;
    qypr::SystemBackends b{};
    qypr::StatusBar bar(loop, inv, b);

    // Default (top): the strip sits `edgeMargin` below the top edge.
    qypr::BarGeometry top;
    top.height = 36; top.edgeMargin = 24; top.sideMargin = 48; top.bottom = false;
    bar.setGeometry(top);
    bar.layout(1920, 66);
    EXPECT_TRUE(std::fabs(bar.bounds.y - 24.0) < 1e-9);
    EXPECT_TRUE(std::fabs(bar.bounds.w - (1920 - 96)) < 1e-9);

    // Bottom: measured from screenH, so it pins to the lower edge — and stays
    // pinned when the host surface grows for an overlay (66 → 1200).
    qypr::BarGeometry bot = top;
    bot.bottom = true;
    bar.setGeometry(bot);
    bar.layout(1920, 66);
    EXPECT_TRUE(std::fabs(bar.bounds.y - (66 - 24 - 36)) < 1e-9);  // 6
    bar.layout(1920, 1200);
    EXPECT_TRUE(std::fabs(bar.bounds.y - (1200 - 24 - 36)) < 1e-9);  // 1140
}

TEST(PopoverGrowsAwayFromBarEdge) {
    // A bottom bar must open its panels upward, or they render off-screen.
    struct P : qypr::DetailedPopover {
        void draw(qypr::Painter&, int64_t) override {}
        double contentHeight() const override { return 100.0; }
        double contentWidth() const override { return 200.0; }
    } pop;

    pop.anchorX = 500; pop.anchorY = 60; pop.growUp = false;
    EXPECT_TRUE(std::fabs(pop.getBounds().y - 60.0) < 1e-9);   // hangs down
    EXPECT_TRUE(std::fabs(pop.getBounds().x - 300.0) < 1e-9);  // right-aligned to anchor

    pop.growUp = true;
    EXPECT_TRUE(std::fabs(pop.getBounds().y - (60.0 - 100.0)) < 1e-9);  // extends up
}

// -----------------------------------------------------------------------------
// Phase 10 — Session surface (media / notifications / power)
// -----------------------------------------------------------------------------

TEST(SessionAppletsAbsentWithoutTheirBackends) {
    // The lock screen's App supplies no PowerManager and no NotificationMonitor.
    // Both applets must then not exist at all — belt-and-braces with sensitive().
    qypr::SystemBackends none{};
    qypr::PowerMenuIndicator power(none);
    qypr::NotificationIndicator notes(none);
    EXPECT_FALSE(power.visible);
    EXPECT_FALSE(notes.visible);

    // And both are session-sensitive, so even a host that constructed them would
    // have to opt in explicitly (StatusBar::setSessionContentVisible).
    EXPECT_TRUE(power.sensitive());
    EXPECT_TRUE(notes.sensitive());
}

TEST(SessionAppletsAppearWithBackends) {
    qypr::EventLoop loop;
    qypr::PowerManager pm;                 // TESTING: actions are no-ops
    qypr::NotificationMonitor mon(loop);   // not started: empty, but present
    qypr::SystemBackends b{};
    b.power = &pm;
    b.notifications = &mon;

    qypr::PowerMenuIndicator power(b);
    qypr::NotificationIndicator notes(b);
    EXPECT_TRUE(power.visible);
    EXPECT_TRUE(notes.visible);

    // Both offer a detailed popover (the menu / the history).
    EXPECT_TRUE(power.hasDetailedView());
    EXPECT_TRUE(notes.hasDetailedView());
    EXPECT_TRUE(power.createDetailedView() != nullptr);
    EXPECT_TRUE(notes.createDetailedView() != nullptr);
}

TEST(NotificationIndicatorCountAndDnd) {
    qypr::EventLoop loop;
    qypr::NotificationMonitor mon(loop);
    qypr::DndState dnd;
    qypr::SystemBackends b{};
    b.notifications = &mon;
    b.dnd = &dnd;
    qypr::NotificationIndicator ind(b);

    // Empty: no count label, muted colour, honest tooltip.
    EXPECT_EQ(ind.label(), std::string(""));
    EXPECT_EQ(ind.tooltip(), std::string("No notifications"));

    // With notifications the bell carries the count and the singular/plural is
    // correct. (notes_ is reachable via `#define private public`.)
    mon.notes_.push_back(qypr::Notification{});
    EXPECT_EQ(ind.label(), std::string("1"));
    EXPECT_EQ(ind.tooltip(), std::string("1 notification"));
    mon.notes_.push_back(qypr::Notification{});
    EXPECT_EQ(ind.label(), std::string("2"));
    EXPECT_EQ(ind.tooltip(), std::string("2 notifications"));

    // DND mutes the bell glyph but keeps the count: suppressed notifications are
    // still waiting for you.
    const std::string bell = ind.icon();
    dnd.setEnabled(true);
    EXPECT_TRUE(ind.icon() != bell);
    EXPECT_EQ(ind.label(), std::string("2"));
    dnd.setEnabled(false);
    EXPECT_EQ(ind.icon(), bell);
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
