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

#include <boost/asio.hpp>
#include <dns/dns.hpp>
#include <dns/dns_cache.hpp>

namespace NET {
    using SocketUDP = boost::asio::ip::udp::socket;
    using SocketTCP = boost::asio::ip::tcp::socket;

    using UDP = boost::asio::ip::udp;
    using UDPoint = boost::asio::ip::udp::endpoint;

    using TCP = boost::asio::ip::tcp;
    using TCPPoint = boost::asio::ip::tcp::endpoint;

    using IOContext = boost::asio::io_context;
    using SignalSet = boost::asio::signal_set;

    using ShutdownType = boost::asio::socket_base::shutdown_type;
    using Exeception = boost::system::system_error;

    using Error = boost::system::error_code;
    using ErrorType = boost::asio::error::basic_errors;

    template<typename... Args>
    auto Buffer(Args&&... args) -> decltype(boost::asio::buffer(std::forward<Args>(args)...)) {
        return boost::asio::buffer(std::forward<Args>(args)...);
    }

    template<typename... Args>
    auto Address(Args&&... args) -> decltype(boost::asio::ip::make_address_v4(std::forward<Args>(args)...)) {
        return boost::asio::ip::make_address_v4(std::forward<Args>(args)...);
    }

    template<typename... Args>
    auto Post(Args&&... args) -> decltype(boost::asio::post(std::forward<Args>(args)...)) {
        return boost::asio::post(std::forward<Args>(args)...);
    }
}

class DNSServer {
public:
    DNSServer(int argc, char* argv[]);

    auto Run() -> void;

private:
    using PtrDNSCache = std::unique_ptr<DNSCache>;
    using PtrSocketUDP = std::unique_ptr<NET::SocketUDP>;
    using PtrSignalSet = std::unique_ptr<NET::SignalSet>;
    using ThreadPool = boost::asio::thread_pool;

    std::atomic_bool m_IsApplicationRun = {};
    ThreadPool       m_Dispather = {};
    PtrDNSCache      m_Cache = {};
    NET::IOContext   m_Service = {};
    PtrSignalSet     m_SignalSet = {};
    PtrSocketUDP     m_Socket = {};
};
