#include <dns/server.h>
#include <fmt/printf.h>
#include <fmt/ostream.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/thread_pool.hpp>

#include <unordered_map>
#include <span>
#include <queue>
#include <shared_mutex>
#include <stack>
#include <future>

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
    using Exeception   = boost::system::system_error;

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


class DNSCache {
public:
    auto Add(std::string const& name, DNS::Package const package) -> void {
        std::unique_lock lock(m_MutexCache);
        m_PackageCache.emplace(name, package);
    }

    auto Get(std::string const& name) const -> std::optional<DNS::Package> {
        std::shared_lock lock(m_MutexCache);
        if (auto iter = m_PackageCache.find(name); iter != m_PackageCache.end())
            return iter->second;
        return std::nullopt;
    }

private:
    mutable std::shared_mutex                    m_MutexCache = {};
    std::unordered_map<std::string, DNS::Package> m_PackageCache = {};
};

class DNSServer {
public:
    DNSServer(int argc, char* argv[]) {
        m_Cache = std::make_unique<DNSCache>();
        m_SignalSet = std::make_unique<NET::SignalSet>(m_Service, SIGINT, SIGTERM);
        m_Socket = std::make_unique<NET::SocketUDP>(m_Service, NET::UDPoint(NET::UDP::v4(), 57));
    #ifdef _WIN32
        struct IOControlCommand {
            uint32_t value = {};
            auto name() -> int32_t { return SIO_UDP_CONNRESET; }
            auto data() -> void* { return &value; }
        };
        IOControlCommand connectionReset = {};
        m_Socket->io_control(connectionReset);
    #endif
    }

    auto Run() -> void {
        fmt::print("DNS Server: Run \n");
        fmt::print("DNS Server: IP: {}, Port: {} \n", m_Socket->local_endpoint().address().to_string(), m_Socket->local_endpoint().port());
   
        m_IsApplicationRun = true;
        boost::asio::post(m_Dispather, [this]() {
            m_SignalSet->async_wait([&](auto const& error, int32_t signal) {
                m_IsApplicationRun = false;    
               
             //   m_Socket->shutdown(NET::ShutdownType::shutdown_both);          
                m_Socket->close();        
                fmt::print("DNS Server: Shutdown \n");
            });
            m_Service.run();
        });

        //The send_to and receive_from functions must be thread safe up to a certain size(65536)
        NET::Post(m_Dispather, ([this]() {
            while (m_IsApplicationRun) {
                try {
                    NET::UDPoint point;
                    std::vector<uint8_t> buffer(DNS::PACKAGE_SIZE);
                    m_Socket->receive_from(NET::Buffer(buffer), point);
                    boost::asio::post(m_Dispather, [this, point = std::move(point), buffer = std::move(buffer)]() mutable {
                        try {
                            DNS::Package packet = DNS::CreatePackageFromBuffer(buffer);
                            auto cacheValue = m_Cache->Get(packet.Questions.at(0).Name);

                            if (cacheValue.has_value()) {
                                DNS::Package packetCached = cacheValue.value();
                                packetCached.Header.ID = packet.Header.ID;
                                m_Socket->send_to(NET::Buffer(DNS::CreateBufferFromPackage(packetCached)), point);

                            } else {
                                NET::SocketUDP socket = {m_Service, NET::UDPoint(NET::UDP::v4(), 0)};
                                socket.send_to(NET::Buffer(buffer), NET::UDPoint(NET::Address("5.3.3.3"), 53));
                                size_t size = socket.receive(NET::Buffer(buffer));
                                m_Socket->send_to(NET::Buffer(buffer, size), point);
                                //   m_Cache->Add(packet.Questions[0].Name, DNS::CreatePackageFromBuffer(buffer));
                            }
                        } catch (NET::Exeception& error) {
                            if (error.code() != boost::asio::error::interrupted)
                                fmt::print("Error: {} \n", error.what());
                        }
                    });

                } catch(NET::Exeception& error) {             
                    if (error.code() != boost::asio::error::interrupted) 
                        fmt::print("Error: {} \n", error.what());                                 
                }  
            }
        }));
        m_Dispather.join();
    }

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

int main(int argc, char* argv[]) {
    DNSServer server(argc, argv);
    server.Run();
}