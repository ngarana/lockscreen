// ClockIndicator.cpp - Status bar clock indicator implementation.
#include "ui/indicators/ClockIndicator.hpp"

#include <ctime>

#include "ui/Theme.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"

namespace qypr {

ClockIndicator::ClockIndicator(const SystemBackends& backends)
    : StatusIndicator("clock", Zone::Left, 0) {
    (void)backends;
}

std::string ClockIndicator::timeString() const {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    localtime_r(&t, &tm);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%a %b %-d   %-I:%M %p", &tm);
    return buf;
}

std::string ClockIndicator::dateString() const {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    localtime_r(&t, &tm);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%A, %B %-d", &tm);
    return buf;
}

std::string ClockIndicator::label() const {
    return cachedTime_.empty() ? timeString() : cachedTime_;
}

std::string ClockIndicator::tooltip() const {
    return dateString();
}

double ClockIndicator::labelFontSize() const {
    return theme::statusbar::iconSize;
}

void ClockIndicator::poll(int64_t now) {
    if (now - lastPoll_ >= 1000) {
        cachedTime_ = timeString();
        lastPoll_ = now;
    }
}

REGISTER_INDICATOR("clock", Zone::Left, 0, ClockIndicator)

}  // namespace qypr
