// IconResolver.cpp - See the header for the design.

#include "ui/IconResolver.hpp"

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

    static const char* kSizes[] = {"16x16", "22x22", "24x24", "32x32",
                                   "48x48", "64x64", "96x96", "128x128"};

    auto tryDir = [&](const std::string& base) -> std::string {
        {
            std::string p = base + "/scalable/apps/" + name + ".svg";
            FILE* f = std::fopen(p.c_str(), "r");
            if (f) { std::fclose(f); return p; }
        }
        for (const char* sz : kSizes) {
            std::string p = base + "/" + sz + "/apps/" + name + ".png";
            FILE* f = std::fopen(p.c_str(), "r");
            if (f) { std::fclose(f); return p; }
        }
        {
            std::string p = base + "/" + name + ".png";
            FILE* f = std::fopen(p.c_str(), "r");
            if (f) { std::fclose(f); return p; }
        }
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
    if (!path.empty()) return loadPng(path);

    if (auto pos = name.rfind('.'); pos != std::string::npos && pos > 0) {
        std::string simple = name.substr(pos + 1);
        path = findFile(simple);
        if (!path.empty()) return loadPng(path);

        auto dot2 = name.find('.');
        if (dot2 != std::string::npos && dot2 < pos) {
            std::string kde = name.substr(dot2 + 1);
            std::transform(kde.begin(), kde.end(), kde.begin(),
                           [](unsigned char c) { return c == '.' ? '-' : c; });
            path = findFile(kde);
            if (!path.empty()) return loadPng(path);
        }
    }

    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return c == '.' ? '-' : std::tolower(c); });
    path = findFile(lower);
    if (!path.empty()) return loadPng(path);

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
        s = loadPng(icon.substr(7));
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
