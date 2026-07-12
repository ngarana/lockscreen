// IndicatorRegistry.cpp - Plugin registration system implementation
#include "ui/statusbar/IndicatorRegistry.hpp"
#include <algorithm>

namespace qypr {

IndicatorRegistry& IndicatorRegistry::instance() {
    static IndicatorRegistry inst;
    return inst;
}

void IndicatorRegistry::registerIndicator(const std::string& id, Zone zone, int priority, Factory factory) {
    entries_.push_back({id, zone, priority, std::move(factory)});
}

std::vector<std::unique_ptr<StatusIndicator>> IndicatorRegistry::createAll(const SystemBackends& backends) const {
    auto sortedEntries = entries_;
    std::sort(sortedEntries.begin(), sortedEntries.end(), [](const Entry& a, const Entry& b) {
        if (a.zone != b.zone) {
            return a.zone < b.zone; // Group by zone
        }
        return a.priority < b.priority; // Sort by priority ascending
    });

    std::vector<std::unique_ptr<StatusIndicator>> result;
    for (const auto& entry : sortedEntries) {
        if (auto ind = entry.factory(backends)) {
            result.push_back(std::move(ind));
        }
    }
    return result;
}

}  // namespace qypr
