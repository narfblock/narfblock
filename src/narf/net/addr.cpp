#include "net.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

bool narf::net::splitHostPort(const std::string &addr, std::string &host, uint16_t &port) {
	size_t portPos;
	host = addr;
	port = 0;
	if (addr[0] == '[') {
		// [host]:port
		size_t hostEnd = addr.find(']');
		if (hostEnd == std::string::npos) {
			// missing closing bracket
			return false;
		}

		host = addr.substr(1, hostEnd - 1);
		portPos = addr.find(':', hostEnd + 1);
	} else {
		portPos = addr.find(':');
		if (portPos != std::string::npos) {
			host = addr.substr(0, portPos);
		}
	}

	if (host.find_first_of("[]") != std::string::npos) {
		// stray brackets in address
		return false;
	}

	if (portPos == std::string::npos) {
		port = 0;
	} else {
		auto portStr = addr.substr(portPos + 1);
		if (portStr.find(':') != std::string::npos) {
			// too many colons
			// TODO: handle bare IPv6 addresses with no port?
			return false;
		}

		char *end;
		unsigned long p = strtoul(portStr.c_str(), &end, 10);
		if (!end || *end != '\0' || p == 0 || p > 65535ul) {
			// invalid port
			return false;
		}

		port = (uint16_t)p;
	}

	return true;
}


std::string narf::net::to_string(const ENetAddress& address) {
	char buf[3 * 4 + 3 + 1 + 5 + 1]; // 3-digit octet * 4 octets + 3 dots + colon + 5-digit port + terminator
	uint32_t host = ntohl(address.host);
	snprintf(buf, sizeof(buf), "%u.%u.%u.%u:%u", host >> 24, (host >> 16) & 0xFF, (host >> 8) & 0xFF, host & 0xFF, address.port);
	return buf;
}
