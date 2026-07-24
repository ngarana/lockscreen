// VolumeIndicator.cpp - Status bar volume indicator implementation.
#include "ui/indicators/VolumeIndicator.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "render/Painter.hpp"
#include "ui/Theme.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"
#include "ui/statusbar/QSTile.hpp"

namespace qypr {

namespace {
constexpr double kScrollStep = 0.05;  // ±5% per scroll tick

const char* volumeIcon(const VolumeSnapshot& s) {
    if (s.muted || s.level <= 0.001) return "󰝟";
    if (s.level >= 0.66) return "󰕾";
    if (s.level >= 0.33) return "󰖀";
    return "󰕿";
}

// Audio panel: output-device picker + per-app stream sliders (Phase 14).
constexpr double kAW = 320.0;
constexpr double kAPad = 12.0;
constexpr double kSecHdr = 22.0;
constexpr double kSinkH = 30.0;
constexpr double kStreamH = 46.0;
constexpr double kDiv = 12.0;

class AudioPopover : public DetailedPopover {
public:
    // `showStreams` is false on the lock screen: per-app names would disclose
    // what is running, so only device switching is offered there.
    AudioPopover(VolumeBackend* backend, bool showStreams)
        : backend_(backend), showStreams_(showStreams) {}

    double contentWidth() const override { return kAW; }
    double contentHeight() const override {
        const size_t nSinks = std::max<size_t>(backend_->sinks().size(), 1);
        double h = kAPad * 2 + kSecHdr + nSinks * kSinkH;
        if (showStreams_ && !backend_->streams().empty()) {
            h += kDiv + kSecHdr + backend_->streams().size() * kStreamH;
        }
        return h;
    }

