// IconResolver.hpp - Memory-efficient app icon loader for notification tiles.
//
// Resolves freedesktop icon names (e.g. "firefox", "org.gnome.Calendar") to
// PNG files via XDG icon theme directories, loads them with cairo's built-in
// PNG support (no extra dependencies), and caches surfaces with bounded memory
// usage. Also handles file:// and data: URIs from the D-Bus app_icon field.

#pragma once

#include <cairo/cairo.h>

#include <string>
#include <unordered_map>

namespace qypr {

class IconResolver {
public:
    IconResolver();
    ~IconResolver();

    IconResolver(const IconResolver&) = delete;
    IconResolver& operator=(const IconResolver&) = delete;

    // Look up an icon by its freedesktop name or URI. Returns a cached surface
    // (caller must NOT unref — the cache owns it) or nullptr if not found.
    // The surface is sized for notification tiles (~48px).
    cairo_surface_t* get(const std::string& icon);

    // Singleton access — the App owns the instance.
    static IconResolver& instance();
    static void setInstance(IconResolver* r);

private:
    static constexpr size_t kMaxCached = 32;

    cairo_surface_t* loadPng(const std::string& path);
    cairo_surface_t* loadDataUri(const std::string& uri);
    cairo_surface_t* resolveName(const std::string& name);
    std::string findFile(const std::string& name);

    std::unordered_map<std::string, cairo_surface_t*> cache_;
    static IconResolver* instance_;
};

}  // namespace qypr
