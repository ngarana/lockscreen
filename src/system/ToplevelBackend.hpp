// ToplevelBackend.hpp - The focused window via wlr-foreign-toplevel-management.
//
// Tracks open toplevels and exposes the *activated* (focused) one's app id and
// title for an "active window" bar widget. Push only: the compositor streams
// title/app_id/state/closed events on the existing display fd.
//
// Protocol note: this is wlr-foreign-toplevel-management (widely supported —
// Hyprland, Sway, all wlroots compositors), used because it is the only broadly
// available protocol that carries per-window *focus* state. The standard
// ext-foreign-toplevel-list-v1 lists windows but has no activated/focus state,
// so it cannot answer "which window is focused". Still no per-WM IPC.
//
// Like WorkspaceBackend, it binds its own registry on the host wl_display, so
// the same class serves the lockscreen and the future standalone qypr-bar.
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

struct wl_display;
struct wl_registry;
struct zwlr_foreign_toplevel_manager_v1;
struct zwlr_foreign_toplevel_handle_v1;

namespace qypr {

struct ToplevelSnapshot {
    bool available = false;  // the compositor exposes the protocol
    bool hasActive = false;  // some toplevel is currently activated
    std::string appId;
    std::string title;

    bool operator==(const ToplevelSnapshot&) const = default;
};

// Per-handle state (public for the C listener trampolines).
struct TlHandle {
    class ToplevelBackend* backend = nullptr;
    zwlr_foreign_toplevel_handle_v1* handle = nullptr;
    std::string appId;
    std::string title;
    bool active = false;
};

class ToplevelBackend {
public:
    ToplevelBackend() = default;
    ~ToplevelBackend();

    ToplevelBackend(const ToplevelBackend&) = delete;
    ToplevelBackend& operator=(const ToplevelBackend&) = delete;

    // Bind the protocol on the given display and take one startup sync.
    // Returns false when the compositor does not expose it.
    bool start(wl_display* display);

    const ToplevelSnapshot& snapshot() const { return snap_; }
    void setOnChange(std::function<void()> cb) { onChange_ = std::move(cb); }

    // --- C listener trampolines (public; not for external use) ---
    void onManagerToplevel(zwlr_foreign_toplevel_handle_v1* h);
    void onHandleTitle(TlHandle*, const char* title);
    void onHandleAppId(TlHandle*, const char* appId);
    void onHandleState(TlHandle*, const uint32_t* states, size_t n);
    void onHandleDone(TlHandle*);
    void onHandleClosed(TlHandle*);

private:
    void rebuildAndNotify();

    wl_display* display_ = nullptr;
    wl_registry* registry_ = nullptr;
    zwlr_foreign_toplevel_manager_v1* manager_ = nullptr;
    std::vector<std::unique_ptr<TlHandle>> handles_;
    ToplevelSnapshot snap_;
    std::function<void()> onChange_;
};

}  // namespace qypr
