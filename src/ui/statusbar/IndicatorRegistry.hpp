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

    // Register an indicator factory.
    void registerIndicator(const std::string& id, Zone zone, int priority, Factory factory);

    // Construct all registered indicators.
    std::vector<std::unique_ptr<StatusIndicator>> createAll(const SystemBackends& backends) const;

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
