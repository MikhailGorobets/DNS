#pragma once

#include <dns/dns.hpp>
#include <condition_variable>
#include <shared_mutex>

class DNSCache {
public:
    auto Add(std::string const& name, DNS::Package const package) -> void;

    auto Get(std::string const& name) const->std::optional<DNS::Package>;

    auto RemoveTimeoutPackages(uint32_t seconds) -> void;

    template<class Predicate>
    auto RemoveTimeoutPackagesWaitFor(uint32_t seconds, Predicate predicate) -> void {
        RemoveTimeoutPackages(seconds);
        std::unique_lock lock(m_MutexWakeUp);
        m_WakeUp.wait_for(lock, std::chrono::seconds(seconds), predicate);
    }

private:
    using MapPackage = std::unordered_map<std::string, DNS::Package>;

    mutable std::shared_mutex m_MutexCache = {};
    MapPackage                m_PackageCache = {};
    std::condition_variable   m_WakeUp;
    std::mutex                m_MutexWakeUp;
};