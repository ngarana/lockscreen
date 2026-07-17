// SNIBackend.hpp - StatusNotifierItem (system tray) host over the session bus.
//
// Implements the *host* side of the freedesktop/KDE StatusNotifierItem spec:
// it registers as a StatusNotifierHost with whatever StatusNotifierWatcher is
// already running (on this system waybar owns it), mirrors the watcher's
// RegisteredStatusNotifierItems list, and fetches each item's icon/title on
// demand. Everything is push: the watcher signals item add/remove, each item
// signals its own icon/title/status changes — no polling.
//
// Standard cross-desktop protocol only (org.kde.StatusNotifier* is *the*
// tray protocol on Wayland; it is not a specific daemon's private interface).
// Own-watcher mode (claiming the watcher name ourselves) is deferred to the
// standalone qypr-bar phase; here we always host against the existing watcher.

#pragma once

#include <cairo/cairo.h>

#include <functional>
#include <string>
#include <vector>

struct sd_bus_message;
struct sd_bus_slot;
struct sd_bus_error;

namespace qypr {

class SystemBus;

struct SNIItem {
    std::string service;   // owning bus name (e.g. ":1.51")
    std::string path;      // item object path (e.g. "/org/blueman/sni")
    std::string iconName;  // themed IconName ("" if the item ships only a pixmap)
    std::string title;     // Title (tooltip text)
    std::string status;    // "Active" | "Passive" | "NeedsAttention"

    // Best IconPixmap converted to a premultiplied cairo surface, or nullptr.
    // Owned by the backend (freed in clearItems / the destructor). Used only
    // when iconName does not resolve through the icon theme.
    cairo_surface_t* pixmap = nullptr;
};

class SNIBackend {
public:
    explicit SNIBackend(SystemBus& bus);
    ~SNIBackend();

    SNIBackend(const SNIBackend&) = delete;
    SNIBackend& operator=(const SNIBackend&) = delete;

    // Register as a StatusNotifierHost and start mirroring the watcher.
    // Returns false only when the session bus itself is unavailable; a running
    // host with no items yet is a success (the indicator stays hidden).
    bool start();

    const std::vector<SNIItem>& items() const { return items_; }

    // Fires whenever the item list or an item's icon/title/status changes.
    void setOnChange(std::function<void()> cb) { onChange_ = std::move(cb); }

    // Left-click activation. x,y are screen coordinates (the item may use them
    // to position its own menu). Fire-and-forget async call.
    void activate(size_t index, int x, int y);

    // Split a watcher item reference ("service/path" or a bare "service") into
    // its parts; the default path is "/StatusNotifierItem". Static + pure so
    // the parsing is unit-testable without a bus.
    static void parseItemRef(const std::string& ref, std::string& service, std::string& path);

private:
    static int onItemRegistered(sd_bus_message*, void*, sd_bus_error*);
    static int onItemUnregistered(sd_bus_message*, void*, sd_bus_error*);
    static int onItemChanged(sd_bus_message*, void*, sd_bus_error*);
    static int onWatcherOwnerChanged(sd_bus_message*, void*, sd_bus_error*);

    void registerHost();
    void refresh();  // re-read the watcher list, rebuild items_
    SNIItem fetchItem(const std::string& service, const std::string& path);
    void clearItems();
    void notify() {
        if (onChange_) onChange_();
    }

    SystemBus& bus_;
    std::string hostName_;
    sd_bus_slot* regSlot_ = nullptr;
    sd_bus_slot* unregSlot_ = nullptr;
    sd_bus_slot* itemSlot_ = nullptr;
    sd_bus_slot* watcherSlot_ = nullptr;
    std::vector<SNIItem> items_;
    std::function<void()> onChange_;
};

}  // namespace qypr
