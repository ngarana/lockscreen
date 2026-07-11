// IconResolver.cpp - See the header for the design.

#include "ui/IconResolver.hpp"

#include <librsvg/rsvg.h>

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <string>
#include <vector>

namespace qypr {

IconResolver* IconResolver::instance_ = nullptr;

IconResolver::IconResolver() = default;

IconResolver::~IconResolver() {
    for (auto& [_, s] : cache_) cairo_surface_destroy(s);
}

IconResolver& IconResolver::instance() {
    if (!instance_) {
        static IconResolver r;
        instance_ = &r;
    }
    return *instance_;
}

void IconResolver::setInstance(IconResolver* r) { instance_ = r; }

// ---------------------------------------------------------------------------
// PNG loading
// ---------------------------------------------------------------------------

cairo_surface_t* IconResolver::loadPng(const std::string& path) {
    cairo_surface_t* s = cairo_image_surface_create_from_png(path.c_str());
    if (cairo_surface_status(s) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(s);
        return nullptr;
    }
    return s;
}

// ---------------------------------------------------------------------------
// SVG loading (librsvg → rasterized cairo surface)
// ---------------------------------------------------------------------------

cairo_surface_t* IconResolver::loadSvg(const std::string& path) {
    GError* err = nullptr;
    RsvgHandle* handle = rsvg_handle_new_from_file(path.c_str(), &err);
    if (!handle) {
        if (err) g_error_free(err);
        return nullptr;
    }

    // Rasterize into a square tile-sized surface; drawSurface() scales the
    // result to the actual tile. Icons are square, so a square viewport keeps
    // aspect; anything non-square is centred rather than stretched.
    constexpr int kPx = 64;
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, kPx, kPx);
    if (cairo_surface_status(surf) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(surf);
        g_object_unref(handle);
        return nullptr;
    }

    double w = kPx, h = kPx;
    double iw = 0, ih = 0;
    if (rsvg_handle_get_intrinsic_size_in_pixels(handle, &iw, &ih) && iw > 0 && ih > 0) {
        double scale = std::min(kPx / iw, kPx / ih);
        w = iw * scale;
        h = ih * scale;
    }

    cairo_t* cr = cairo_create(surf);
    RsvgRectangle viewport{(kPx - w) / 2.0, (kPx - h) / 2.0, w, h};
    gboolean ok = rsvg_handle_render_document(handle, cr, &viewport, &err);
    cairo_destroy(cr);
    g_object_unref(handle);

    if (!ok) {
        if (err) g_error_free(err);
        cairo_surface_destroy(surf);
        return nullptr;
    }
    cairo_surface_flush(surf);
    return surf;
}

// Pick the decoder from the file extension.
cairo_surface_t* IconResolver::loadFile(const std::string& path) {
    auto endsWith = [&](const char* ext) {
        size_t n = std::strlen(ext);
        return path.size() >= n &&
               std::equal(path.end() - n, path.end(), ext,
                          [](char a, char b) { return std::tolower((unsigned char)a) == b; });
    };
    if (endsWith(".svg") || endsWith(".svgz")) return loadSvg(path);
    return loadPng(path);
}

// ---------------------------------------------------------------------------
// data: URI decoding (base64 PNG)
// ---------------------------------------------------------------------------

static int b64val(unsigned char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

static std::string base64Decode(const char* src, size_t len) {
    std::string out;
    out.reserve(len * 3 / 4);
    unsigned int buf = 0;
    int bits = 0;
    for (size_t i = 0; i < len; ++i) {
        int v = b64val(static_cast<unsigned char>(src[i]));
        if (v < 0) continue;
        buf = (buf << 6) | static_cast<unsigned int>(v);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out.push_back(static_cast<char>((buf >> bits) & 0xFF));
        }
    }
    return out;
}

struct PngReader {
    const char* ptr;
    size_t rem;
};

static cairo_status_t pngRead(void* closure, unsigned char* data, unsigned int length) {
    auto* r = static_cast<PngReader*>(closure);
    if (r->rem < length) return CAIRO_STATUS_READ_ERROR;
    std::memcpy(data, r->ptr, length);
    r->ptr += length;
    r->rem -= length;
    return CAIRO_STATUS_SUCCESS;
}

cairo_surface_t* IconResolver::loadDataUri(const std::string& uri) {
    const char* b64 = std::strstr(uri.c_str(), "base64,");
    if (!b64) return nullptr;
    b64 += 7;
    std::string raw = base64Decode(b64, std::strlen(b64));
    if (raw.size() < 8) return nullptr;

    // Check PNG magic bytes.
    if (static_cast<unsigned char>(raw[0]) != 0x89 || raw[1] != 'P' ||
        raw[2] != 'N' || raw[3] != 'G')
        return nullptr;

    PngReader r{raw.data(), raw.size()};
    cairo_surface_t* s = cairo_image_surface_create_from_png_stream(pngRead, &r);
    if (cairo_surface_status(s) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(s);
        return nullptr;
    }
    return s;
}

