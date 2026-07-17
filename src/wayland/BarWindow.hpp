// BarWindow.hpp - One monitor's wlr-layer-shell surface for the standalone bar.
//
// The unlocked-desktop counterpart to Output (which owns a session-lock
// surface). It anchors a chromeless strip to the top of an output, reserves an
// exclusive zone so tiled windows never sit under it, and drives the same
// throttled shm render loop. When the bar opens an overlay (Quick Settings or a
// popover) the surface grows to the full output height so the overlay — drawn
// at absolute coordinates, exactly as on the lock screen — is visible and can
// take input; it shrinks back to the strip when the overlay closes, so the rest
// of the screen passes clicks through to the apps below.

#pragma once

#include <wayland-client.h>

#include <cstdint>
#include <memory>
#include <vector>

#include "wayland/Output.hpp"  // OutputEnv
#include "wayland/ShmBuffer.hpp"

struct zwlr_layer_shell_v1;
struct zwlr_layer_surface_v1;

namespace qypr {

class BarWindow {
public:
    BarWindow(wl_output* output, uint32_t name, OutputEnv* env, int reservedHeight);
    ~BarWindow();

    BarWindow(const BarWindow&) = delete;
    BarWindow& operator=(const BarWindow&) = delete;

    // Create the layer surface, anchor it, and reserve its exclusive zone. The
    // shell is passed here (not the ctor) so the window is constructed — and its
    // wl_output listener attached — before the compositor streams mode/scale.
    void createLayerSurface(zwlr_layer_shell_v1* shell);

    // Grow to full-output height while an overlay is open, shrink back to the
    // reserved strip when it closes (governs both visibility and input grab).
    void setOverlayActive(bool active);

    // Mark dirty and repaint as soon as the compositor allows.
    void invalidate();

    uint32_t name() const { return name_; }
    wl_surface* surface() const { return surface_; }
    int logicalWidth() const { return width_; }
    int logicalHeight() const { return height_; }

    // Wayland C callbacks (public so file-scope listener tables can bind them).
    static void onGeometry(void*, wl_output*, int32_t, int32_t, int32_t, int32_t, int32_t,
                           const char*, const char*, int32_t);
    static void onMode(void*, wl_output*, uint32_t, int32_t, int32_t, int32_t);
    static void onDone(void*, wl_output*);
    static void onScale(void*, wl_output*, int32_t);
    static void onName(void*, wl_output*, const char*);
    static void onDescription(void*, wl_output*, const char*);
    static void onConfigure(void*, zwlr_layer_surface_v1*, uint32_t, uint32_t, uint32_t);
    static void onClosed(void*, zwlr_layer_surface_v1*);
    static void onFrame(void*, wl_callback*, uint32_t);

private:
    void render();
    void requestHeight(int logicalH);
    ShmBuffer* acquireBuffer(int pxW, int pxH);

    wl_output* output_ = nullptr;
    uint32_t name_ = 0;
    OutputEnv* env_ = nullptr;
    zwlr_layer_shell_v1* shell_ = nullptr;

    wl_surface* surface_ = nullptr;
    zwlr_layer_surface_v1* layerSurface_ = nullptr;
    wl_callback* frameCallback_ = nullptr;

    std::vector<std::unique_ptr<ShmBuffer>> buffers_;

    int reservedHeight_ = 0;    // exclusive zone + idle strip height (logical)
    int outputHeight_ = 0;      // full output height (logical), for overlays
    int requestedHeight_ = 0;   // current set_size height (logical)
    bool overlayActive_ = false;

    int width_ = 0;      // logical, from configure
    int height_ = 0;     // logical, from configure
    int scale_ = 1;
    bool configured_ = false;
    bool dirty_ = true;
    bool framePending_ = false;
};

}  // namespace qypr