    void draw(Painter& p, int64_t now) override {
        Rect b = getBounds();
        b.y += (growUp ? 1.0 : -1.0) * (1.0 - openProgress_.value(now)) * 6.0;
        p.fillRoundedRect(b, theme::statusbar::popoverRadius, theme::color::surface);

        sinkRows_.clear();
        streamTracks_.clear();
        double y = b.y + kAPad;

        // ── Output devices ───────────────────────────────────────────────────
        TextStyle hdr{theme::font::family, 11.0, PANGO_WEIGHT_BOLD, theme::color::textSubtle};
        p.drawText(b.x + kAPad, y, "OUTPUT", hdr);
        y += kSecHdr;

        const auto& sinks = backend_->sinks();
        if (sinks.empty()) {
            TextStyle e{theme::font::family, 12.0, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
            p.drawText(b.x + kAPad, y + 6.0, "No output devices", e);
            y += kSinkH;
        }
        for (const auto& s : sinks) {
            const Rect row{b.x + kAPad, y, b.w - kAPad * 2, kSinkH};
            if (row.contains(hoverX_, hoverY_))
                p.fillRoundedRect(row, 6.0, theme::color::glassHover);
            TextStyle dot{theme::font::family, 12.0, PANGO_WEIGHT_NORMAL,
                          s.isDefault ? theme::color::primary : theme::color::textSubtle};
            p.drawText(row.x + 4.0, y + (kSinkH - 14.0) / 2.0, s.isDefault ? "●" : "○", dot);
            TextStyle ls{theme::font::family, 12.0,
                         s.isDefault ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL,
                         theme::color::text};
            p.drawText(row.x + 24.0, y + (kSinkH - 14.0) / 2.0, s.description, ls, HAlign::Left,
                       row.w - 28.0);
            sinkRows_.push_back({row, s.name});
            y += kSinkH;
        }

        // ── Per-app streams (bar only) ───────────────────────────────────────
        const auto& streams = backend_->streams();
        if (showStreams_ && !streams.empty()) {
            y += kDiv;  // gap after the device list
            p.fillRect({b.x + kAPad, y - 6.0, b.w - kAPad * 2, 1.0}, theme::color::glassBorder);
            p.drawText(b.x + kAPad, y, "APPLICATIONS", hdr);
            y += kSecHdr;
            for (const auto& s : streams) {
                // Name + percentage.
                TextStyle ns{theme::font::family, 12.0, PANGO_WEIGHT_NORMAL, theme::color::text};
                p.drawText(b.x + kAPad, y + 2.0, s.appName, ns, HAlign::Left, b.w - kAPad * 2 - 48.0);
                const int pct = static_cast<int>(std::round(s.level * 100.0));
                TextStyle ps{theme::font::family, 11.0, PANGO_WEIGHT_NORMAL, theme::color::textSubtle};
                p.drawText(b.x + b.w - kAPad, y + 2.0, std::to_string(pct) + "%", ps, HAlign::Right);

                // Slider track.
                const double tx = b.x + kAPad;
                const double tw = b.w - kAPad * 2;
                const double ty = y + 26.0;
                const Rect track{tx, ty, tw, 4.0};
                p.fillRoundedRect(track, 2.0, theme::color::surface);
                Rect fill = track;
                fill.w = tw * std::clamp(s.level, 0.0, 1.0);
                const Color fg = s.muted ? theme::color::textSubtle : theme::color::primary;
                p.fillRoundedRect(fill, 2.0, fg);
                p.fillCircle(tx + fill.w, ty + 2.0, 6.0, fg);
                streamTracks_.push_back({track, s.index});
                y += kStreamH;
            }
        }
    }

    bool handleClick(double x, double y) override {
        for (const auto& r : sinkRows_) {
            if (r.rect.contains(x, y)) {
                backend_->setDefaultSink(r.name);
                return true;
            }
        }
        return dragStream(x, y);
    }

    bool handleDrag(double x, double y) override {
        hoverX_ = x;
        hoverY_ = y;
        return dragStream(x, y);
    }

private:
    struct SinkRow {
        Rect rect;
        std::string name;
    };
    struct StreamTrack {
        Rect track;
        uint32_t index;
    };

    // A press/drag on (or near) a stream's track sets that stream's volume.
    bool dragStream(double x, double y) {
        for (const auto& t : streamTracks_) {
            Rect hit = t.track;
            hit.y -= 12.0;
            hit.h += 24.0;  // generous vertical target
            if (!hit.contains(x, y) || t.track.w <= 0) continue;
            backend_->setStreamVolume(t.index, (x - t.track.x) / t.track.w);
            return true;
        }
        return false;
    }

    VolumeBackend* backend_ = nullptr;
    bool showStreams_ = false;
    std::vector<SinkRow> sinkRows_;
    std::vector<StreamTrack> streamTracks_;
    double hoverX_ = -1, hoverY_ = -1;
};
}  // namespace

VolumeIndicator::VolumeIndicator(const SystemBackends& backends)
    : StatusIndicator("volume", Zone::Right, 200),
      backend_(backends.volume),
      sessionSurface_(backends.sessionSurface) {
    // Hidden until the backend pushes real data; render defaults without one.
    if (backend_) visible = false;
}

std::string VolumeIndicator::icon() const {
    return volumeIcon(lastSnap_);
}

std::string VolumeIndicator::tooltip() const {
    if (lastSnap_.muted) return "Muted — " + lastSnap_.sinkName;
    return "Volume " + std::to_string(static_cast<int>(lastSnap_.level * 100 + 0.5)) + "% — " +
           lastSnap_.sinkName;
}

void VolumeIndicator::onBackendUpdate() {
    if (!backend_) return;
    lastSnap_ = backend_->snapshot();
    visible = lastSnap_.available;
}

bool VolumeIndicator::onScroll(double dx, double dy, double x, double y) {
    (void)x; (void)y;
    if (!backend_ || !lastSnap_.available) return false;
    // Scroll up (negative dy in Wayland) raises the volume.
    const double delta = dy < 0 ? kScrollStep : -kScrollStep;
    backend_->setLevel(lastSnap_.level + delta);
    return true;
}

std::unique_ptr<QSTile> VolumeIndicator::createTile() {
    auto snap = &lastSnap_;
    auto backend = backend_;
    return std::make_unique<QSSliderTile>(
        "󰕾",
        [snap]() { return snap->level; },
        [backend](double v) {
            if (backend) backend->setLevel(v);
        },
        // Icon follows the live mute/level state; clicking it toggles mute; the
        // row greys while muted.
        [snap]() { return volumeIcon(*snap); },
        [backend]() {
            if (backend) backend->toggleMute();
        },
        [snap]() { return snap->muted; });
}

std::unique_ptr<DetailedPopover> VolumeIndicator::createDetailedView() {
    if (!backend_) return nullptr;
    return std::make_unique<AudioPopover>(backend_, sessionSurface_);
}

REGISTER_INDICATOR("volume", Zone::Right, 200, VolumeIndicator)

}  // namespace qypr
