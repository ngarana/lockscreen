// ToplevelBackend.cpp - wlr-foreign-toplevel-management client (push-driven).
#include "system/ToplevelBackend.hpp"

#include <wayland-client.h>

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "wlr-foreign-toplevel-management-unstable-v1-client-protocol.h"

namespace qypr {

namespace {

// --- zwlr_foreign_toplevel_handle_v1 listener trampolines ---
void tlTitle(void* data, zwlr_foreign_toplevel_handle_v1*, const char* title) {
    auto* h = static_cast<TlHandle*>(data);
    h->backend->onHandleTitle(h, title);
}
void tlAppId(void* data, zwlr_foreign_toplevel_handle_v1*, const char* appId) {
    auto* h = static_cast<TlHandle*>(data);
    h->backend->onHandleAppId(h, appId);
}
void tlOutputEnter(void*, zwlr_foreign_toplevel_handle_v1*, wl_output*) {}
void tlOutputLeave(void*, zwlr_foreign_toplevel_handle_v1*, wl_output*) {}
void tlState(void* data, zwlr_foreign_toplevel_handle_v1*, wl_array* states) {
    auto* h = static_cast<TlHandle*>(data);
    h->backend->onHandleState(h, static_cast<const uint32_t*>(states->data),
                              states->size / sizeof(uint32_t));
}
void tlDone(void* data, zwlr_foreign_toplevel_handle_v1*) {
    auto* h = static_cast<TlHandle*>(data);
    h->backend->onHandleDone(h);
}
void tlClosed(void* data, zwlr_foreign_toplevel_handle_v1*) {
    auto* h = static_cast<TlHandle*>(data);
    h->backend->onHandleClosed(h);
}
void tlParent(void*, zwlr_foreign_toplevel_handle_v1*, zwlr_foreign_toplevel_handle_v1*) {}
const zwlr_foreign_toplevel_handle_v1_listener kHandleListener = {
    tlTitle, tlAppId, tlOutputEnter, tlOutputLeave, tlState, tlDone, tlClosed, tlParent};

// --- zwlr_foreign_toplevel_manager_v1 listener trampolines ---
void managerToplevel(void* data, zwlr_foreign_toplevel_manager_v1*,
                     zwlr_foreign_toplevel_handle_v1* h) {
    static_cast<ToplevelBackend*>(data)->onManagerToplevel(h);
}
void managerFinished(void*, zwlr_foreign_toplevel_manager_v1*) {}
const zwlr_foreign_toplevel_manager_v1_listener kManagerListener = {managerToplevel,
                                                                    managerFinished};

// --- registry ---
constexpr uint32_t kWantVersion = 3;

void registryGlobal(void* data, wl_registry* reg, uint32_t name, const char* iface,
                    uint32_t version) {
    if (std::strcmp(iface, zwlr_foreign_toplevel_manager_v1_interface.name) == 0) {
        auto** mgr = static_cast<zwlr_foreign_toplevel_manager_v1**>(data);
        uint32_t bind = version < kWantVersion ? version : kWantVersion;
        *mgr = static_cast<zwlr_foreign_toplevel_manager_v1*>(
            wl_registry_bind(reg, name, &zwlr_foreign_toplevel_manager_v1_interface, bind));
    }
}
void registryGlobalRemove(void*, wl_registry*, uint32_t) {}
const wl_registry_listener kRegistryListener = {registryGlobal, registryGlobalRemove};

}  // namespace

ToplevelBackend::~ToplevelBackend() {
    for (auto& h : handles_) {
        if (h->handle) zwlr_foreign_toplevel_handle_v1_destroy(h->handle);
    }
    if (manager_) zwlr_foreign_toplevel_manager_v1_destroy(manager_);
    if (registry_) wl_registry_destroy(registry_);
}

bool ToplevelBackend::start(wl_display* display) {
    if (!display) return false;
    display_ = display;

    registry_ = wl_display_get_registry(display);
    wl_registry_add_listener(registry_, &kRegistryListener, &manager_);
    wl_display_roundtrip(display);  // one startup sync: discover + bind
    if (!manager_) {
        std::fprintf(stderr,
                     "qypr: no wlr-foreign-toplevel-management; active-window indicator disabled\n");
        return false;
    }
    zwlr_foreign_toplevel_manager_v1_add_listener(manager_, &kManagerListener, this);
    wl_display_roundtrip(display);  // pull the initial toplevel set

    snap_.available = true;
    rebuildAndNotify();
    return true;
}

void ToplevelBackend::onManagerToplevel(zwlr_foreign_toplevel_handle_v1* h) {
    auto t = std::make_unique<TlHandle>();
    t->backend = this;
    t->handle = h;
    zwlr_foreign_toplevel_handle_v1_add_listener(h, &kHandleListener, t.get());
    handles_.push_back(std::move(t));
}

void ToplevelBackend::onHandleTitle(TlHandle* h, const char* title) {
    h->title = title ? title : "";
}

void ToplevelBackend::onHandleAppId(TlHandle* h, const char* appId) {
    h->appId = appId ? appId : "";
}

void ToplevelBackend::onHandleState(TlHandle* h, const uint32_t* states, size_t n) {
    h->active = false;
    for (size_t i = 0; states && i < n; ++i) {
        if (states[i] == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED) h->active = true;
    }
}

void ToplevelBackend::onHandleDone(TlHandle*) {
    // A handle's properties are consistent after 'done'; recompute the focused
    // window (only notifies when the active app id / title actually changed).
    rebuildAndNotify();
}

void ToplevelBackend::onHandleClosed(TlHandle* h) {
    if (h->handle) {
        zwlr_foreign_toplevel_handle_v1_destroy(h->handle);
        h->handle = nullptr;
    }
    handles_.erase(std::remove_if(handles_.begin(), handles_.end(),
                                  [&](const std::unique_ptr<TlHandle>& p) { return p.get() == h; }),
                   handles_.end());
    rebuildAndNotify();
}

void ToplevelBackend::rebuildAndNotify() {
    ToplevelSnapshot next;
    next.available = snap_.available;
    // The most recently focused toplevel wins if several report activated
    // (during a focus handoff both may briefly carry the bit).
    for (const auto& h : handles_) {
        if (h->active) {
            next.hasActive = true;
            next.appId = h->appId;
            next.title = h->title;
        }
    }

    if (next == snap_) return;
    snap_ = std::move(next);
    if (onChange_) onChange_();
}

}  // namespace qypr
