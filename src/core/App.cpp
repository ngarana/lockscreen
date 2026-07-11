#include "core/App.hpp"

#include <cairo/cairo.h>
#include <unistd.h>

#include <cstdio>
#include <string>

#include "ui/Notification.hpp"

namespace qypr {

namespace {
void renderToPng(qypr::LockScreen& ls, const std::string& path, int w, int h) {
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    cairo_t* cr = cairo_create(surface);
    ls.draw(cr, w, h, 1);
    cairo_destroy(cr);
    cairo_surface_write_to_png(surface, path.c_str());
    cairo_surface_destroy(surface);
}
}  // namespace

App::App()
    : display_(loop_),
      lock_(display_),
      pam_(loop_),
      audio_(mpris_),
      video_(loop_, *this),
      lockScreen_(loop_, *this, pam_, power_) {
    lockScreen_.setAudioController(&audio_);
    lockScreen_.setVideoPlayer(&video_);

    // Live notifications are pushed into the UI as the monitor sees them.
    // No fallback data on the real lock screen — samples are preview-only.
    notifications_.setOnChange([this] {
        lockScreen_.setNotifications(notifications_.notifications());
        invalidate();
    });
}

int App::run() {
    if (!display_.connect()) {
        std::fprintf(stderr, "qypr-lock: no Wayland display or no ext-session-lock support\n");
        return 1;
    }

    display_.setInputSink(&lockScreen_);
    display_.setRenderFn(
        [this](cairo_t* cr, int w, int h, int s) { lockScreen_.draw(cr, w, h, s); });
    display_.setAnimatingFn([this] { return lockScreen_.isAnimating(); });

    lock_.setOnFinished([this] {
        std::fprintf(stderr, "qypr-lock: session lock refused or lost\n");
        loop_.quit();
    });

    if (!lock_.lock()) {
        std::fprintf(stderr, "qypr-lock: failed to acquire session lock\n");
        return 1;
    }

    // Start the notification monitor, seeded with the pre-lock backlog from a
    // running --record service (non-fatal: it logs when unavailable).
    notifications_.start(/*seedFromLog=*/true);

    // Start video playback (non-fatal if it fails).
    if (video_.init()) {
        video_.start();
    } else {
        std::fprintf(stderr, "qypr-lock: video background unavailable\n");
    }

    loop_.run();
    return 0;
}

int App::preview(const std::string& path, int width, int height) {
    lockScreen_.setNotifications(demoNotifications());  // sample cards, preview only

    // Idle state (before any interaction).
    renderToPng(lockScreen_, path.substr(0, path.rfind('.')) + "-idle.png", width, height);

    // Revealed state: simulate typing, then let the reveal animation finish.
    mpris_.refresh();  // surface the audio panel if something is playing
    lockScreen_.onTextInput("password");
    usleep(700 * 1000);
    renderToPng(lockScreen_, path, width, height);

    std::fprintf(stderr, "qypr-lock: wrote preview frames near %s\n", path.c_str());
    return 0;
}

int App::videoTest(int seconds) {
    if (!display_.connect()) {
        std::fprintf(stderr, "video-test: no Wayland display\n");
        return 1;
    }
    if (!video_.init()) {
        std::fprintf(stderr, "video-test: init FAILED\n");
        return 1;
    }
    video_.start();

    // Heartbeat: report frame status once a second.
    loop_.addTimer(1000, true, [this] {
        std::fprintf(stderr, "video-test: hasFrame=%d\n", video_.hasFrame() ? 1 : 0);
    });
    loop_.addTimer(seconds * 1000, false, [this] { loop_.quit(); });

    loop_.run();
    std::fprintf(stderr, "video-test: DONE hasFrame=%d\n", video_.hasFrame() ? 1 : 0);

    // Dump one composited frame so orientation/colour can be eyeballed.
    if (video_.hasFrame()) {
        cairo_surface_t* s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1920, 1080);
        cairo_t* cr = cairo_create(s);
        video_.draw(cr, 1920, 1080);
        cairo_destroy(cr);
        cairo_surface_write_to_png(s, "/tmp/qypr-videoframe.png");
        cairo_surface_destroy(s);
        std::fprintf(stderr, "video-test: wrote /tmp/qypr-videoframe.png\n");
    }
    return video_.hasFrame() ? 0 : 2;
}

void App::setIdleTimeout(int seconds) {
    if (seconds > 0) lockScreen_.setIdleTimeout(static_cast<int64_t>(seconds) * 1000);
}

void App::invalidate() { display_.invalidateAll(); }

void App::requestUnlock() {
    lock_.unlock();
    loop_.quit();
}

}  // namespace qypr
