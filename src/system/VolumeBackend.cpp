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
        snap_.available = false;
        if (onChange_) onChange_();  // hide the placeholder
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
                                                    PA_SUBSCRIPTION_MASK_SINK_INPUT |
                                                    PA_SUBSCRIPTION_MASK_SERVER),
                nullptr, nullptr));
            self->queryServer();
            self->querySinks();
            self->queryStreams();
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
        self->querySinks();    // …which re-marks isDefault
    } else if (facility == PA_SUBSCRIPTION_EVENT_SINK) {
        self->querySink();     // default volume/mute changed
        self->querySinks();    // a device came/went
    } else if (facility == PA_SUBSCRIPTION_EVENT_SINK_INPUT) {
        self->queryStreams();  // an app stream came/went/changed
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

void VolumeBackend::querySinks() {
    sinksBuilding_.clear();
    fire(pa_context_get_sink_info_list(ctx_, &VolumeBackend::onSinkList, this));
}

void VolumeBackend::onSinkList(pa_context*, const pa_sink_info* info, int eol, void* userdata) {
    auto* self = static_cast<VolumeBackend*>(userdata);
    if (eol) {
        // Enumeration complete: publish if it actually changed.
        if (self->sinksBuilding_ != self->sinks_) {
            self->sinks_ = self->sinksBuilding_;
            self->notify();
        }
        self->sinksBuilding_.clear();
        return;
    }
    if (!info) return;
    AudioSink s;
    s.name = info->name ? info->name : "";
    s.description = info->description ? info->description : s.name;
    s.isDefault = s.name == self->defaultSink_;
    self->sinksBuilding_.push_back(std::move(s));
}

void VolumeBackend::queryStreams() {
    streamsBuilding_.clear();
    fire(pa_context_get_sink_input_info_list(ctx_, &VolumeBackend::onStreamList, this));
}

void VolumeBackend::onStreamList(pa_context*, const pa_sink_input_info* info, int eol,
                                 void* userdata) {
    auto* self = static_cast<VolumeBackend*>(userdata);
    if (eol) {
        if (self->streamsBuilding_ != self->streams_) {
            self->streams_ = self->streamsBuilding_;
            self->notify();
        }
        self->streamsBuilding_.clear();
        return;
    }
    if (!info) return;
    // Skip streams with no client (e.g. internal monitors) and PulseAudio's own
    // helpers; a stream with no app name is not useful in the app list.
    const char* app = pa_proplist_gets(info->proplist, PA_PROP_APPLICATION_NAME);
    AudioStream s;
    s.index = info->index;
    s.appName = app ? app : (info->name ? info->name : "Audio");
    s.level = static_cast<double>(pa_cvolume_avg(&info->volume)) / PA_VOLUME_NORM;
    s.muted = info->mute != 0;
    s.channels = info->volume.channels;
    self->streamsBuilding_.push_back(std::move(s));
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

void VolumeBackend::setDefaultSink(const std::string& name) {
    if (!ctx_ || name.empty()) return;
    fire(pa_context_set_default_sink(ctx_, name.c_str(), nullptr, nullptr));
    // Optimistic: re-mark the list; the SERVER event confirms and re-queries.
    defaultSink_ = name;
    for (auto& s : sinks_) s.isDefault = (s.name == name);
    notify();
}

void VolumeBackend::setStreamVolume(uint32_t index, double frac) {
    if (!ctx_) return;
    frac = std::clamp(frac, 0.0, 1.0);
    for (auto& s : streams_) {
        if (s.index != index) continue;
        pa_cvolume cv;
        pa_cvolume_set(&cv, s.channels, static_cast<pa_volume_t>(frac * PA_VOLUME_NORM + 0.5));
        fire(pa_context_set_sink_input_volume(ctx_, index, &cv, nullptr, nullptr));
        s.level = frac;  // optimistic; the SINK_INPUT event confirms
        notify();
        return;
    }
}

void VolumeBackend::toggleStreamMute(uint32_t index) {
    if (!ctx_) return;
    for (auto& s : streams_) {
        if (s.index != index) continue;
        const bool mute = !s.muted;
        fire(pa_context_set_sink_input_mute(ctx_, index, mute ? 1 : 0, nullptr, nullptr));
        s.muted = mute;
        notify();
        return;
    }
}

}  // namespace qypr
