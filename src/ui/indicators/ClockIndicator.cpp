// ClockIndicator.cpp - Status bar clock indicator implementation.
#include "ui/indicators/ClockIndicator.hpp"

#include <ctime>
#include "ui/Theme.hpp"
#include "render/Painter.hpp"

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

std::string ClockIndicator::icon() const {
    return cachedTime_.empty() ? timeString() : cachedTime_;
}

std::string ClockIndicator::label() const {
    return "";
}

std::string ClockIndicator::tooltip() const {
    return dateString();
}

double ClockIndicator::measureWidth(Painter& p) {
    TextStyle style{theme::font::family, theme::statusbar::iconSize, PANGO_WEIGHT_NORMAL, theme::color::text};
    Size sz = p.measureText(timeString(), style);
    return sz.w + 16.0;  // 8px padding each side
}

void ClockIndicator::poll(int64_t now) {
    // Update once per second
    if (now - lastPoll_ >= 1000) {
        cachedTime_ = timeString();
        cachedDate_ = dateString();
        lastPoll_ = now;
    }
}

}  // namespace qypr
