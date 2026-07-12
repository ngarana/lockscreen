// ClockIndicator.hpp - Status bar clock text indicator.
//
// Text-only (icon() is empty); formats its own time — shares no LockScreen
// widget code and ticks on StatusBar's timer, never LockScreen's.

#pragma once

#include "ui/statusbar/StatusIndicator.hpp"
#include <string>

namespace qypr {

class ClockIndicator : public StatusIndicator {
public:
    explicit ClockIndicator(const SystemBackends& backends);

    std::string icon() const override { return ""; }
    std::string label() const override;
    std::string tooltip() const override;
    double labelFontSize() const override;

    void poll(int64_t now) override;

private:
    std::string timeString() const;
    std::string dateString() const;

    std::string cachedTime_;
    int64_t lastPoll_ = 0;
};

}  // namespace qypr
