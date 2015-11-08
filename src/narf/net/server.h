#ifndef NARF_NET_SERVER_H
#define NARF_NET_SERVER_H

#include "narf/chunkcache.h"
#include "narf/entity.h"
#include "narf/world.h"
#include "narf/net/net.h"

#include <queue>

namespace narf {
	namespace net {
		// server-side representation of a connected client
		class Client {
		public:

			Client() : peer(nullptr) {
			}

			ENetPeer* peer;

			// entity this player is controlling/spectating
			Entity::ID entityID;
		};

		// server chunk state
		// TODO: track which clients have received which chunks
		class ChunkState {
		public:
			ChunkState() : dirty(true) {}
			~ChunkState() {}

			bool dirty;
		};

		class Server {
		public:
			Server(size_t maxClients, uint16_t port);
			~Server();

			void tick(narf::timediff dt);

			std::string formatChat(const Client* from, const std::string& text);
			void tell(const Client* to, const Client* from, const std::string& text);
			void tellAll(const Client* from, const std::string& text);

			World* world; // TODO: this should probably be private

		private:
			void genWorld();

			void chunkUpdate(const ChunkCoord& cc);
			void blockUpdate(const BlockCoord& wbc);
			bool clientChunkDirty(const Client* client, const ChunkCoord& cc);
			void markClientChunkClean(const Client* client, const ChunkCoord& cc);
			void markChunksClean();
			void sendChunkUpdate(const Client* to, const ChunkCoord& wcc, bool dirtyOnly);
			void sendEntityUpdate(const Client* to, const Entity& ent);
			void sendDeletedEntityUpdate(const Client* to, Entity::ID id);
			void onEntityDeleted(Entity::ID id);
			void sendPlayerCameraUpdate(const Client* to, Entity::ID followID);

			void processConnect(ENetEvent& evt);
			void processDisconnect(ENetEvent& evt);
			void processChat(ENetEvent& evt, net::Client* client);
			void processPlayerCommand(ENetEvent& evt, net::Client* client);
			void processReceive(ENetEvent& evt);
			void processNetEvent(ENetEvent& evt);

			ENetHost* server;
			size_t maxClients;
			Client* clients;

			std::deque<Entity::ID> deletedEntities;
			ChunkCache<ChunkCoord, ChunkState> chunkCache;
		};
	}
}

#endif // NARF_NET_PROTOCOL_H
