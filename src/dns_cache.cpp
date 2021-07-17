/*
 * MIT License
 *
 * Copyright(c) 2021 Mikhail Gorobets
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this softwareand associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright noticeand this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


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
