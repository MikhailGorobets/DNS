#include <dns/dns_server.hpp>

int main(int argc, char* argv[]) {
    DNSServer server(argc, argv);
    server.Run();
}