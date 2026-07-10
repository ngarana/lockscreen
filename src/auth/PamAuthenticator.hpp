// PamAuthenticator.hpp - Asynchronous PAM password authentication.
//
// pam_authenticate() blocks, so it runs on a worker thread; the result is
// posted back onto the event loop so all UI/state changes stay single-threaded.
// Mirrors LockController.qml's PAM flow (service "login").

#pragma once

#include <atomic>
#include <functional>
#include <string>
#include <thread>

namespace qypr {

class EventLoop;

class PamAuthenticator {
public:
    enum class Result { Success, Failure, Error };
    using Done = std::function<void(Result, std::string message)>;

    PamAuthenticator(EventLoop& loop, std::string service = "login");
    ~PamAuthenticator();

    bool busy() const { return busy_.load(); }

    // Begin authentication. `done` runs on the loop thread exactly once.
    // Returns false if a previous attempt is still running or password empty.
    bool authenticate(const std::string& password, Done done);

private:
    EventLoop& loop_;
    std::string service_;
    std::atomic<bool> busy_{false};
    std::thread worker_;
};

}  // namespace qypr
