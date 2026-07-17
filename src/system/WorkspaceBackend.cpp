// WorkspaceBackend.cpp - ext-workspace-v1 client (push-driven).
#include "system/WorkspaceBackend.hpp"

#include <wayland-client.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "ext-workspace-v1-client-protocol.h"

namespace qypr {

namespace {

// ext_workspace_handle_v1 state bits.
constexpr uint32_t kStateActive = 1;
constexpr uint32_t kStateUrgent = 2;
constexpr uint32_t kStateHidden = 4;

// --- ext_workspace_handle_v1 listener trampolines ---
void handleId(void*, ext_workspace_handle_v1*, const char*) {}
void handleName(void* data, ext_workspace_handle_v1*, const char* name) {
    auto* h = static_cast<WsHandle*>(data);
    h->backend->onHandleName(h, name);
}
void handleCoordinates(void* data, ext_workspace_handle_v1*, wl_array* coords) {
    auto* h = static_cast<WsHandle*>(data);
    h->backend->onHandleCoordinates(h, static_cast<const uint32_t*>(coords->data),
                                    coords->size / sizeof(uint32_t));
}
void handleState(void* data, ext_workspace_handle_v1*, uint32_t state) {
    auto* h = static_cast<WsHandle*>(data);
    h->backend->onHandleState(h, state);
}
void handleCapabilities(void*, ext_workspace_handle_v1*, uint32_t) {}
void handleRemoved(void* data, ext_workspace_handle_v1*) {
    auto* h = static_cast<WsHandle*>(data);
    h->backend->onHandleRemoved(h);
}
const ext_workspace_handle_v1_listener kHandleListener = {
    handleId, handleName, handleCoordinates, handleState, handleCapabilities, handleRemoved};

// --- ext_workspace_manager_v1 listener trampolines ---
void managerGroup(void*, ext_workspace_manager_v1*, ext_workspace_group_handle_v1*) {}
void managerWorkspace(void* data, ext_workspace_manager_v1*, ext_workspace_handle_v1* ws) {
    static_cast<WorkspaceBackend*>(data)->onManagerWorkspace(ws);
}
void managerDone(void* data, ext_workspace_manager_v1*) {
    static_cast<WorkspaceBackend*>(data)->onManagerDone();
}
void managerFinished(void*, ext_workspace_manager_v1*) {}
const ext_workspace_manager_v1_listener kManagerListener = {managerGroup, managerWorkspace,
                                                            managerDone, managerFinished};

// --- registry ---
void registryGlobal(void* data, wl_registry* reg, uint32_t name, const char* iface,
                    uint32_t version) {
    if (std::strcmp(iface, ext_workspace_manager_v1_interface.name) == 0) {
        auto** mgr = static_cast<ext_workspace_manager_v1**>(data);
        *mgr = static_cast<ext_workspace_manager_v1*>(
            wl_registry_bind(reg, name, &ext_workspace_manager_v1_interface, 1));
    }
}
void registryGlobalRemove(void*, wl_registry*, uint32_t) {}
const wl_registry_listener kRegistryListener = {registryGlobal, registryGlobalRemove};

}  // namespace

WorkspaceBackend::~WorkspaceBackend() {
    for (auto& h : handles_) {
        if (h->handle) ext_workspace_handle_v1_destroy(h->handle);
    }
    if (manager_) ext_workspace_manager_v1_destroy(manager_);
    if (registry_) wl_registry_destroy(registry_);
}

bool WorkspaceBackend::start(wl_display* display) {
    if (!display) return false;
    display_ = display;

    registry_ = wl_display_get_registry(display);
    wl_registry_add_listener(registry_, &kRegistryListener, &manager_);
    // One startup sync: discover + bind the global, then pull the initial
    // workspace set (create + name/state + done). Subsequent updates are
    // delivered by the host's normal dispatch — no polling.
    wl_display_roundtrip(display);
    if (!manager_) {
        std::fprintf(stderr, "qypr: no ext-workspace-v1; workspaces indicator disabled\n");
        return false;
    }
    ext_workspace_manager_v1_add_listener(manager_, &kManagerListener, this);
    wl_display_roundtrip(display);

    snap_.available = true;
    rebuildAndNotify();
    return true;
}

void WorkspaceBackend::onManagerWorkspace(ext_workspace_handle_v1* ws) {
    auto h = std::make_unique<WsHandle>();
    h->backend = this;
    h->handle = ws;
    ext_workspace_handle_v1_add_listener(ws, &kHandleListener, h.get());
    handles_.push_back(std::move(h));
}

void WorkspaceBackend::onHandleName(WsHandle* h, const char* name) {
    h->name = name ? name : "";
}

void WorkspaceBackend::onHandleCoordinates(WsHandle* h, const uint32_t* coords, size_t n) {
    if (n > 0 && coords) {
        h->coord = coords[0];
        h->hasCoord = true;
    } else {
        h->hasCoord = false;
    }
}

void WorkspaceBackend::onHandleState(WsHandle* h, uint32_t state) {
    h->active = state & kStateActive;
    h->urgent = state & kStateUrgent;
    h->hidden = state & kStateHidden;
}

void WorkspaceBackend::onHandleRemoved(WsHandle* h) {
    if (h->handle) {
        ext_workspace_handle_v1_destroy(h->handle);
        h->handle = nullptr;
    }
    handles_.erase(std::remove_if(handles_.begin(), handles_.end(),
                                  [&](const std::unique_ptr<WsHandle>& p) { return p.get() == h; }),
                   handles_.end());
    // 'removed' arrives outside a manager transaction; refresh immediately.
    rebuildAndNotify();
}

void WorkspaceBackend::onManagerDone() { rebuildAndNotify(); }

void WorkspaceBackend::rebuildAndNotify() {
    WorkspaceSnapshot next;
    next.available = snap_.available;

    std::vector<const WsHandle*> visible;
    for (const auto& h : handles_) {
        if (h->hidden) continue;  // spec: hidden workspaces must not be displayed
        visible.push_back(h.get());
    }
    // Order by coordinate when the compositor supplies one, else keep a stable,
    // numeric-name-aware order so "1 2 … 10" reads naturally.
    auto sortKey = [](const WsHandle* h) -> long {
        if (h->hasCoord) return static_cast<long>(h->coord);
        char* end = nullptr;
        long n = std::strtol(h->name.c_str(), &end, 10);
        return (end && *end == '\0' && !h->name.empty()) ? n : 1'000'000L;
    };
    std::stable_sort(visible.begin(), visible.end(),
                     [&](const WsHandle* a, const WsHandle* b) { return sortKey(a) < sortKey(b); });

    for (const WsHandle* h : visible) {
        next.workspaces.push_back({h->name, h->active, h->urgent});
    }

    if (next == snap_) return;
    snap_ = std::move(next);
    if (onChange_) onChange_();
}

void WorkspaceBackend::activate(const std::string& name) {
    if (!manager_) return;
    for (const auto& h : handles_) {
        if (h->handle && h->name == name) {
            ext_workspace_handle_v1_activate(h->handle);
            ext_workspace_manager_v1_commit(manager_);
            if (display_) wl_display_flush(display_);
            return;
        }
    }
}

}  // namespace qypr
