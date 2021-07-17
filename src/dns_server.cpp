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


#include <dns/dns_server.hpp>
#include <fmt/printf.h>

 DNSServer::DNSServer(int argc, char* argv[]) {
    m_Cache = std::make_unique<DNSCache>();
    m_SignalSet = std::make_unique<NET::SignalSet>(m_Service, SIGINT, SIGTERM);
    m_Socket = std::make_unique<NET::SocketUDP>(m_Service, NET::UDPoint(NET::UDP::v4(), 57));
    m_Socket->set_option(boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>{ 200 });

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

auto DNSServer::Run() -> void {
    fmt::print("DNS Server: Run \n");
    fmt::print("DNS Server: IP: {}, Port: {} \n", m_Socket->local_endpoint().address().to_string(), m_Socket->local_endpoint().port());

    m_IsApplicationRun = true;

    //Run a thread which remove DNS packet with timeout TTL
    NET::Post(m_Dispather, [this]() {
        while (m_IsApplicationRun.load())
            m_Cache->RemoveTimeoutPackagesWaitFor(60, [this]()-> bool { return !m_IsApplicationRun; });
    });

    //Run a thread which to process signals
    NET::Post(m_Dispather, [this]() {
        m_SignalSet->async_wait([&](auto const& error, int32_t signal) {
            m_IsApplicationRun.store(false);
            m_Dispather.stop();
            fmt::print("DNS Server: Shutdown \n");
        });
        m_Service.run();
    });

    //Run a thread which accept DNS questions
    NET::Post(m_Dispather, ([this]() {
        while (m_IsApplicationRun.load()) {

            NET::UDPoint point;
            std::vector<uint8_t> buffer(DNS::PACKAGE_SIZE);

            NET::Error result;
            m_Socket->receive_from(NET::Buffer(buffer), point, {}, result);
            switch (result.value()) {
                case NET::ErrorType{}:
                    break;
                case NET::ErrorType::timed_out:
                    continue;
                default:
                    fmt::print("Error: {} \n", result.message());
                    continue;
            }

            //Run a thread which to pricess a DNS question
            NET::Post(m_Dispather, [this, point = std::move(point), buffer = std::move(buffer)]() mutable {
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
                    m_Cache->Add(packet.Questions[0].Name, DNS::CreatePackageFromBuffer(buffer));
                }
            });
        }
    }));
    m_Dispather.join();
}
