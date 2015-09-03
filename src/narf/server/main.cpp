#include "narf/version.h"
#include "narf/bytestream.h"
#include "narf/chunkcache.h"
#include "narf/cursesconsole.h"
#include "narf/gameloop.h"
#include "narf/playercmd.h"
#include "narf/world.h"
#include "narf/cmd/cmd.h"
#include "narf/util/path.h"
#include "narf/net/net.h"

#include <chrono>

#include <enet/enet.h>

#include <string.h>

narf::CursesConsole *cursesConsole;

// TODO: put this somewhere else
class Client {
public:

	Client() : peer(nullptr) {
	}

	ENetPeer* peer;
};


class ServerGameLoop : public narf::GameLoop {
public:
	ServerGameLoop(double maxFrameTime, double tickRate, size_t maxClients);
	~ServerGameLoop();

	void getInput() override;
	void tick(narf::timediff dt) override;
	void updateStatus(const std::string& status) override;
	void draw(float stateBlend) override;

	void processConnect(ENetEvent& evt);
	void processDisconnect(ENetEvent& evt);
	void processChat(ENetEvent& evt, Client* client);
	void processPlayerCommand(ENetEvent& evt, Client* client);
	void processReceive(ENetEvent& evt);
	void processNetEvent(ENetEvent& evt);

	ENetHost* server;
	size_t maxClients;
	Client* clients;
};

ServerGameLoop* game;

narf::World *world;

// TODO: move these
#define WORLD_X_MAX 64
#define WORLD_Y_MAX 64
#define WORLD_Z_MAX 64


// server chunk state
// TODO: track which clients have received which chunks
class ChunkState {
public:
	ChunkState() : dirty(true) {}
	~ChunkState() {}

	bool dirty;
};

narf::ChunkCache<narf::ChunkCoord, ChunkState> chunkCache;


