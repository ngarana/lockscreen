// IconResolver.hpp - Memory-efficient app icon loader for notification tiles.
//
// Resolves freedesktop icon names (e.g. "firefox", "org.gnome.Calendar") to
// icon files via XDG icon theme directories. PNGs load through cairo's built-in
// decoder; scalable/SVG icons are rasterized with librsvg. Surfaces are cached
// with bounded memory usage. Also handles absolute paths, file:// and data:
// URIs from the D-Bus app_icon field.

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
    cairo_surface_t* loadSvg(const std::string& path);
    cairo_surface_t* loadFile(const std::string& path);  // dispatch by extension
    cairo_surface_t* loadDataUri(const std::string& uri);
    cairo_surface_t* resolveName(const std::string& name);
    std::string findFile(const std::string& name);

    std::unordered_map<std::string, cairo_surface_t*> cache_;
    static IconResolver* instance_;
};

}  // namespace qypr
