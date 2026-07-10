#include "power/PowerManager.hpp"

#include <signal.h>
#include <unistd.h>

namespace qypr {

PowerManager::PowerManager() {
    // Auto-reap spawned systemctl processes instead of leaving zombies.
    signal(SIGCHLD, SIG_IGN);
}

void PowerManager::run(const char* verb) {
    pid_t pid = fork();
    if (pid == 0) {
        execlp("systemctl", "systemctl", verb, static_cast<char*>(nullptr));
        _exit(127);  // exec failed
    }
    // Parent: do not block; the action either takes over or fails silently.
}

}  // namespace qypr
