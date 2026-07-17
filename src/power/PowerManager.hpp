// PowerManager.hpp - System power actions via systemctl (port of PowerManager.qml).
//
// Fire-and-forget: each action spawns `systemctl <verb>`. Children are
// auto-reaped (SIGCHLD ignored) so no zombies accumulate.
//
// Spawning here is deliberate and within the footprint principle (STATUS_BAR.md
// decision D1): these are user-initiated, one-shot actions with a bounded
// lifetime — never a way to *read* state, and never on a timer.

#pragma once

namespace qypr {

class PowerManager {
public:
    PowerManager();

    void suspend() { run("suspend"); }
    void reboot() { run("reboot"); }
    void shutdown() { run("poweroff"); }
    void hibernate() { run("hibernate"); }

    // Lock the session via logind, which asks whatever locker the session has
    // configured to run (the standard, daemon-agnostic path — qypr never
    // launches itself directly).
    void lock() { runCmd("loginctl", "lock-session"); }

private:
    void run(const char* verb);  // systemctl <verb>
    void runCmd(const char* prog, const char* arg);
};

}  // namespace qypr
