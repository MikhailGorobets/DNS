#pragma once

#define _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING 1

#include <thread>
#include <atomic>
#include <array>
#include <experimental/filesystem>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/optional/optional.hpp>

#include "dns.hpp"
#include "dns_cache.hpp"

struct DNSServerCreateInfo {
public:
	using Address = std::pair<std::string, uint16_t>;
	using AddressList = std::vector<Address>;
public:
	AddressList DNSServers;
	Address     BindAdress;
	std::string FileNameCache = ""; //Если строка пустая то создать кэш
public:
	static auto CreateFromFile(std::string const& fileName)->DNSServerCreateInfo;
};


class DNSServer {
public:
	using Address = std::pair<std::string, uint16_t>;
	using AddressList = std::vector<Address>;
	using Service = boost::asio::io_service;
	using Sock = boost::asio::ip::udp::socket;
	using SockPtr = std::shared_ptr<Sock>;
	using Thread = std::thread;
	using ThreadPtr = std::unique_ptr<Thread>;
	using Result = boost::optional<boost::system::error_code>;
	using Timer = boost::asio::steady_timer;
	using Endpoint = boost::asio::ip::udp::endpoint;
	using Buffer = std::array<uint8_t, DNS_PACKAGE_SIZE>;
public:
	DNSServer(DNSServerCreateInfo const& desc);
	auto Run() -> void;
	auto Shutdown() -> void;
private:
	auto Connection(Endpoint const& endpoint, DNS::Package const& query) -> void;
	auto WaitTimeout(SockPtr sock, Timer& timer, Result& operationResult, Result& timerResult) -> bool;
private:
	DNS::Cache        m_Cache;
	Service           m_IOService;
	SockPtr           m_UDPSock;
	AddressList       m_DNSServers;
	ThreadPtr         m_ThreadUpdateCache;
	ThreadPtr         m_ThreadMain;
	std::string       m_FileNameCache;
	std::atomic<bool> m_IsRunApplication = false;
	std::atomic<bool> m_IsExit = false;

};