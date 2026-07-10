// PowerManager.hpp - System power actions via systemctl (port of PowerManager.qml).
//
// Fire-and-forget: each action spawns `systemctl <verb>`. Children are
// auto-reaped (SIGCHLD ignored) so no zombies accumulate.

#pragma once

namespace qypr {

class PowerManager {
public:
    PowerManager();

    void suspend() { run("suspend"); }
    void reboot() { run("reboot"); }
    void shutdown() { run("poweroff"); }
    void hibernate() { run("hibernate"); }

private:
    void run(const char* verb);
};

}  // namespace qypr
