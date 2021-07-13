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
