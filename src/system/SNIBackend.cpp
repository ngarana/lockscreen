// SNIBackend.cpp - StatusNotifierItem host implementation (push-driven).
#include "system/SNIBackend.hpp"

#include <systemd/sd-bus.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <cstring>

#include "system/SystemBus.hpp"

namespace qypr {

namespace {
constexpr const char* kWatcher = "org.kde.StatusNotifierWatcher";
constexpr const char* kWatcherPath = "/StatusNotifierWatcher";
constexpr const char* kWatcherIface = "org.kde.StatusNotifierWatcher";
constexpr const char* kItemIface = "org.kde.StatusNotifierItem";
constexpr const char* kPropsIface = "org.freedesktop.DBus.Properties";

// Read a variant known to hold a string. Consumes the variant either way so
// the caller can close the surrounding dict-entry container.
bool readVariantString(sd_bus_message* m, std::string* out) {
    if (sd_bus_message_enter_container(m, 'v', "s") < 0) {
        sd_bus_message_skip(m, "v");
        return false;
    }
    const char* s = nullptr;
    if (sd_bus_message_read_basic(m, 's', &s) >= 0 && s) *out = s;
    sd_bus_message_exit_container(m);
    return true;
}

// Same, for a variant holding an object path (the item's Menu property).
bool readVariantObjectPath(sd_bus_message* m, std::string* out) {
    if (sd_bus_message_enter_container(m, 'v', "o") < 0) {
        sd_bus_message_skip(m, "v");
        return false;
    }
    const char* s = nullptr;
    if (sd_bus_message_read_basic(m, 'o', &s) >= 0 && s) *out = s;
    sd_bus_message_exit_container(m);
    return true;
}

// ARGB32 in network byte order (bytes A,R,G,B per the SNI spec) → a cairo
// CAIRO_FORMAT_ARGB32 surface (native-endian, premultiplied).
cairo_surface_t* pixmapToSurface(const unsigned char* argb, int w, int h) {
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    if (cairo_surface_status(surf) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(surf);
        return nullptr;
    }
    cairo_surface_flush(surf);
    unsigned char* dst = cairo_image_surface_get_data(surf);
    int stride = cairo_image_surface_get_stride(surf);
    for (int y = 0; y < h; ++y) {
        auto* row = reinterpret_cast<uint32_t*>(dst + y * stride);
        const unsigned char* src = argb + static_cast<size_t>(y) * w * 4;
        for (int x = 0; x < w; ++x) {
            unsigned a = src[0], r = src[1], g = src[2], b = src[3];
            src += 4;
            // Premultiply into cairo's expected 0xAARRGGBB layout.
            r = r * a / 255;
            g = g * a / 255;
            b = b * a / 255;
            row[x] = (a << 24) | (r << 16) | (g << 8) | b;
        }
    }
    cairo_surface_mark_dirty(surf);
    return surf;
}

// Read a variant known to hold a(iiay) and return the largest pixmap as a
// cairo surface (owned by the caller), or nullptr. Consumes the variant.
cairo_surface_t* readVariantPixmap(sd_bus_message* m) {
    if (sd_bus_message_enter_container(m, 'v', "a(iiay)") < 0) {
        sd_bus_message_skip(m, "v");
        return nullptr;
    }
    cairo_surface_t* best = nullptr;
    int bestW = 0;
    if (sd_bus_message_enter_container(m, 'a', "(iiay)") >= 0) {
        while (sd_bus_message_enter_container(m, 'r', "iiay") > 0) {
            int32_t w = 0, h = 0;
            sd_bus_message_read(m, "ii", &w, &h);
            const void* data = nullptr;
            size_t len = 0;
            sd_bus_message_read_array(m, 'y', &data, &len);
            sd_bus_message_exit_container(m);  // (iiay)
            if (w > 0 && h > 0 && data && len >= static_cast<size_t>(w) * h * 4 && w > bestW) {
                cairo_surface_t* s = pixmapToSurface(static_cast<const unsigned char*>(data), w, h);
                if (s) {
                    if (best) cairo_surface_destroy(best);
                    best = s;
                    bestW = w;
                }
            }
        }
        sd_bus_message_exit_container(m);  // array
    }
    sd_bus_message_exit_container(m);  // variant
    return best;
}
}  // namespace

SNIBackend::SNIBackend(SystemBus& bus) : bus_(bus) {}

SNIBackend::~SNIBackend() {
    if (regSlot_) sd_bus_slot_unref(regSlot_);
    if (unregSlot_) sd_bus_slot_unref(unregSlot_);
    if (itemSlot_) sd_bus_slot_unref(itemSlot_);
    if (watcherSlot_) sd_bus_slot_unref(watcherSlot_);
    clearItems();
}

void SNIBackend::parseItemRef(const std::string& ref, std::string& service, std::string& path) {
    auto slash = ref.find('/');
    if (slash == std::string::npos) {
        service = ref;
        path = "/StatusNotifierItem";
    } else {
        service = ref.substr(0, slash);
        path = ref.substr(slash);
    }
}

bool SNIBackend::start() {
    if (!bus_.available()) return false;

    hostName_ = "org.kde.StatusNotifierHost-" + std::to_string(getpid()) + "-1";
    registerHost();

    // Watcher item add/remove.
    regSlot_ = bus_.addMatch(
        "type='signal',interface='org.kde.StatusNotifierWatcher',"
        "member='StatusNotifierItemRegistered'",
        &SNIBackend::onItemRegistered, this);
    unregSlot_ = bus_.addMatch(
        "type='signal',interface='org.kde.StatusNotifierWatcher',"
        "member='StatusNotifierItemUnregistered'",
        &SNIBackend::onItemUnregistered, this);
    // Per-item NewIcon/NewTitle/NewStatus/… — the handler refetches only the
    // signalling item (matched by sender+path), so this is not a storm.
    itemSlot_ = bus_.addMatch("type='signal',interface='org.kde.StatusNotifierItem'",
                              &SNIBackend::onItemChanged, this);
    // Watcher (re)appearing (e.g. bar restart) → re-register and re-sync.
    watcherSlot_ = bus_.addMatch(
        "type='signal',sender='org.freedesktop.DBus',interface='org.freedesktop.DBus',"
        "member='NameOwnerChanged',arg0='org.kde.StatusNotifierWatcher'",
        &SNIBackend::onWatcherOwnerChanged, this);

    refresh();
    // Host mode never truly fails: items may arrive later. Keep the indicator
    // alive (it stays hidden until items_ is non-empty).
    return true;
}

void SNIBackend::registerHost() {
    sd_bus* bus = bus_.get();
    if (!bus) return;
    // Own the well-known host name (best-effort) and tell the watcher.
    sd_bus_request_name(bus, hostName_.c_str(), 0);
    sd_bus_call_method_async(bus, nullptr, kWatcher, kWatcherPath, kWatcherIface,
                             "RegisterStatusNotifierHost", nullptr, nullptr, "s",
                             hostName_.c_str());
}

int SNIBackend::onItemRegistered(sd_bus_message*, void* ud, sd_bus_error*) {
    static_cast<SNIBackend*>(ud)->refresh();
    return 0;
}

int SNIBackend::onItemUnregistered(sd_bus_message*, void* ud, sd_bus_error*) {
    static_cast<SNIBackend*>(ud)->refresh();
    return 0;
}

int SNIBackend::onItemChanged(sd_bus_message* m, void* ud, sd_bus_error*) {
    auto* self = static_cast<SNIBackend*>(ud);
    const char* sender = sd_bus_message_get_sender(m);
    const char* path = sd_bus_message_get_path(m);
    if (!sender || !path) return 0;

    for (auto& it : self->items_) {
        if (it.service == sender && it.path == path) {
            SNIItem fresh = self->fetchItem(it.service, it.path);
            if (it.pixmap) cairo_surface_destroy(it.pixmap);
            it = std::move(fresh);  // raw pixmap ptr transfers; no dtor, no double free
            self->notify();
            return 0;
        }
    }
    // Signal from an item we do not track yet (registration race): full resync.
    self->refresh();
    return 0;
}

int SNIBackend::onWatcherOwnerChanged(sd_bus_message* m, void* ud, sd_bus_error*) {
    auto* self = static_cast<SNIBackend*>(ud);
    const char *name = nullptr, *oldOwner = nullptr, *newOwner = nullptr;
    sd_bus_message_read(m, "sss", &name, &oldOwner, &newOwner);
    if (newOwner && *newOwner) self->registerHost();
    self->refresh();
    return 0;
}

void SNIBackend::refresh() {
    sd_bus* bus = bus_.get();
    if (!bus) {
        clearItems();
        notify();
        return;
    }

    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    int r = sd_bus_get_property(bus, kWatcher, kWatcherPath, kWatcherIface,
                               "RegisteredStatusNotifierItems", &err, &reply, "as");
    sd_bus_error_free(&err);
    if (r < 0 || !reply) {
        // No watcher (or none yet): clear to empty.
        if (reply) sd_bus_message_unref(reply);
        clearItems();
        notify();
        return;
    }

    std::vector<SNIItem> next;
    if (sd_bus_message_enter_container(reply, 'a', "s") >= 0) {
        const char* ref = nullptr;
        while (sd_bus_message_read(reply, "s", &ref) > 0 && ref) {
            std::string service, path;
            parseItemRef(ref, service, path);
            next.push_back(fetchItem(service, path));
        }
        sd_bus_message_exit_container(reply);
    }
    sd_bus_message_unref(reply);

    clearItems();
    items_ = std::move(next);
    notify();
}

SNIItem SNIBackend::fetchItem(const std::string& service, const std::string& path) {
    SNIItem item;
    item.service = service;
    item.path = path;

    sd_bus* bus = bus_.get();
    if (!bus) return item;

    sd_bus_error err = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = nullptr;
    int r = sd_bus_call_method(bus, service.c_str(), path.c_str(), kPropsIface, "GetAll", &err,
                               &reply, "s", kItemIface);
    sd_bus_error_free(&err);
    if (r < 0 || !reply) {
        if (reply) sd_bus_message_unref(reply);
        return item;
    }

    if (sd_bus_message_enter_container(reply, 'a', "{sv}") >= 0) {
        while (sd_bus_message_enter_container(reply, 'e', "sv") > 0) {
            const char* key = nullptr;
            if (sd_bus_message_read(reply, "s", &key) < 0 || !key) {
                sd_bus_message_skip(reply, "v");
                sd_bus_message_exit_container(reply);
                continue;
            }
            if (std::strcmp(key, "IconName") == 0) {
                readVariantString(reply, &item.iconName);
            } else if (std::strcmp(key, "Title") == 0) {
                readVariantString(reply, &item.title);
            } else if (std::strcmp(key, "Status") == 0) {
                readVariantString(reply, &item.status);
            } else if (std::strcmp(key, "IconPixmap") == 0) {
                item.pixmap = readVariantPixmap(reply);
            } else if (std::strcmp(key, "Menu") == 0) {
                readVariantObjectPath(reply, &item.menuPath);
            } else {
                sd_bus_message_skip(reply, "v");
            }
            sd_bus_message_exit_container(reply);
        }
        sd_bus_message_exit_container(reply);
    }
    sd_bus_message_unref(reply);
    return item;
}

void SNIBackend::activate(size_t index, int x, int y) {
    if (!bus_.available() || index >= items_.size()) return;
    const SNIItem& it = items_[index];
    sd_bus_call_method_async(bus_.get(), nullptr, it.service.c_str(), it.path.c_str(), kItemIface,
                             "Activate", nullptr, nullptr, "ii", x, y);
}

void SNIBackend::secondaryActivate(size_t index, int x, int y) {
    if (!bus_.available() || index >= items_.size()) return;
    const SNIItem& it = items_[index];
    sd_bus_call_method_async(bus_.get(), nullptr, it.service.c_str(), it.path.c_str(), kItemIface,
                             "SecondaryActivate", nullptr, nullptr, "ii", x, y);
}

void SNIBackend::clearItems() {
    for (auto& it : items_) {
        if (it.pixmap) cairo_surface_destroy(it.pixmap);
    }
    items_.clear();
}

}  // namespace qypr
