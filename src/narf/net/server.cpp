#include "narf/console.h"
#include "narf/playercmd.h"
#include "narf/net/protocol.h"
#include "narf/net/server.h"

// TODO: move these
#define WORLD_X_MAX 64
#define WORLD_Y_MAX 64
#define WORLD_Z_MAX 64

using namespace narf;


std::string net::to_string(const ENetAddress& address) {
	char buf[3 * 4 + 3 + 1 + 5 + 1]; // 3-digit octet * 4 octets + 3 dots + colon + 5-digit port + terminator
	uint32_t host = ntohl(address.host);
	snprintf(buf, sizeof(buf), "%u.%u.%u.%u:%u", host >> 24, (host >> 16) & 0xFF, (host >> 8) & 0xFF, host & 0xFF, address.port);
	return buf;
}


net::Server::Server(size_t maxClients, uint16_t port) : maxClients(maxClients) {
	if (enet_initialize() != 0) {
		console->println("Error initializing ENet");
		// TODO throw
		return;
	}

	console->println("Hosting server with maxClients = " + std::to_string(maxClients));
	clients = new Client[maxClients];

	ENetAddress bindAddress;
	bindAddress.host = ENET_HOST_ANY;
	bindAddress.port = port;

	console->println("Binding to " + net::to_string(bindAddress));
	server = enet_host_create(&bindAddress, maxClients, net::MAX_CHANNELS, 0, 0);
	if (server == nullptr) {
		console->println("Could not bind to " + net::to_string(bindAddress));
	}

	genWorld(); // TODO: allow loading here too
}


net::Server::~Server() {
	if (server) {
		enet_host_destroy(server);
	}
	enet_deinitialize();
	if (clients) {
		delete[] clients;
	}
}


void net::Server::chunkUpdate(const ChunkCoord& cc) {
	// TODO: total hax
	auto chunkState = chunkCache.get(cc);
	if (!chunkState) {
		chunkCache.put(cc, ChunkState());
		chunkState = chunkCache.get(cc);
	}
	if (chunkState) {
		chunkState->dirty = true;
	}
}


void net::Server::blockUpdate(const BlockCoord& wbc) {
	ChunkCoord cc;
	Chunk::BlockCoord cbc;
	world->calcChunkCoords(wbc, cc, cbc);
	chunkUpdate(cc);
}


void net::Server::genWorld() {
	world = new World(WORLD_X_MAX, WORLD_Y_MAX, WORLD_Z_MAX, 16, 16, 16);
	world->setGravity(-24.0f);

	world->chunkUpdate = [this](const ChunkCoord& cc) { chunkUpdate(cc); };
	world->blockUpdate = [this](const BlockCoord& wbc) { blockUpdate(wbc); };
	world->entityManager.onEntityDeleted += [this](Entity::ID id) { onEntityDeleted(id); };

	// add test entity
	auto bouncyBlockEID = world->entityManager.newEntity();
	{
		EntityRef bouncyBlock(world->entityManager, bouncyBlockEID);
		bouncyBlock->position = Vector3f(10.0f, 10.0f, 21.0f);
		bouncyBlock->prevPosition = bouncyBlock->position;
		bouncyBlock->bouncy = true;
		bouncyBlock->model = true;
	}
}


std::string net::Server::formatChat(const Client* from, const std::string& text) {
	// TODO: use client nickname here
	std::string fromName = from ? to_string(from->peer->address) : "server";
	return "<" + fromName + "> " + text;
}


