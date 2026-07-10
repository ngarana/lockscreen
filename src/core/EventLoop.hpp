// EventLoop.hpp - Single-threaded epoll reactor.
//
// One responsibility: multiplex file descriptors, timers, and cross-thread
// task posts onto one thread. Everything else (Wayland, PAM results, D-Bus)
// plugs in through addFd / addTimer / post — the loop knows nothing about them.

#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace qypr {

class EventLoop {
public:
    using Callback = std::function<void()>;
    using FdCallback = std::function<void(uint32_t events)>;

    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    // Watch fd for readability; cb runs on the loop thread when it fires.
    void addFd(int fd, FdCallback cb);
    void removeFd(int fd);

    // Create a timerfd. Returns its fd (an opaque handle for removeTimer).
    int addTimer(int64_t intervalMs, bool repeat, Callback cb);
    void removeTimer(int timerFd);

    // Run cb before every epoll_wait (e.g. flush the Wayland connection).
    void addPrepare(Callback cb) { prepares_.push_back(std::move(cb)); }

    // Thread-safe: enqueue cb to run on the loop thread and wake the loop.
    void post(Callback cb);

    void run();
    void quit();

private:
    void dispatchPosted();

    int epollFd_ = -1;
    int wakeFd_ = -1;  // eventfd for post()/quit() wakeups
    bool running_ = false;

    struct Timer { Callback cb; bool repeat; };
    std::unordered_map<int, FdCallback> fds_;
    std::unordered_map<int, Timer> timers_;
    std::vector<Callback> prepares_;

    std::mutex postMutex_;
    std::vector<Callback> posted_;
};

}  // namespace qypr
