// IndicatorRegistry.hpp - Plugin registration system for status bar indicators.
#pragma once

#include "ui/statusbar/StatusIndicator.hpp"
#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace qypr {

class IndicatorRegistry {
public:
    static IndicatorRegistry& instance();

    using Factory = std::function<std::unique_ptr<StatusIndicator>(const SystemBackends&)>;

    // Config-driven module selection: ordered indicator ids per zone, as read
    // from `modules-left/center/right`. Order is visual (left → right); an
    // empty vector means that zone is deliberately empty. A module may be
    // placed in any zone regardless of its compiled-in default.
    struct ModuleSelection {
        std::vector<std::string> left, center, right;
    };

    // Register an indicator factory.
    void registerIndicator(const std::string& id, Zone zone, int priority, Factory factory);

    // Construct indicators. With `sel == nullptr` (no config): every registered
    // indicator, grouped by its compiled zone and ordered by priority. With a
    // selection: only the named ids, in the given order, re-homed to the zone
    // they were listed under. Unknown ids are skipped (a typo drops a module,
    // it never crashes the bar).
    std::vector<std::unique_ptr<StatusIndicator>> createAll(
        const SystemBackends& backends, const ModuleSelection* sel = nullptr) const;

    // Ids of every registered indicator (for diagnostics / typo reporting).
    std::vector<std::string> registeredIds() const;

private:
    IndicatorRegistry() = default;
    ~IndicatorRegistry() = default;
    IndicatorRegistry(const IndicatorRegistry&) = delete;
    IndicatorRegistry& operator=(const IndicatorRegistry&) = delete;

    struct Entry {
        std::string id;
        Zone zone;
        int priority;
        Factory factory;
    };
    std::vector<Entry> entries_;
};

// Macro to self-register an indicator at static-init time.
#define REGISTER_INDICATOR(id, zone, priority, Type)                           \
    static const bool _reg_##Type = [] {                                       \
        IndicatorRegistry::instance().registerIndicator(                       \
            id, zone, priority,                                                \
            [](const SystemBackends& b) { return std::make_unique<Type>(b); }  \
        );                                                                     \
        return true;                                                           \
    }();

}  // namespace qypr
