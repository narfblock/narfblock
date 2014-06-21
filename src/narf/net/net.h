#ifndef NARF_NET_H
#define NARF_NET_H

#include <stdint.h>
#include <string>

#include <enet/enet.h>

namespace narf {

	namespace net {

		const size_t MAX_CHANNELS = 2; // TODO

		const uint8_t CHAN_CHAT = 0;
		const uint8_t CHAN_PLAYERCMD = 1;

		const uint16_t DEFAULT_PORT = 8686;

		enum class DisconnectType {
			Timeout = 0,
			UserQuit,
		};

		// split net address into host and (optional) port
		bool splitHostPort(const std::string &addr, std::string &host, uint16_t &port);

		std::string to_string(const ENetAddress& address);
	}
}

#endif // NARF_NET_H
