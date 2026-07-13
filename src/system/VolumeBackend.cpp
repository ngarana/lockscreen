// VolumeBackend.cpp - libpulse default-sink monitor implementation.
#include "system/VolumeBackend.hpp"

#include <algorithm>
#include <cstdio>

#include "core/EventLoop.hpp"

namespace qypr {

namespace {
// Run a pa_operation-returning call and release the handle.
template <typename Op>
void fire(Op* op) {
    if (op) pa_operation_unref(op);
}
}  // namespace

VolumeBackend::VolumeBackend(EventLoop& loop) : pulseLoop_(loop) {}

VolumeBackend::~VolumeBackend() {
    if (ctx_) {
        // Silence callbacks first: disconnect() fires the TERMINATED state
        // callback synchronously, and consumers (StatusBar) may already be
        // destroyed by now.
        onChange_ = nullptr;
        pa_context_set_state_callback(ctx_, nullptr, nullptr);
        pa_context_set_subscribe_callback(ctx_, nullptr, nullptr);
        // Disconnect synchronously frees every io/time/defer event that
        // libpulse created through PulseLoop.
        pa_context_disconnect(ctx_);
        pa_context_unref(ctx_);
    }
}

bool VolumeBackend::start() {
    ctx_ = pa_context_new(pulseLoop_.api(), "qypr");
    if (!ctx_) return false;

    pa_context_set_state_callback(ctx_, &VolumeBackend::onContextState, this);
    pa_context_set_subscribe_callback(ctx_, &VolumeBackend::onSubscribe, this);

    if (pa_context_connect(ctx_, nullptr, PA_CONTEXT_NOAUTOSPAWN, nullptr) < 0) {
        std::fprintf(stderr, "qypr: pulse connect failed (%s); volume indicator disabled\n",
                     pa_strerror(pa_context_errno(ctx_)));
        pa_context_unref(ctx_);
        ctx_ = nullptr;
        return false;
    }
    return true;
}

void VolumeBackend::onContextState(pa_context* c, void* userdata) {
    auto* self = static_cast<VolumeBackend*>(userdata);
    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_READY:
            fire(pa_context_subscribe(
                c,
                static_cast<pa_subscription_mask_t>(PA_SUBSCRIPTION_MASK_SINK |
                                                    PA_SUBSCRIPTION_MASK_SERVER),
                nullptr, nullptr));
            self->queryServer();
            break;
        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED: {
            VolumeSnapshot gone;
            self->changed(gone);
            break;
        }
        default:
            break;
    }
}

void VolumeBackend::onSubscribe(pa_context*, pa_subscription_event_type_t t, uint32_t,
                                void* userdata) {
    auto* self = static_cast<VolumeBackend*>(userdata);
    const auto facility = t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;
    if (facility == PA_SUBSCRIPTION_EVENT_SERVER) {
        self->queryServer();   // default sink may have changed
    } else if (facility == PA_SUBSCRIPTION_EVENT_SINK) {
        self->querySink();     // volume/mute changed
    }
}

void VolumeBackend::queryServer() {
    fire(pa_context_get_server_info(ctx_, &VolumeBackend::onServerInfo, this));
}

void VolumeBackend::onServerInfo(pa_context*, const pa_server_info* info, void* userdata) {
    auto* self = static_cast<VolumeBackend*>(userdata);
    if (!info || !info->default_sink_name) return;
    self->defaultSink_ = info->default_sink_name;
    self->querySink();
}

void VolumeBackend::querySink() {
    if (defaultSink_.empty()) return;
    fire(pa_context_get_sink_info_by_name(ctx_, defaultSink_.c_str(),
                                          &VolumeBackend::onSinkInfo, this));
}

void VolumeBackend::onSinkInfo(pa_context*, const pa_sink_info* info, int eol, void* userdata) {
    if (eol || !info) return;
    auto* self = static_cast<VolumeBackend*>(userdata);
    self->channels_ = info->volume.channels;

    VolumeSnapshot next;
    next.available = true;
    next.level = static_cast<double>(pa_cvolume_avg(&info->volume)) / PA_VOLUME_NORM;
    next.muted = info->mute != 0;
    next.sinkName = info->description ? info->description : "";
    self->changed(next);
}

void VolumeBackend::changed(const VolumeSnapshot& next) {
    if (next == snap_) return;
    snap_ = next;
    if (onChange_) onChange_();
}

void VolumeBackend::setLevel(double frac) {
    if (!ctx_ || defaultSink_.empty()) return;
    frac = std::clamp(frac, 0.0, 1.0);

    pa_cvolume cv;
    pa_cvolume_set(&cv, channels_, static_cast<pa_volume_t>(frac * PA_VOLUME_NORM + 0.5));
    fire(pa_context_set_sink_volume_by_name(ctx_, defaultSink_.c_str(), &cv, nullptr, nullptr));

    // Optimistic; the SINK subscription event confirms.
    VolumeSnapshot next = snap_;
    next.level = frac;
    changed(next);
}

void VolumeBackend::toggleMute() {
    if (!ctx_ || defaultSink_.empty()) return;
    const bool mute = !snap_.muted;
    fire(pa_context_set_sink_mute_by_name(ctx_, defaultSink_.c_str(), mute ? 1 : 0, nullptr,
                                          nullptr));
    VolumeSnapshot next = snap_;
    next.muted = mute;
    changed(next);
}

}  // namespace qypr
