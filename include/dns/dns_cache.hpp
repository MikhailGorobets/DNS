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