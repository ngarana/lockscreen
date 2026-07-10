#include "mpris/MprisController.hpp"

#include <sdbus-c++/sdbus-c++.h>

#include <algorithm>
#include <map>
#include <optional>

namespace qypr {

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

MprisController::MprisController() {
    try {
        conn_ = sdbus::createSessionBusConnection();
        dbusProxy_ = sdbus::createProxy(*conn_, sdbus::ServiceName{"org.freedesktop.DBus"},
                                        sdbus::ObjectPath{"/org/freedesktop/DBus"});
    } catch (...) {
        conn_.reset();  // no session bus: controller stays inert
    }
}

MprisController::~MprisController() = default;

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

void MprisController::refresh() {
    if (!conn_) {
        snap_ = Snapshot{};
        return;
    }
    std::string name = pickActive(listPlayers());
    snap_ = name.empty() ? Snapshot{} : readSnapshot(name);
}

void MprisController::togglePlaying() {
    if (!snap_.valid || !snap_.canControl) return;
    try {
        playerProxy(snap_.dbusName)->callMethod("PlayPause").onInterface(kPlayerIface).dontExpectReply();
    } catch (...) {
    }
    refresh();
}

void MprisController::next() {
    if (!snap_.valid || !snap_.canGoNext) return;
    try {
        playerProxy(snap_.dbusName)->callMethod("Next").onInterface(kPlayerIface).dontExpectReply();
    } catch (...) {
    }
    refresh();
}

void MprisController::previous() {
    if (!snap_.valid || !snap_.canGoPrevious) return;
    try {
        playerProxy(snap_.dbusName)
            ->callMethod("Previous")
            .onInterface(kPlayerIface)
            .dontExpectReply();
    } catch (...) {
    }
    refresh();
}

void MprisController::setVolume(double level) {
    if (!snap_.valid || !snap_.volumeSupported) return;
    double clamped = std::clamp(level, 0.0, 1.0);
    try {
        playerProxy(snap_.dbusName)->setProperty("Volume").onInterface(kPlayerIface).toValue(clamped);
        snap_.volume = clamped;
    } catch (...) {
    }
}

}  // namespace qypr
