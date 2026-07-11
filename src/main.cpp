// main.cpp - qypr-lock entry point.
//
// A lean C++ Wayland session-lock screen (ext-session-lock-v1). Running the
// binary locks the session; successful PAM authentication unlocks and exits.

#include <cstdlib>
#include <cstring>
#include <string>

#include "core/App.hpp"
#include "core/EventLoop.hpp"
#include "notifications/NotificationLog.hpp"
#include "notifications/NotificationMonitor.hpp"

namespace qypr {

// Session-long recorder: mirror the notification daemon's queue (captures
// minus dismissals) and serve it as org.qypr.Notifications, so a lock screen
// starting later can seed itself with the pre-lock backlog. Meant to run for
// the whole session (systemd --user service).
int runRecorder() {
    EventLoop loop;
    NotificationMonitor monitor(loop);
    NotificationLog log(loop, monitor);
    if (!monitor.start() || !log.start()) return 1;
    loop.run();
    return 0;
}

}  // namespace qypr

int main(int argc, char** argv) {
    // Recorder mode never needs the App (Wayland, PAM, video, ...): keep the
    // session service tiny.
    //   qypr-lock --record
    for (int i = 1; i < argc; ++i)
        if (std::strcmp(argv[i], "--record") == 0) return qypr::runRecorder();

    qypr::App app;

    // Offline preview: render frames to PNG without locking the session.
    //   qypr-lock --preview [path.png]
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--preview") == 0) {
            std::string path = (i + 1 < argc) ? argv[i + 1] : "qypr-preview.png";
            return app.preview(path);
        }
        // Offscreen video-pipeline test: exercise mpv without locking.
        //   qypr-lock --video-test [seconds]
        if (std::strcmp(argv[i], "--video-test") == 0) {
            int secs = (i + 1 < argc) ? std::atoi(argv[i + 1]) : 6;
            return app.videoTest(secs > 0 ? secs : 6);
        }
        // Idle seconds before the video pauses and the screen dims.
        //   qypr-lock --idle-timeout <seconds>
        if (std::strcmp(argv[i], "--idle-timeout") == 0 && i + 1 < argc) {
            app.setIdleTimeout(std::atoi(argv[++i]));
        }
    }

    return app.run();
}
