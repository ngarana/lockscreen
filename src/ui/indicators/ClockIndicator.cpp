// ClockIndicator.cpp - Status bar clock indicator implementation.
#include "ui/indicators/ClockIndicator.hpp"

#include <ctime>

#include "core/Config.hpp"
#include "ui/Theme.hpp"
#include "ui/statusbar/IndicatorRegistry.hpp"

namespace qypr {

namespace {
// strftime patterns. Overridable via `[clock] format` / `tooltip-format`;
// qypr-lock passes no config and keeps these.
constexpr const char* kDefaultFormat = "%a %b %-d   %-I:%M %p";
constexpr const char* kDefaultTooltipFormat = "%A, %B %-d";

std::string formatNow(const std::string& fmt) {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    localtime_r(&t, &tm);
    char buf[256];
    // strftime returns 0 both for "empty result" and "didn't fit"; either way an
    // empty string is the honest answer for a format we cannot render.
    const size_t n = std::strftime(buf, sizeof(buf), fmt.c_str(), &tm);
    return n == 0 ? std::string() : std::string(buf, n);
}
}  // namespace

ClockIndicator::ClockIndicator(const SystemBackends& backends)
    : StatusIndicator("clock", Zone::Left, 0) {
    if (backends.config) {
        format_ = backends.config->getString("clock", "format", kDefaultFormat);
        tooltipFormat_ =
            backends.config->getString("clock", "tooltip-format", kDefaultTooltipFormat);
    }
}

std::string ClockIndicator::timeString() const { return formatNow(format_); }

std::string ClockIndicator::dateString() const { return formatNow(tooltipFormat_); }

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
