#include "wayland/BarWindow.hpp"

#include "wlr-layer-shell-unstable-v1-client-protocol.h"

#include <cairo/cairo.h>

namespace qypr {

namespace {
const wl_output_listener kOutputListener = {
    .geometry = BarWindow::onGeometry,
    .mode = BarWindow::onMode,
    .done = BarWindow::onDone,
    .scale = BarWindow::onScale,
    .name = BarWindow::onName,
    .description = BarWindow::onDescription,
};

const zwlr_layer_surface_v1_listener kLayerSurfaceListener = {
    .configure = BarWindow::onConfigure,
    .closed = BarWindow::onClosed,
};

// Fallback output height (logical) if the compositor never sent a mode before
// we needed to size an overlay — generous enough for any Quick Settings panel.
constexpr int kFallbackOutputHeight = 2160;
}  // namespace

BarWindow::BarWindow(wl_output* output, uint32_t name, OutputEnv* env, int reservedHeight)
    : output_(output),
      name_(name),
      env_(env),
      reservedHeight_(reservedHeight),
      requestedHeight_(reservedHeight) {
    wl_output_add_listener(output_, &kOutputListener, this);
}

BarWindow::~BarWindow() {
    if (frameCallback_) wl_callback_destroy(frameCallback_);
    buffers_.clear();
    if (layerSurface_) zwlr_layer_surface_v1_destroy(layerSurface_);
    if (surface_) wl_surface_destroy(surface_);
    if (output_) wl_output_destroy(output_);
}

void BarWindow::createLayerSurface(zwlr_layer_shell_v1* shell) {
    if (layerSurface_) return;
    shell_ = shell;
    surface_ = wl_compositor_create_surface(env_->compositor);
    wl_surface_set_buffer_scale(surface_, scale_);

    layerSurface_ = zwlr_layer_shell_v1_get_layer_surface(
        shell_, surface_, output_, ZWLR_LAYER_SHELL_V1_LAYER_TOP, "qypr-bar");
    zwlr_layer_surface_v1_add_listener(layerSurface_, &kLayerSurfaceListener, this);

    // Anchor a full-width strip to the top; width 0 stretches between the
    // left/right anchors. Reserve the strip height so tiled windows avoid it.
    zwlr_layer_surface_v1_set_anchor(layerSurface_,
                                     ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                                         ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                                         ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
    zwlr_layer_surface_v1_set_size(layerSurface_, 0, requestedHeight_);
    zwlr_layer_surface_v1_set_exclusive_zone(layerSurface_, reservedHeight_);
    zwlr_layer_surface_v1_set_keyboard_interactivity(
        layerSurface_, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE);

    // First commit with no buffer: the compositor answers with a configure that
    // carries our size, and render() attaches the first frame.
    wl_surface_commit(surface_);
}

void BarWindow::requestHeight(int logicalH) {
    if (!layerSurface_ || logicalH == requestedHeight_) return;
    requestedHeight_ = logicalH;
    zwlr_layer_surface_v1_set_size(layerSurface_, 0, requestedHeight_);
    // Commit the size change; the compositor replies with a configure and
    // render() repaints at the new height.
    wl_surface_commit(surface_);
}

void BarWindow::setOverlayActive(bool active) {
    if (active == overlayActive_) return;
    overlayActive_ = active;
    const int full = outputHeight_ > 0 ? outputHeight_ : kFallbackOutputHeight;
    requestHeight(active ? full : reservedHeight_);
}

// -----------------------------------------------------------------------------
// wl_output — track scale and the current mode (for overlay sizing).
// -----------------------------------------------------------------------------
void BarWindow::onScale(void* data, wl_output*, int32_t factor) {
    auto* self = static_cast<BarWindow*>(data);
    if (factor > 0) self->scale_ = factor;
}
void BarWindow::onMode(void* data, wl_output*, uint32_t flags, int32_t, int32_t height, int32_t) {
    auto* self = static_cast<BarWindow*>(data);
    constexpr uint32_t kCurrent = 0x1;  // WL_OUTPUT_MODE_CURRENT
    if ((flags & kCurrent) && height > 0) self->outputHeight_ = height / (self->scale_ > 0 ? self->scale_ : 1);
}
void BarWindow::onGeometry(void*, wl_output*, int32_t, int32_t, int32_t, int32_t, int32_t,
                           const char*, const char*, int32_t) {}
void BarWindow::onDone(void*, wl_output*) {}
void BarWindow::onName(void*, wl_output*, const char*) {}
void BarWindow::onDescription(void*, wl_output*, const char*) {}

// -----------------------------------------------------------------------------
// Layer-surface configure: adopt the compositor's size, then paint.
// -----------------------------------------------------------------------------
void BarWindow::onConfigure(void* data, zwlr_layer_surface_v1* surf, uint32_t serial,
                            uint32_t width, uint32_t height) {
    auto* self = static_cast<BarWindow*>(data);
    zwlr_layer_surface_v1_ack_configure(surf, serial);
    self->width_ = static_cast<int>(width);
    self->height_ = static_cast<int>(height);
    self->configured_ = true;
    self->dirty_ = true;
    self->render();
}

void BarWindow::onClosed(void* data, zwlr_layer_surface_v1*) {
    auto* self = static_cast<BarWindow*>(data);
    self->configured_ = false;  // compositor tore the surface down (e.g. output gone)
}

void BarWindow::invalidate() {
    dirty_ = true;
    if (!framePending_) render();
}

// -----------------------------------------------------------------------------
// Frame callback: throttle repaints to the compositor's cadence.
// -----------------------------------------------------------------------------
void BarWindow::onFrame(void* data, wl_callback* cb, uint32_t) {
    auto* self = static_cast<BarWindow*>(data);
    wl_callback_destroy(cb);
    self->frameCallback_ = nullptr;
    self->framePending_ = false;
    if (self->dirty_ || (self->env_->animating && self->env_->animating())) self->render();
}

ShmBuffer* BarWindow::acquireBuffer(int pxW, int pxH) {
    for (auto& b : buffers_) {
        if (!b->busy() && b->width() == pxW && b->height() == pxH) return b.get();
    }
    // Drop stale-sized free buffers to bound memory, then allocate.
    if (buffers_.size() >= 2) {
        for (auto it = buffers_.begin(); it != buffers_.end();) {
            if (!(*it)->busy() && ((*it)->width() != pxW || (*it)->height() != pxH))
                it = buffers_.erase(it);
            else
                ++it;
        }
    }
    auto buf = ShmBuffer::create(env_->shm, pxW, pxH);
    if (!buf) return nullptr;
    buffers_.push_back(std::move(buf));
    return buffers_.back().get();
}

void BarWindow::render() {
    if (!configured_ || width_ <= 0 || height_ <= 0) return;

    const int pxW = width_ * scale_;
    const int pxH = height_ * scale_;
    ShmBuffer* buf = acquireBuffer(pxW, pxH);
    if (!buf) return;

    cairo_t* cr = cairo_create(buf->cairoSurface());
    cairo_scale(cr, scale_, scale_);
    if (env_->render) env_->render(cr, width_, height_, scale_);
    cairo_destroy(cr);
    cairo_surface_flush(buf->cairoSurface());

    wl_surface_attach(surface_, buf->buffer(), 0, 0);
    buf->markBusy();
    wl_surface_damage_buffer(surface_, 0, 0, pxW, pxH);

    frameCallback_ = wl_surface_frame(surface_);
    static const wl_callback_listener kFrameListener = {.done = BarWindow::onFrame};
    wl_callback_add_listener(frameCallback_, &kFrameListener, this);
    framePending_ = true;
    dirty_ = false;

    wl_surface_commit(surface_);
}

}  // namespace qypr
