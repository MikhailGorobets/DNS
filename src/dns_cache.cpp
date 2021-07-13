#include <dns/dns_cache.hpp>

auto DNSCache::Add(std::string const& name, DNS::Package const package) -> void {
    std::unique_lock lock(m_MutexCache);
    m_PackageCache.emplace(name, package);
}

auto DNSCache::Get(std::string const& name) const -> std::optional<DNS::Package> {
    std::shared_lock lock(m_MutexCache);
    if (auto iter = m_PackageCache.find(name); iter != m_PackageCache.end())
        return iter->second;
    return std::nullopt;
}

auto DNSCache::RemoveTimeoutPackages(uint32_t seconds) -> void {
    std::unique_lock lock(m_MutexCache);
    auto Predicate = [seconds](DNS::ResourceRecord& record) -> bool {
        if (!std::in_range<uint32_t>(DNS::SwapEndian(record.Answer.TTL) - seconds))
            return true;
        record.Answer.TTL = DNS::SwapEndian((DNS::SwapEndian(record.Answer.TTL) - seconds));
        return false;
    };

    for (auto& [name, package] : m_PackageCache) {
        std::erase_if(package.Answers, Predicate);
        std::erase_if(package.Authoritys, Predicate);
    }

    std::erase_if(m_PackageCache, [](auto const& item) -> bool {
        auto const& [name, package] = item;
        if (package.Answers.empty() && package.Authoritys.empty())
            return true;
        return false;
    });
}
