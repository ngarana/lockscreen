#include "core/App.hpp"

#include <cairo/cairo.h>
#include <unistd.h>

#include <cstdio>
#include <string>

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
      lockScreen_(loop_, *this, pam_, power_) {
    lockScreen_.setAudioController(&audio_);
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

    loop_.run();
    return 0;
}

int App::preview(const std::string& path, int width, int height) {
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

void App::invalidate() { display_.invalidateAll(); }

void App::requestUnlock() {
    lock_.unlock();
    loop_.quit();
}

}  // namespace qypr
