#ifndef NARF_NET_PROTOCOL_H
#define NARF_NET_PROTOCOL_H

#include <stdint.h>

namespace narf {
	namespace net {

		const uint8_t CHAN_CHAT			= 0;
		const uint8_t CHAN_PLAYERCMD	= 1;
		const uint8_t CHAN_CHUNK		= 2;
		const uint8_t CHAN_ENTITY		= 3;
		const size_t MAX_CHANNELS		= 4;

		const uint16_t DEFAULT_PORT = 8686;

		enum class DisconnectType {
			Timeout = 0,
			UserQuit,
		};
	}
}

#endif // NARF_NET_PROTOCOL_H
