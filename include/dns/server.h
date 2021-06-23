#pragma once


#include <thread>
#include <atomic>
#include <array>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/optional/optional.hpp>
#include <filesystem>

#include "dns.hpp"
#include "dns_cache.hpp"

//
//using Address     = std::pair<std::string, uint16_t>;
//using AddressList = std::vector<Address>;
//
//
//struct DNSServerCreateInfo {
//	AddressList DNSServers;
//	Address     BindAdress;
//	std::string FileNameCache = ""; 
//
//};
//
//
//class DNSServer {
//    using Service = boost::asio::io_service;
//    using Sock = boost::asio::ip::udp::socket;
//    using SockPtr = std::shared_ptr<Sock>;
//    using Thread = std::thread;
//    using ThreadPtr = std::unique_ptr<Thread>;
//    using Result = boost::optional<boost::system::error_code>;
//    using Timer = boost::asio::steady_timer;
//    using Endpoint = boost::asio::ip::udp::endpoint;
//    using Buffer = std::array<uint8_t, DNS::PACKAGE_SIZE>;
//public:
//	DNSServer(DNSServerCreateInfo const& desc);
//	auto Run() -> void;
//	auto Shutdown() -> void;
//private:
//	auto Connection(Endpoint const& endpoint, DNS::Package const& query) -> void;
//	auto WaitTimeout(SockPtr sock, Timer& timer, Result& operationResult, Result& timerResult) -> bool;
//private:
//	DNS::Cache        m_Cache;
//	Service           m_IOService;
//	SockPtr           m_UDPSock;
//	AddressList       m_DNSServers;
//	ThreadPtr         m_ThreadUpdateCache;
//	ThreadPtr         m_ThreadMain;
//	std::string       m_FileNameCache;
//};