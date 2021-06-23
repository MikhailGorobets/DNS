#include <dns/server.h>
#include <fmt/printf.h>
#include <fmt/ostream.h>
#include <memory>
#include <iostream>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

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


int main(int argc, char* argv[]) {


    NET::IOSerivce service = {};       
    NET::SocketUDP socketRCV = { service, NET::UDPoint(NET::UDP::v4(), 256)};
    NET::SocketUDP socketSND = { service, NET::UDPoint(NET::UDP::v4(), 0) };
    socketSND.connect(NET::UDPoint(boost::asio::ip::make_address_v4("5.3.3.3"), 53));
   

    fmt::print("Application Run: \n");
    fmt::print("IP:   {}\n", socketRCV.local_endpoint().address().to_string());
    fmt::print("Port: {}\n", socketRCV.local_endpoint().port());

    uint32_t interation = 0;
    while (true) {
        try {
            NET::UDPoint sender;

            std::vector<uint8_t> buffer(DNS::PACKAGE_SIZE);
            socketRCV.receive_from(NET::Buffer(buffer), sender);
                       

            socketSND.send(NET::Buffer(buffer));
            socketSND.receive(NET::Buffer(buffer));
            DNS::Package packet = DNS::CreatePackageFromBuffer(buffer);
   
            socketRCV.send_to(NET::Buffer(DNS::CreateBufferFromPackage(packet)), sender);
          
        } catch(std::exception const& e) {
            fmt::print("Error: {}\n ", e.what());
        }
    }
}