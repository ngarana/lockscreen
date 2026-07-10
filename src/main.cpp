// main.cpp - qypr-lock entry point.
//
// A lean C++ Wayland session-lock screen (ext-session-lock-v1). Running the
// binary locks the session; successful PAM authentication unlocks and exits.

#include <cstring>
#include <string>

#include "core/App.hpp"

int main(int argc, char** argv) {
    qypr::App app;

    // Offline preview: render frames to PNG without locking the session.
    //   qypr-lock --preview [path.png]
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--preview") == 0) {
            std::string path = (i + 1 < argc) ? argv[i + 1] : "qypr-preview.png";
            return app.preview(path);
        }
    }

    return app.run();
}