void net::Server::tell(const Client* to, const Client* from, const std::string& text) {
	std::string packetData(formatChat(from, text));
	auto packet = enet_packet_create(packetData.c_str(), packetData.length(), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(to->peer, CHAN_CHAT, packet);
}


void net::Server::tellAll(const Client* from, const std::string& text) {
	for (size_t i = 0; i < maxClients; i++) {
		auto client = &clients[i];
		if (client->peer) {
			tell(client, from, text);
		}
	}
}


bool net::Server::clientChunkDirty(const Client* client, const ChunkCoord& cc) {
	auto chunkState = chunkCache.get(cc);
	if (chunkState) {
		return chunkState->dirty;
	}
	return false;
}


void net::Server::markClientChunkClean(const Client* client, const ChunkCoord& cc) {
	auto chunkState = chunkCache.get(cc);
	if (chunkState && chunkState->dirty) {
		chunkState->dirty = false;
	}
}


void net::Server::sendChunkUpdate(const Client* to, const ChunkCoord& wcc, bool dirtyOnly) {
	auto chunk = world->getChunk(wcc);
	if (!dirtyOnly || clientChunkDirty(to, wcc)) {
		ByteStream bs;
		world->serializeChunk(bs, wcc);
		auto packet = enet_packet_create(bs.data(), bs.size(), ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(to->peer, CHAN_CHUNK, packet);
	}
}


void net::Server::markChunksClean() {
	ZYXCoordIter<ChunkCoord> iter({ 0, 0, 0 }, { world->chunksX(), world->chunksY(), world->chunksZ() });
	for (const auto& wcc : iter) {
		for (size_t i = 0; i < maxClients; i++) {
			auto client = &clients[i];
			if (client->peer) {
				markClientChunkClean(client, wcc);
			}
		}
	}
}


void net::Server::sendEntityUpdate(const Client* to, const Entity& ent) {
	ByteStream bs;
	world->entityManager.serializeEntityFullUpdate(bs, ent);
	auto packet = enet_packet_create(bs.data(), bs.size(), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(to->peer, CHAN_ENTITY, packet);
}


void net::Server::sendDeletedEntityUpdate(const Client* to, Entity::ID id) {
	ByteStream bs;
	world->entityManager.serializeEntityDelete(bs, id);
	auto packet = enet_packet_create(bs.data(), bs.size(), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(to->peer, CHAN_ENTITY, packet);
}


void net::Server::onEntityDeleted(Entity::ID id) {
	console->println("entity " + std::to_string(id) + " deleted");
	deletedEntities.push_back(id);
}


void net::Server::sendPlayerCameraUpdate(const Client* to, Entity::ID followID) {
	ByteStream bs;
	// TODO: put this somewhere better
	// TODO: for now, this is the only server->client message on CHAN_PLAYERCMD,
	// but this should have a message type later.
	bs.write(followID, LE);
	auto packet = enet_packet_create(bs.data(), bs.size(), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(to->peer, CHAN_PLAYERCMD, packet);
}


void net::Server::processConnect(ENetEvent& evt) {
	console->println("Client connected from " + net::to_string(evt.peer->address));
	// add the client to list of connected clients
	// find an unused client
	Client* client = nullptr;
	for (unsigned i = 0; i < maxClients; i++) {
		if (!clients[i].peer) {
			client = &clients[i];
			break;
		}
	}

	if (client == nullptr) {
		// TODO!!!!
		console->println("ERROR: No client slots available");
		enet_peer_reset(evt.peer);
		return;
	}

	client->peer = evt.peer;

	// store Client pointer in peer data
	evt.peer->data = client;

	// send a server message greeting
	tellAll(nullptr, "Client connected from " + to_string(evt.peer->address));

	// spawn a new entity for this player (TODO: allow 'spectate mode', etc?)
	client->entityID = world->entityManager.newEntity();
	{
		EntityRef player(world->entityManager, client->entityID);

		// initial player position
		player->position = Vector3f(15.0f, 10.0f, 3.0f * 16.0f);
		player->prevPosition = player->position;
	}
	sendPlayerCameraUpdate(client, client->entityID);

	// send all chunks (!!)
	ZYXCoordIter<ChunkCoord> iter({ 0, 0, 0 }, { world->chunksX(), world->chunksY(), world->chunksZ() });
	for (const auto& wcc : iter) {
		sendChunkUpdate(client, wcc, false);
	}
}


void net::Server::processDisconnect(ENetEvent& evt) {
	auto client = static_cast<Client*>(evt.peer->data);
	auto disconnectType = static_cast<DisconnectType>(evt.data);
	std::string reason;
	switch (disconnectType) {
	case DisconnectType::Timeout:
		reason = "timed out";
		break;
	case DisconnectType::UserQuit:
		reason = "user quit";
		break;
	default:
		reason = "unknown";
		break;
	}

	std::string disconnectMsg = "Client disconnected from " + to_string(evt.peer->address) + " (" + reason + ")";
	client->peer = nullptr;
	evt.peer->data = nullptr;

	console->println(disconnectMsg);
	tellAll(nullptr, disconnectMsg);

	// TODO: despawn player's entity
}


void net::Server::processChat(ENetEvent& evt, Client* client) {
	std::string text((char*)evt.packet->data, evt.packet->dataLength);
	console->println(formatChat(client, text));
	// send the chat out to all clients
	tellAll(client, text);
}


void net::Server::processPlayerCommand(ENetEvent& evt, Client* client) {
	ByteStream bs(evt.packet->data, evt.packet->dataLength);
	PlayerCommand cmd(bs);
	cmd.exec(world);
}


void net::Server::processReceive(ENetEvent& evt) {
	auto client = static_cast<Client*>(evt.peer->data);
	switch (evt.channelID) {
	case CHAN_CHAT:
		processChat(evt, client);
		break;
	case CHAN_PLAYERCMD:
		processPlayerCommand(evt, client);
		break;
	default:
		console->println("Got unexpected packet from " + to_string(evt.peer->address) + " channel " + std::to_string(evt.channelID) + " size " + std::to_string(evt.packet->dataLength));
		break;
	}
}


void net::Server::processNetEvent(ENetEvent& evt) {
	switch (evt.type) {
	case ENET_EVENT_TYPE_CONNECT:
		processConnect(evt);
		break;
	case ENET_EVENT_TYPE_DISCONNECT:
		processDisconnect(evt);
		break;
	case ENET_EVENT_TYPE_RECEIVE:
		processReceive(evt);
		enet_packet_destroy(evt.packet);
		break;
	case ENET_EVENT_TYPE_NONE:
		// make the compiler shut up about unhandled enum value
		break;
	}
}


void net::Server::tick(timediff dt) {
	ENetEvent evt;
	if (enet_host_service(server, &evt, 0) > 0) {
		processNetEvent(evt);
	}

	world->update(dt);

	// send chunk and entity updates to all clients
	for (size_t i = 0; i < maxClients; i++) {
		auto client = &clients[i];
		if (client->peer) {
			ZYXCoordIter<ChunkCoord> iter({ 0, 0, 0 }, { world->chunksX(), world->chunksY(), world->chunksZ() });
			for (const auto& wcc : iter) {
				sendChunkUpdate(client, wcc, true);
			}
			for (const auto& ent : world->entityManager.getEntities()) {
				sendEntityUpdate(client, ent);
			}

			while (deletedEntities.size()) {
				auto id = deletedEntities.front();
				deletedEntities.pop_front();
				sendDeletedEntityUpdate(client, id);
			}
		}
	}
	markChunksClean();
}
