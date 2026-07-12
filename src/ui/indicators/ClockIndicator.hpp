// ClockIndicator.hpp - Status bar clock text indicator.
#pragma once

#include "ui/statusbar/StatusIndicator.hpp"
#include <string>

namespace qypr {

class ClockIndicator : public StatusIndicator {
public:
    ClockIndicator(const SystemBackends& backends);

    std::string icon() const override;
    std::string label() const override;
    std::string tooltip() const override;
    double measureWidth(Painter& p) override;

    void poll(int64_t now) override;

private:
    std::string timeString() const;
    std::string dateString() const;

    mutable std::string cachedTime_;
    mutable std::string cachedDate_;
    mutable int64_t lastPoll_ = 0;
};

}  // namespace qypr