void chunkUpdate(const narf::ChunkCoord& cc) {
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


void blockUpdate(const narf::BlockCoord& wbc) {
	narf::ChunkCoord cc;
	narf::Chunk::BlockCoord cbc;
	world->calcChunkCoords(wbc, cc, cbc);
	chunkUpdate(cc);
}


// TODO: this is copied from client
void genWorld() {
	world = new narf::World(WORLD_X_MAX, WORLD_Y_MAX, WORLD_Z_MAX, 16, 16, 16);
	world->setGravity(-24.0f);

	world->chunkUpdate = chunkUpdate;
	world->blockUpdate = blockUpdate;

	// add test entity
	auto bouncyBlockEID = world->entityManager.newEntity();
	{
		narf::EntityRef bouncyBlock(world->entityManager, bouncyBlockEID);
		bouncyBlock->position = narf::Vector3f(10.0f, 10.0f, 21.0f);
		bouncyBlock->prevPosition = bouncyBlock->position;
		bouncyBlock->bouncy = true;
		bouncyBlock->model = true;
	}
}



void cmdQuit(const std::string &args) {
	narf::console->println("Quitting in response to user command");
	game->quit = true;
}


void cmdStats(const std::string& args) {
	game->dumpTickTimeHistogram();
}


void cmdAbout(const std::string& args) {
	narf::console->println("");
	narf::console->println("About NarfBlock");
	narf::console->println("Version: " VERSION_STR);
	narf::console->println("");
	narf::console->println("Authors:");
	narf::console->println(VERSION_AUTHORS);
	narf::console->println("");
	narf::console->println("Library versions:");
	narf::console->println("ENet " + std::to_string(ENET_VERSION_MAJOR) + "." + std::to_string(ENET_VERSION_MINOR) + "." + std::to_string(ENET_VERSION_PATCH));
	narf::console->println(cursesConsole->version()); // version() result includes name of library
}


std::string formatChat(const Client* from, const std::string& text) {
	// TODO: use client nickname here
	std::string fromName = from ? narf::net::to_string(from->peer->address) : "server";
	return "<" + fromName + "> " + text;
}


void tell(const Client* to, const Client* from, const std::string& text) {
	std::string packetData(formatChat(from, text));
	auto packet = enet_packet_create(packetData.c_str(), packetData.length(), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(to->peer, narf::net::CHAN_CHAT, packet);
}


void tellAll(const Client* from, const std::string& text) {
	for (size_t i = 0; i < game->maxClients; i++) {
		auto client = &game->clients[i];
		if (client->peer) {
			tell(client, from, text);
		}
	}
}


bool clientChunkDirty(const Client* client, const narf::ChunkCoord& cc) {
	auto chunkState = chunkCache.get(cc);
	if (chunkState) {
		return chunkState->dirty;
	}
	return false;
}


void markClientChunkClean(const Client* client, const narf::ChunkCoord& cc) {
	auto chunkState = chunkCache.get(cc);
	if (chunkState && chunkState->dirty) {
		chunkState->dirty = false;
	}
}


void sendChunkUpdate(const Client* to, const narf::ChunkCoord& wcc, bool dirtyOnly) {
	auto chunk = world->getChunk(wcc);
	if (!dirtyOnly || clientChunkDirty(to, wcc)) {
		narf::ByteStreamWriter bs;
		world->serializeChunk(bs, wcc);
		auto packet = enet_packet_create(bs.data(), bs.size(), ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(to->peer, narf::net::CHAN_CHUNK, packet);
	}
}


void markChunksClean() {
	narf::ZYXCoordIter<narf::ChunkCoord> iter({ 0, 0, 0 }, { world->chunksX(), world->chunksY(), world->chunksZ() });
	for (const auto& wcc : iter) {
		for (size_t i = 0; i < game->maxClients; i++) {
			auto client = &game->clients[i];
			if (client->peer) {
				markClientChunkClean(client, wcc);
			}
		}
	}
}

void sendEntityUpdate(const Client* to, const narf::Entity& ent) {
	narf::ByteStreamWriter bs;
	ent.serialize(bs);
	auto packet = enet_packet_create(bs.data(), bs.size(), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(to->peer, narf::net::CHAN_ENTITY, packet);
}

ServerGameLoop::ServerGameLoop(double maxFrameTime, double tickRate, size_t maxClients) :
	narf::GameLoop(maxFrameTime, tickRate),
	maxClients(maxClients) {

	clients = new Client[maxClients];
	if (!clients) {
		return;
	}

	ENetAddress bindAddress;
	bindAddress.host = ENET_HOST_ANY;
	bindAddress.port = narf::net::DEFAULT_PORT;

	narf::console->println("Binding to " + narf::net::to_string(bindAddress));
	server = enet_host_create(&bindAddress, maxClients, narf::net::MAX_CHANNELS, 0, 0);
	if (server == nullptr) {
		narf::console->println("Could not bind to " + narf::net::to_string(bindAddress));
	}
}


ServerGameLoop::~ServerGameLoop() {
	if (server) {
		enet_host_destroy(server);
	}
	enet_deinitialize();
	if (clients) {
		delete[] clients;
	}
}


void ServerGameLoop::processConnect(ENetEvent& evt) {
	narf::console->println("Client connected from " + narf::net::to_string(evt.peer->address));
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
		narf::console->println("ERROR: No client slots available");
		enet_peer_reset(evt.peer);
		return;
	}

	client->peer = evt.peer;

	// store Client pointer in peer data
	evt.peer->data = client;

	// send a server message greeting
	tellAll(nullptr, "Client connected from " + narf::net::to_string(evt.peer->address));

	// send all chunks (!!)
	narf::ZYXCoordIter<narf::ChunkCoord> iter({ 0, 0, 0 }, { world->chunksX(), world->chunksY(), world->chunksZ() });
	for (const auto& wcc : iter) {
		sendChunkUpdate(client, wcc, false);
	}
}


void ServerGameLoop::processDisconnect(ENetEvent& evt) {
	auto client = static_cast<Client*>(evt.peer->data);
	auto disconnectType = static_cast<narf::net::DisconnectType>(evt.data);
	std::string reason;
	switch (disconnectType) {
	case narf::net::DisconnectType::Timeout:
		reason = "timed out";
		break;
	case narf::net::DisconnectType::UserQuit:
		reason = "user quit";
		break;
	default:
		reason = "unknown";
		break;
	}

	std::string disconnectMsg = "Client disconnected from " + narf::net::to_string(evt.peer->address) + " (" + reason + ")";
	client->peer = nullptr;
	evt.peer->data = nullptr;

	narf::console->println(disconnectMsg);
	tellAll(nullptr, disconnectMsg);
}


void ServerGameLoop::processChat(ENetEvent& evt, Client* client) {
	std::string text((char*)evt.packet->data, evt.packet->dataLength);
	narf::console->println(formatChat(client, text));
	// send the chat out to all clients
	tellAll(client, text);
}


void ServerGameLoop::processPlayerCommand(ENetEvent& evt, Client* client) {
	narf::ByteStreamReader bs(evt.packet->data, evt.packet->dataLength);
	narf::PlayerCommand cmd(bs);
	cmd.exec(world);
}


void ServerGameLoop::processReceive(ENetEvent& evt) {
	auto client = static_cast<Client*>(evt.peer->data);
	switch (evt.channelID) {
	case narf::net::CHAN_CHAT:
		processChat(evt, client);
		break;
	case narf::net::CHAN_PLAYERCMD:
		processPlayerCommand(evt, client);
		break;
	default:
		narf::console->println("Got unexpected packet from " + narf::net::to_string(evt.peer->address) + " channel " + std::to_string(evt.channelID) + " size " + std::to_string(evt.packet->dataLength));
		break;
	}
}


void ServerGameLoop::processNetEvent(ENetEvent& evt) {
	switch (evt.type) {
	case ENET_EVENT_TYPE_CONNECT:
		processConnect(evt);
		break;
	case ENET_EVENT_TYPE_DISCONNECT:
		processDisconnect(evt);
		break;
	case ENET_EVENT_TYPE_RECEIVE:
		processReceive(evt);
		break;
	case ENET_EVENT_TYPE_NONE:
		// make the compiler shut up about unhandled enum value
		break;
	}
}


void ServerGameLoop::getInput() {
	// check for console input
	auto input = narf::console->pollInput();
	if (input != "") {
		if (input[0] == '/') { // commands begin with slash
			narf::cmd::exec(input.substr(1));
		} else {
			narf::console->println(formatChat(nullptr, input));
			tellAll(nullptr, input);
		}
	}
}


void ServerGameLoop::tick(narf::timediff dt) {
	ENetEvent evt;
	if (enet_host_service(server, &evt, 0) > 0) {
		processNetEvent(evt);
	}

	world->update(dt);

	// send chunk and entity updates to all clients
	for (size_t i = 0; i < maxClients; i++) {
		auto client = &clients[i];
		if (client->peer) {
			narf::ZYXCoordIter<narf::ChunkCoord> iter({ 0, 0, 0 }, { world->chunksX(), world->chunksY(), world->chunksZ() });
			for (const auto& wcc : iter) {
				sendChunkUpdate(client, wcc, true);
			}
			for (const auto& ent : world->entityManager.getEntities()) {
				sendEntityUpdate(client, ent);
			}
		}
	}
	markChunksClean();
}


void ServerGameLoop::updateStatus(const std::string& status) {
	cursesConsole->setStatus(status + " " + std::to_string(world->entityManager.getNumEntities()) + " entities");
}


void ServerGameLoop::draw(float stateBlend) {
}


int main(int argc, char **argv)
{
	cursesConsole = new narf::CursesConsole();
	narf::console = cursesConsole;

	narf::console->println("Hello, world - I'm a server.");
	narf::console->println("Version: " VERSION_STR);

	if (enet_initialize() != 0) {
		narf::console->println("Error initializing ENet");
		return 1;
	}

	narf::console->println("Executable filename:  " + narf::util::exeName());
	narf::console->println("Executable directory: " + narf::util::exeDir());
	narf::console->println("Data directory: " + narf::util::dataDir());
	narf::console->println("Type /quit to quit.");

	narf::cmd::cmds["quit"] = cmdQuit;
	narf::cmd::cmds["stats"] = cmdStats;
	narf::cmd::cmds["about"] = cmdAbout;

	genWorld();

	// TODO: make tick rate and max clients configurable
	game = new ServerGameLoop(0.25, 60.0, 32);
	game->callDraw = false;
	game->run();
	delete game;
	delete narf::console;
	return 0;
}
