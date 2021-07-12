import socket
import multiprocessing

import dns.name
import dns.message
import dns.query
import dns.flags

HOST = '127.0.0.1'  # The server's hostname or IP address
PORT = 57         # The port used by the server
ADDITIONAL_RDCLASS = 65535


QUERY_LIST = ["google.com", "microsoft.com", "twitter.com", "stackoverflow.com", "github.com", "youtube.com", "google.com", "microsoft.com", "twitter.com", "stackoverflow.com", "github.com", "youtube.com", "google.com", "microsoft.com", "twitter.com", "stackoverflow.com", "github.com", "youtube.com"]


def DNSQuery(name):
	for queryID in range(0, 10000):
		domain = dns.name.from_text(name)
		if not domain.is_absolute():
			domain = domain.concatenate(dns.name.root)

		request = dns.message.make_query(domain, dns.rdatatype.TXT)
		request.flags |= dns.flags.AD
		request.find_rrset(request.additional, dns.name.root, ADDITIONAL_RDCLASS, dns.rdatatype.OPT, create=True, force_unique=True)

		data = dns.query.udp(request, HOST, timeout=5, port=PORT)
		#print(name)

if __name__ == '__main__':
    processList = []
    for name in QUERY_LIST:
        process = multiprocessing.Process(target=DNSQuery, args=(name, ))
        processList.append(process)

    for process in processList:
        process.start()

    for process in processList:
        process.join()

