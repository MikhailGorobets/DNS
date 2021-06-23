import socket

HOST = '127.0.0.1' # The server's hostname or IP address
PORT = 256         # The port used by the server
ADDITIONAL_RDCLASS = 65535

import dns.name
import dns.message
import dns.query
import dns.flags
 
domain = dns.name.from_text("google.com")
if not domain.is_absolute():
	domain = domain.concatenate(dns.name.root)
 
request = dns.message.make_query(domain, dns.rdatatype.TXT)
request.flags |= dns.flags.AD
request.find_rrset(request.additional, dns.name.root, ADDITIONAL_RDCLASS, dns.rdatatype.OPT, create=True, force_unique=True)

data = dns.query.udp(request, HOST, timeout=5, port=PORT)
print(data)