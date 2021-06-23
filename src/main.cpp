#include <dns/server.h>
#include <fmt/printf.h>
#include <fmt/ostream.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <unordered_map>

namespace NET {
    using SocketUDP = boost::asio::ip::udp::socket;
    using SocketTCP = boost::asio::ip::tcp::socket;

    using UDP = boost::asio::ip::udp;
    using UDPoint = boost::asio::ip::udp::endpoint;

    using TCP = boost::asio::ip::tcp;
    using TCPPoint = boost::asio::ip::tcp::endpoint;

    using IOSerivce = boost::asio::io_service;
    
    template<typename... Args> \
    auto Buffer(Args&&... args) -> decltype(boost::asio::buffer(std::forward<Args>(args)...))  { 
        return boost::asio::buffer(std::forward<Args>(args)...); 
    }
}   

class QueueMT {

};

class Scheduler {


};

class LRUCache {

};


int main(int argc, char* argv[]) {

    NET::IOSerivce service = {};       
    NET::SocketUDP socketRCV = { service, NET::UDPoint(NET::UDP::v4(), 53)};
    NET::SocketUDP socketSND = { service, NET::UDPoint(NET::UDP::v4(), 0) };
    socketSND.connect(NET::UDPoint(boost::asio::ip::make_address_v4("5.3.3.3"), 53));

    fmt::print("Application Run: \n");
    fmt::print("IP:   {}\n", socketRCV.local_endpoint().address().to_string());
    fmt::print("Port: {}\n", socketRCV.local_endpoint().port());

    std::unordered_map<std::string, DNS::Package> Cache;

    while (true) {
        try {
            NET::UDPoint sender;

            std::vector<uint8_t> buffer(DNS::PACKAGE_SIZE);

            {
                socketRCV.receive_from(NET::Buffer(buffer), sender);
                DNS::Package packet = DNS::CreatePackageFromBuffer(buffer);
             
                if (packet.Questions.size() > 0) {
                    if (auto iter = Cache.find(packet.Questions[0].Name); iter != Cache.end()) {
                        fmt::print("Cache hit \n");

                        DNS::Package packetCached = iter->second;
                        packetCached.Header.ID = packet.Header.ID;
                        socketRCV.send_to(NET::Buffer(DNS::CreateBufferFromPackage(packetCached)), sender);
                    } else {
                        fmt::print("Cache miss \n");
                        
                       
                        socketSND.send(NET::Buffer(buffer));
                        size_t size = socketSND.receive(NET::Buffer(buffer));
                        socketRCV.send_to(NET::Buffer(buffer, size), sender);
                        Cache.emplace(packet.Questions[0].Name, DNS::CreatePackageFromBuffer(buffer));
                    }   
                } else {
                    fmt::print("Error: Don't correct Header::CountQuestion {}", packet.Questions.size());
                }        
            }
          
        } catch(std::exception const& e) {
            fmt::print("Error: {} \n", e.what());
        }
    }
}