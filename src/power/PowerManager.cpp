#include "power/PowerManager.hpp"

#include <signal.h>
#include <unistd.h>

namespace qypr {

PowerManager::PowerManager() {
    // Auto-reap spawned systemctl processes instead of leaving zombies.
    signal(SIGCHLD, SIG_IGN);
}

void PowerManager::run(const char* verb) { runCmd("systemctl", verb); }

void PowerManager::runCmd(const char* prog, const char* arg) {
#ifdef TESTING
    (void)prog;
    (void)arg;
#else
    pid_t pid = fork();
    if (pid == 0) {
        execlp(prog, prog, arg, static_cast<char*>(nullptr));
        _exit(127);  // exec failed
    }
    // Parent: do not block; the action either takes over or fails silently.
#endif
}

}  // namespace qypr
