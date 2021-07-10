#include <dns/server.h>
#include <fmt/printf.h>
#include <fmt/ostream.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>

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

    template<typename... Args>
    auto Buffer(Args&&... args) -> decltype(boost::asio::buffer(std::forward<Args>(args)...)) {
        return boost::asio::buffer(std::forward<Args>(args)...);
    }

    template<typename... Args>
    auto Address(Args&&... args) -> decltype(boost::asio::ip::make_address_v4(std::forward<Args>(args)...)) {
        return boost::asio::ip::make_address_v4(std::forward<Args>(args)...);
    }
}

template<typename T>
class ThreadSafeQueue final {
public:
    ThreadSafeQueue() = default;

    auto Push(T&& value) -> void {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_Queue.push(value);
        lock.unlock();
        m_Condition.notify_one();
    }

    auto Push(const T& value) -> void {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_Queue.push(value);
        lock.unlock();
        m_Condition.notify_one();
    }

    auto Pop() -> std::optional<T> {
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_Condition.wait(lock, [this]() { return !m_Queue.empty() || !m_IsValid; });
        if (!m_IsValid)
            return std::nullopt;

        auto value = std::move(m_Queue.front());
        m_Queue.pop();
        return value;
    }

    auto IsEmpty() const -> bool {
        std::unique_lock<std::mutex> lock(m_Mutex);
        return m_Queue.empty();
    }

    auto Invalidate() -> void {
        m_IsValid = false;
        m_Condition.notify_all();
    }

private:
    std::atomic_bool        m_IsValid = {true};
    std::queue<T>           m_Queue = {};
    mutable std::mutex      m_Mutex = {};
    std::condition_variable m_Condition = {};
};

class ThreadPool final {
public:
    using Task = std::function<void()>;
    using TaskQueue = ThreadSafeQueue<std::function<void()>>;
    using ListThread = std::vector<std::thread>;
public:
    ThreadPool(std::optional<uint32_t> threadCount = std::nullopt) {
        uint32_t hardwareThreads = std::thread::hardware_concurrency();
        for (size_t index = 0; index < threadCount.value_or(hardwareThreads); index++) {
            m_Threads.emplace_back([this]() {
                while (true) {
                    auto packagedTask = m_Queue.Pop();
                    if (!packagedTask.has_value())
                        break;
                    m_ActiveThreads++;
                    (*packagedTask)();
                    m_ActiveThreads--;
                    m_ThreadsCompleteCondition.notify_all();
                }
            });
        }
    }

    ~ThreadPool() {
        m_Queue.Invalidate();
        for (auto& thread : m_Threads)
            thread.join();
    }

    auto AddTask(Task&& task) -> void {
        m_Queue.Push(task);
    }

    auto Wait() -> void {
        std::unique_lock<std::mutex> lock(m_ThreadsCompleteMutex);
        m_ThreadsCompleteCondition.wait(lock, [this] { return !m_ActiveThreads && m_Queue.IsEmpty(); });
    }

    auto Abort() -> void {
        m_Queue.Invalidate();
        Wait();
    }

private:
    ListThread m_Threads = {};
    TaskQueue  m_Queue = {};
    std::atomic<uint32_t>   m_ActiveThreads = {};
    std::mutex              m_ThreadsCompleteMutex = {};
    std::condition_variable m_ThreadsCompleteCondition = {};
};

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
        m_Socket = std::make_unique<NET::SocketUDP>(m_Service, NET::UDPoint(NET::UDP::v4(), 53));
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
        fmt::print("Application Run: \n");
        fmt::print("IP:   {}\n", m_Socket->local_endpoint().address().to_string());
        fmt::print("Port: {}\n", m_Socket->local_endpoint().port());

        m_IsApplicationRun = true;
        m_Dispather.AddTask([this]() {
            m_SignalSet->async_wait([&](auto const& error, int32_t signal) {
                m_IsApplicationRun = false;
                fmt::print("Application Shutdown: \n");
            });
            m_Service.run();
        });
        m_Dispather.AddTask([this]() {
            while (m_IsApplicationRun) {
                NET::UDPoint sender;
                std::vector<uint8_t> buffer(DNS::PACKAGE_SIZE);
                size_t size = m_Socket->receive_from(NET::Buffer(buffer), sender);
                m_Connections.Push(std::make_tuple(std::move(sender), std::move(buffer)));
            }
        });
        m_Dispather.AddTask([this]() {
            while (m_IsApplicationRun) {
                auto connection = m_Connections.Pop();
                m_Dispather.AddTask([this, connection = std::move(connection)]() mutable {
                    auto& [point, buffer] = *connection;

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
        });
        m_Dispather.Wait();
    }

private:
    using ConnectionContext = std::tuple<NET::UDPoint, std::vector<uint8_t>>;
    using ConnectionList = ThreadSafeQueue<ConnectionContext>;

    using PtrDNSCache = std::unique_ptr<DNSCache>;
    using PtrSocketUDP = std::unique_ptr<NET::SocketUDP>;
    using PtrSignalSet = std::unique_ptr<NET::SignalSet>;

    std::atomic_bool m_IsApplicationRun = {};
    ConnectionList   m_Connections = {};
    ThreadPool       m_Dispather = {};
    PtrDNSCache      m_Cache = {};
    NET::IOContext   m_Service = {};
    PtrSignalSet     m_SignalSet = {};
    PtrSocketUDP     m_Socket = {};
    std::mutex       m_MutexSocket = {};
};

int main(int argc, char* argv[]) {
    DNSServer server(argc, argv);
    server.Run();
}