// ---------------------------------------------------------------------------
// XDG icon theme lookup
// ---------------------------------------------------------------------------

std::string IconResolver::findFile(const std::string& name) {
    static const char* kDirs[] = {
        "/usr/share/icons/hicolor",
        "/usr/share/icons/Adwaita",
        "/usr/share/pixmaps",
        nullptr,
    };

    std::vector<std::string> xdgDirs;
    if (const char* dataDirs = std::getenv("XDG_DATA_DIRS")) {
        std::string dirs(dataDirs);
        size_t pos = 0;
        while (pos < dirs.size()) {
            size_t next = dirs.find(':', pos);
            std::string d = (next != std::string::npos)
                                ? dirs.substr(pos, next - pos)
                                : dirs.substr(pos);
            xdgDirs.push_back(d + "/icons/hicolor");
            pos = (next != std::string::npos) ? next + 1 : dirs.size();
        }
    }

    // Notification tiles are ~48px, so prefer fixed sizes near that for a crisp
    // PNG. A scalable SVG (rasterized by loadSvg) is tried after the raster
    // sizes so an existing PNG still wins, but it now provides a fallback for
    // themes that ship apps only as SVG.
    static const char* kSizes[] = {"48x48", "64x64", "32x32", "96x96",
                                   "128x128", "24x24", "22x22", "16x16"};

    auto exists = [](const std::string& p) {
        FILE* f = std::fopen(p.c_str(), "r");
        if (f) { std::fclose(f); return true; }
        return false;
    };

    auto tryDir = [&](const std::string& base) -> std::string {
        for (const char* sz : kSizes) {
            std::string p = base + "/" + sz + "/apps/" + name + ".png";
            if (exists(p)) return p;
        }
        std::string svg = base + "/scalable/apps/" + name + ".svg";
        if (exists(svg)) return svg;
        std::string png = base + "/" + name + ".png";
        if (exists(png)) return png;
        std::string flatSvg = base + "/" + name + ".svg";
        if (exists(flatSvg)) return flatSvg;
        return {};
    };

    for (const char** d = kDirs; *d; ++d) {
        std::string r = tryDir(*d);
        if (!r.empty()) return r;
    }
    for (const auto& d : xdgDirs) {
        std::string r = tryDir(d);
        if (!r.empty()) return r;
    }
    return {};
}

// ---------------------------------------------------------------------------
// Name resolution
// ---------------------------------------------------------------------------

cairo_surface_t* IconResolver::resolveName(const std::string& name) {
    std::string path = findFile(name);
    if (!path.empty()) return loadFile(path);

    if (auto pos = name.rfind('.'); pos != std::string::npos && pos > 0) {
        std::string simple = name.substr(pos + 1);
        path = findFile(simple);
        if (!path.empty()) return loadFile(path);

        // Reverse-DNS app IDs (e.g. "org.chromium.Chromium") commonly map to a
        // lowercase icon file ("chromium.png"); try the last component lowered.
        std::string simpleLower = simple;
        std::transform(simpleLower.begin(), simpleLower.end(), simpleLower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (simpleLower != simple) {
            path = findFile(simpleLower);
            if (!path.empty()) return loadFile(path);
        }

        auto dot2 = name.find('.');
        if (dot2 != std::string::npos && dot2 < pos) {
            std::string kde = name.substr(dot2 + 1);
            std::transform(kde.begin(), kde.end(), kde.begin(),
                           [](unsigned char c) { return c == '.' ? '-' : c; });
            path = findFile(kde);
            if (!path.empty()) return loadFile(path);
        }
    }

    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return c == '.' ? '-' : std::tolower(c); });
    path = findFile(lower);
    if (!path.empty()) return loadFile(path);

    return nullptr;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

cairo_surface_t* IconResolver::get(const std::string& icon) {
    if (icon.empty()) return nullptr;

    if (auto it = cache_.find(icon); it != cache_.end()) return it->second;

    cairo_surface_t* s = nullptr;

    if (icon.rfind("data:", 0) == 0) {
        s = loadDataUri(icon);
    } else if (icon.rfind("file://", 0) == 0) {
        s = loadFile(icon.substr(7));
    } else if (icon[0] == '/') {
        // The freedesktop app_icon may be a full path rather than a themed
        // name (e.g. "/usr/lib/kitty/logo/kitty.png"). Load it directly;
        // resolveName() would only look it up as a theme name and miss it.
        s = loadFile(icon);
    } else {
        s = resolveName(icon);
    }

    if (s && cache_.size() >= kMaxCached) {
        auto it = cache_.begin();
        cairo_surface_destroy(it->second);
        cache_.erase(it);
    }

    if (s) cache_[icon] = s;
    return s;
}

}  // namespace qypr
