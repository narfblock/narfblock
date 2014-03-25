#ifndef NARF_NET_H
#define NARF_NET_H

#include <string>

namespace narf {

	namespace net {

		// split net address into host and (optional) port
		bool splitHostPort(const std::string &addr, std::string &host, uint16_t &port);

	}
}

#endif // NARF_NET_H
