#include "mpris/MprisController.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <algorithm>
#include <map>
#include <optional>

namespace qypr {

#ifndef TESTING
namespace {
constexpr const char* kObjectPath = "/org/mpris/MediaPlayer2";
constexpr const char* kPlayerIface = "org.mpris.MediaPlayer2.Player";
constexpr const char* kAppIface = "org.mpris.MediaPlayer2";
constexpr const char* kPrefix = "org.mpris.MediaPlayer2.";

const std::vector<std::string> kPriority = {"org.mpris.MediaPlayer2.mpv",
                                            "org.mpris.MediaPlayer2.mpd"};
const std::vector<std::string> kPrefixes = {"org.mpris.MediaPlayer2.firefox",
                                            "org.mpris.MediaPlayer2.chromium",
                                            "org.mpris.MediaPlayer2.spotify"};

bool startsWith(const std::string& s, const std::string& p) {
    return s.size() >= p.size() && s.compare(0, p.size(), p) == 0;
}

bool matchesPriority(const std::string& name) {
    if (std::find(kPriority.begin(), kPriority.end(), name) != kPriority.end()) return true;
    for (const auto& p : kPrefixes)
        if (startsWith(name, p)) return true;
    return false;
}

template <typename T>
std::optional<T> getProp(sdbus::IProxy& proxy, const char* iface, const char* name) {
    try {
        sdbus::Variant v = proxy.getProperty(name).onInterface(iface);
        return v.get<T>();
    } catch (...) {
        return std::nullopt;
    }
}
}  // namespace
#endif

MprisController::MprisController() {
#ifdef TESTING
    // Mock connection setup: does nothing, just lets available() return true
#else
    try {
        conn_ = sdbus::createSessionBusConnection();
        dbusProxy_ = sdbus::createProxy(*conn_, sdbus::ServiceName{"org.freedesktop.DBus"},
                                        sdbus::ObjectPath{"/org/freedesktop/DBus"});
    } catch (...) {
        conn_.reset();  // no session bus: controller stays inert
    }
#endif
}

MprisController::~MprisController() = default;

#ifndef TESTING
std::unique_ptr<sdbus::IProxy> MprisController::playerProxy(const std::string& name) {
    return sdbus::createProxy(*conn_, sdbus::ServiceName{name}, sdbus::ObjectPath{kObjectPath});
}

std::vector<std::string> MprisController::listPlayers() {
    std::vector<std::string> players;
    if (!dbusProxy_) return players;
    try {
        std::vector<std::string> names;
        dbusProxy_->callMethod("ListNames").onInterface("org.freedesktop.DBus").storeResultsTo(names);
        for (auto& n : names)
            if (startsWith(n, kPrefix)) players.push_back(n);
    } catch (...) {
    }
    return players;
}

std::string MprisController::pickActive(const std::vector<std::string>& players) {
    std::vector<std::string> prioritized;
    std::string firstPlaying;
    for (const auto& name : players) {
        if (matchesPriority(name)) prioritized.push_back(name);
        auto proxy = playerProxy(name);
        auto status = getProp<std::string>(*proxy, kPlayerIface, "PlaybackStatus");
        if (status && *status == "Playing" && firstPlaying.empty()) firstPlaying = name;
    }
    for (const auto& name : prioritized) {
        auto proxy = playerProxy(name);
        auto status = getProp<std::string>(*proxy, kPlayerIface, "PlaybackStatus");
        if (status && *status == "Playing") return name;
    }
    if (!prioritized.empty()) return prioritized.front();
    if (!firstPlaying.empty()) return firstPlaying;
    if (!players.empty()) return players.front();
    return "";
}

MprisController::Snapshot MprisController::readSnapshot(const std::string& name) {
    Snapshot s;
    auto proxy = playerProxy(name);
    s.dbusName = name;

    if (auto v = getProp<std::string>(*proxy, kPlayerIface, "PlaybackStatus")) s.status = *v;
    if (auto v = getProp<std::string>(*proxy, kAppIface, "Identity")) s.identity = *v;
    if (auto v = getProp<bool>(*proxy, kPlayerIface, "CanControl")) s.canControl = *v;
    if (auto v = getProp<bool>(*proxy, kPlayerIface, "CanGoNext")) s.canGoNext = *v;
    if (auto v = getProp<bool>(*proxy, kPlayerIface, "CanGoPrevious")) s.canGoPrevious = *v;
    if (auto v = getProp<int64_t>(*proxy, kPlayerIface, "Position")) s.positionUs = *v;
    if (auto v = getProp<double>(*proxy, kPlayerIface, "Volume")) {
        s.volume = *v;
        s.volumeSupported = true;
    }

    // Metadata (a{sv}): title, artist(s), album, length.
    if (auto md = getProp<std::map<std::string, sdbus::Variant>>(*proxy, kPlayerIface, "Metadata")) {
        auto& m = *md;
        auto strOf = [&](const char* key) -> std::string {
            auto it = m.find(key);
            if (it == m.end()) return "";
            try { return it->second.get<std::string>(); } catch (...) {}
            try {
                auto arr = it->second.get<std::vector<std::string>>();
                std::string out;
                for (size_t i = 0; i < arr.size(); ++i) out += (i ? ", " : "") + arr[i];
                return out;
            } catch (...) {}
            return "";
        };
        s.title = strOf("xesam:title");
        s.artist = strOf("xesam:artist");
        s.album = strOf("xesam:album");
        auto it = m.find("mpris:length");
        if (it != m.end()) {
            try { s.lengthUs = it->second.get<int64_t>(); } catch (...) {}
        }
    }

    s.valid = true;
    return s;
}
#endif

void MprisController::refresh() {
#ifdef TESTING
    // Populate fake snapshot data
    snap_.valid = true;
    snap_.dbusName = "org.mpris.MediaPlayer2.mock";
    snap_.identity = "Mock Player";
    snap_.title = "Mock Song";
    snap_.artist = "Mock Artist";
    snap_.album = "Mock Album";
    if (snap_.status.empty()) snap_.status = "Playing";
    snap_.volume = 0.8;
    snap_.positionUs = 60 * 1000000;
    snap_.lengthUs = 180 * 1000000;
    snap_.canControl = true;
    snap_.canGoNext = true;
    snap_.canGoPrevious = true;
    snap_.volumeSupported = true;
#else
    if (!conn_) {
        snap_ = Snapshot{};
        return;
    }
    std::string name = pickActive(listPlayers());
    snap_ = name.empty() ? Snapshot{} : readSnapshot(name);
#endif
}

void MprisController::togglePlaying() {
#ifdef TESTING
    if (!snap_.valid || !snap_.canControl) return;
    if (snap_.status == "Playing") {
        snap_.status = "Paused";
    } else {
        snap_.status = "Playing";
    }
#else
    if (!snap_.valid || !snap_.canControl) return;
    try {
        playerProxy(snap_.dbusName)->callMethod("PlayPause").onInterface(kPlayerIface).dontExpectReply();
    } catch (...) {
    }
    refresh();
#endif
}

void MprisController::next() {
#ifdef TESTING
    if (!snap_.valid || !snap_.canGoNext) return;
    snap_.title = "Next Song";
    snap_.positionUs = 0;
#else
    if (!snap_.valid || !snap_.canGoNext) return;
    try {
        playerProxy(snap_.dbusName)->callMethod("Next").onInterface(kPlayerIface).dontExpectReply();
    } catch (...) {
    }
    refresh();
#endif
}

void MprisController::previous() {
#ifdef TESTING
    if (!snap_.valid || !snap_.canGoPrevious) return;
    snap_.title = "Previous Song";
    snap_.positionUs = 0;
#else
    if (!snap_.valid || !snap_.canGoPrevious) return;
    try {
        playerProxy(snap_.dbusName)
            ->callMethod("Previous")
            .onInterface(kPlayerIface)
            .dontExpectReply();
    } catch (...) {
    }
    refresh();
#endif
}

void MprisController::setVolume(double level) {
#ifdef TESTING
    if (!snap_.valid || !snap_.volumeSupported) return;
    snap_.volume = std::clamp(level, 0.0, 1.0);
#else
    if (!snap_.valid || !snap_.volumeSupported) return;
    double clamped = std::clamp(level, 0.0, 1.0);
    try {
        playerProxy(snap_.dbusName)->setProperty("Volume").onInterface(kPlayerIface).toValue(clamped);
        snap_.volume = clamped;
    } catch (...) {
    }
#endif
}

}  // namespace qypr
