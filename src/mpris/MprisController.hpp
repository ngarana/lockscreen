// MprisController.hpp - MPRIS media state + control over the session bus.
//
// Port of AudioService.qml. Uses sdbus-c++ with simple synchronous polling
// (refreshed once a second from the event loop) rather than signal plumbing —
// far less machinery for a screen that only samples state when visible (KISS).

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace sdbus {
class IConnection;
class IProxy;
}  // namespace sdbus

namespace qypr {

class MprisController {
public:
    MprisController();
    ~MprisController();

    // Re-query the active player. Cheap no-op if the session bus is unavailable.
    void refresh();

    // Snapshot accessors (mirror AudioService's read-only properties).
#ifdef TESTING
    bool available() const { return true; }
#else
    bool available() const { return conn_ != nullptr; }
#endif
    bool active() const { return snap_.valid && snap_.status != "Stopped"; }
    bool playing() const { return snap_.status == "Playing"; }
    const std::string& title() const { return snap_.title; }
    const std::string& artist() const { return snap_.artist; }
    const std::string& album() const { return snap_.album; }
    const std::string& sourceLabel() const { return snap_.identity; }
    double positionSeconds() const { return snap_.positionUs / 1'000'000.0; }
    double durationSeconds() const { return snap_.lengthUs / 1'000'000.0; }
    double volume() const { return snap_.volume; }
    bool canGoNext() const { return snap_.canGoNext; }
    bool canGoPrevious() const { return snap_.canGoPrevious; }
    bool canTogglePlaying() const { return snap_.canControl; }
    bool canSetVolume() const { return snap_.canControl && snap_.volumeSupported; }

    // Transport controls.
    void togglePlaying();
    void next();
    void previous();
    void setVolume(double level);

private:
    struct Snapshot {
        bool valid = false;
        std::string dbusName;
        std::string identity;
        std::string title, artist, album;
        std::string status;  // Playing / Paused / Stopped
        double volume = 1.0;
        int64_t positionUs = 0;
        int64_t lengthUs = 0;
        bool canControl = false;
        bool canGoNext = false;
        bool canGoPrevious = false;
        bool volumeSupported = false;
    };

    std::vector<std::string> listPlayers();
    std::string pickActive(const std::vector<std::string>& players);
    Snapshot readSnapshot(const std::string& name);
    std::unique_ptr<sdbus::IProxy> playerProxy(const std::string& name);

    std::unique_ptr<sdbus::IConnection> conn_;
    std::unique_ptr<sdbus::IProxy> dbusProxy_;
    Snapshot snap_;
};

}  // namespace qypr
