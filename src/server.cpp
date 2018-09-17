
#define _WIN32_WINNT 0x0601

#include "server.h"
#include <boost/property_tree/json_parser.hpp>




[[nodiscard]] auto DNSServerCreateInfo::CreateFromFile(std::string const & fileName) -> DNSServerCreateInfo {

	boost::property_tree::ptree config;
	boost::property_tree::read_json(fileName, config);
	DNSServerCreateInfo desc;
	desc.FileNameCache = config.get<std::string>("FileNameCache");
	desc.BindAdress = { config.get_child("BindAddress").get<std::string>("IP"), config.get_child("BindAddress").get<uint16_t>("Port") };

	for (auto const&[_, tree] : config.get_child("DNSServers"))
		desc.DNSServers.push_back({ tree.get<std::string>("IP"), tree.get<uint16_t>("Port") });

	return desc;
}


DNSServer::DNSServer(DNSServerCreateInfo const& desc) {
	namespace fs = std::experimental::filesystem;
	m_UDPSock = std::make_shared<Sock>(m_IOService);
	m_UDPSock->open(boost::asio::ip::udp::v4());
	m_UDPSock->non_blocking(true);
	m_UDPSock->bind({ boost::asio::ip::address::from_string(desc.BindAdress.first), desc.BindAdress.second });
	m_FileNameCache = desc.FileNameCache;
	m_DNSServers = desc.DNSServers;
	m_IsRunApplication = true;
	if (fs::exists(fs::path{ m_FileNameCache }))
		m_Cache = DNS::Cache::Load(m_FileNameCache);

}


auto DNSServer::Run() -> void {

	std::cout << "Application run ";
	std::cout << " IP: " << m_UDPSock->local_endpoint().address().to_string();
	std::cout << " Port:" << m_UDPSock->local_endpoint().port() << std::endl;

	m_ThreadUpdateCache = std::make_unique<Thread>([&] {
		while (m_IsRunApplication) {
			auto currentTime = std::time(nullptr);
			m_Cache.Update(currentTime);
			std::this_thread::sleep_for(std::chrono::seconds{ 5 });
		}
	});

	m_ThreadMain = std::make_unique<Thread>([&] {
		while (m_IsRunApplication) {
			Result reciverClientResult;
			Result timerResult;
			size_t dataSize;

			auto timerHandler = [&](auto const& err)                    -> void { timerResult.reset(err); };
			auto reciverClientHandler = [&](auto const& err, auto size) -> void { reciverClientResult.reset(err); dataSize = size; };

			Buffer buffer;
			Endpoint endpoint;
			Timer timer{ m_IOService };

			timer.expires_from_now(std::chrono::seconds(2));
			timer.async_wait(timerHandler);
			m_UDPSock->async_receive_from(boost::asio::buffer(buffer), endpoint, reciverClientHandler);
			m_IOService.reset();
			if (!this->WaitTimeout(m_UDPSock, timer, reciverClientResult, timerResult))
				continue;

			this->Connection(endpoint, DNS::Package::CreatePackageFromBuffer(buffer));
		}
	});

	m_ThreadUpdateCache->join();
	m_ThreadMain->join();
	DNS::Cache::Save(m_FileNameCache, m_Cache);
	m_IsExit = true;
}

auto DNSServer::Shutdown() -> void {
	std::cout << "Application shutdown " << std::endl;
	if (m_IsRunApplication) {
		m_IsRunApplication = false;
		while (!m_IsExit);
	}
}

auto DNSServer::Connection(Endpoint const& endpoint, DNS::Package const& query) -> void {

	

	if (m_Cache.ContainsKey({ query.Header.Flags, query.Questions.front().Name, query.Questions.front().Question })) {


		auto name = query.Questions.front().Name;
		name.erase(std::remove(name.begin(), name.end(), 0x07), name.end());

		std::cout << "Get from cache: " << name.data() << std::endl;

		Result senderResult;
		Result timerResult;

		auto timerHandler = [&](auto const& err)             -> void { timerResult.reset(err); };
		auto senderHandler = [&](auto const& err, auto size) -> void { senderResult.reset(err); };

		Timer timer(m_IOService);
		auto answer = m_Cache.Get({ query.Header.Flags, query.Questions.front().Name, query.Questions.front().Question });
		answer.Header.ID = query.Header.ID;
		{
			timer.expires_from_now(std::chrono::seconds(2));
			timer.async_wait(timerHandler);
			m_UDPSock->async_send_to(boost::asio::buffer(DNS::Package::CreateBufferFromPackage(answer), DNS::Package::ComputeSize(answer)), endpoint, senderHandler);
			m_IOService.reset();
			this->WaitTimeout(m_UDPSock, timer, senderResult, timerResult);
		}

	} else {
		for (auto const&[ip, port] : m_DNSServers) {


			Result senderServerResult;
			Result senderClientResult;
			Result recieverServerResult;
			Result timerResult;

			auto timerHandler = [&](auto const& err)            -> void { timerResult.reset(err); };
			auto senderServerHandler = [&](auto const& err, auto size) -> void { senderServerResult.reset(err); };
			auto senderClientHandler = [&](auto const& err, auto size) -> void { senderClientResult.reset(err); };
			auto receiverServerHandler = [&](auto const& err, auto size) -> void { recieverServerResult.reset(err); };

			Timer timer{ m_IOService };
			auto sock = std::make_shared<Sock>(m_IOService, Endpoint{ boost::asio::ip::udp::v4(), 0 });

			{
				timer.expires_from_now(std::chrono::seconds(2));
				timer.async_wait(timerHandler);
				sock->async_send_to(boost::asio::buffer(DNS::Package::CreateBufferFromPackage(query), DNS::Package::ComputeSize(query)), { boost::asio::ip::address::from_string(ip), port }, senderServerHandler);
				m_IOService.reset();
				if (!this->WaitTimeout(sock, timer, senderServerResult, timerResult))
					continue;
			}

			DNS::Package answer;
			{
				Buffer buffer;
				timer.expires_from_now(std::chrono::seconds(2));
				timer.async_wait(timerHandler);
				sock->async_receive(boost::asio::buffer(buffer), receiverServerHandler);
				m_IOService.reset();
				if (!this->WaitTimeout(sock, timer, recieverServerResult, timerResult))
					continue;
				answer = std::move(DNS::Package::CreatePackageFromBuffer(buffer));
				answer.Header.ID = query.Header.ID;
			}

			{
				timer.expires_from_now(std::chrono::seconds(2));
				timer.async_wait(timerHandler);
				m_UDPSock->async_send_to(boost::asio::buffer(DNS::Package::CreateBufferFromPackage(answer), DNS::Package::ComputeSize(answer)), endpoint, senderClientHandler);
				m_IOService.reset();
				if (!this->WaitTimeout(m_UDPSock, timer, senderClientResult, timerResult))
					continue;

			}
			m_Cache.Insert({ query.Header.Flags, answer.Questions.front().Name, answer.Questions.front().Question }, answer);
			break;

		}
	}

}

auto DNSServer::WaitTimeout(SockPtr sock, Timer& timer, Result& operationResult, Result& timerResult) -> bool
{
	while (m_IOService.run_one()) {
		if (operationResult)
			timer.cancel();
		else if (timerResult)
			sock->cancel();
	}
	return (*operationResult) ? false : true;
}
