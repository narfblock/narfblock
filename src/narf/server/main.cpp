#include "narf/version.h"
#include "narf/bytestream.h"
#include "narf/cursesconsole.h"
#include "narf/playercmd.h"
#include "narf/world.h"
#include "narf/cmd/cmd.h"
#include "narf/util/path.h"
#include "narf/net/net.h"

#include <enet/enet.h>

#include <string.h>


narf::World *world;

// TODO: move these
#define WORLD_X_MAX 64
#define WORLD_Y_MAX 64
#define WORLD_Z_MAX 64

// TODO: this is copied from client
narf::BlockType genNormalBlockType() {
	narf::BlockType bt;
	bt.solid = true;
	bt.indestructible = false;
	return bt;
}

// TODO: this is copied from client
void genWorld() {
	world = new narf::World(WORLD_X_MAX, WORLD_Y_MAX, WORLD_Z_MAX, 16, 16, 16);

	// set up block types
	// TODO: put this in a config file
	auto airType = genNormalBlockType(); // TODO
	airType.solid = false;
	world->addBlockType(airType); // air

	auto adminiumType = genNormalBlockType();
	adminiumType.indestructible = true;
	world->addBlockType(adminiumType); // adminium

	world->addBlockType(genNormalBlockType()); // dirt
	world->addBlockType(genNormalBlockType()); // dirt with grass top
	world->addBlockType(genNormalBlockType()); // TODO
	world->addBlockType(genNormalBlockType()); // brick
	world->addBlockType(genNormalBlockType()); // stone1
	auto stone2 = world->addBlockType(genNormalBlockType()); // stone2
	world->addBlockType(genNormalBlockType()); // stone3

	world->set_gravity(-24.0f);
}


// TODO: put this somewhere else
class Client {
public:

	Client() : peer(nullptr) {
	}

	ENetPeer* peer;

};

size_t maxClients;
Client* clients = nullptr;


bool quitServerLoop = false;


void cmdQuit(const std::string &args) {
	narf::console->println("Quitting in response to user command");
	quitServerLoop = true;
}


void tell(const Client* to, const Client* from, const std::string& text) {
	// TODO: use client nickname here
	std::string fromName = from ? narf::net::to_string(from->peer->address) : "server";
	std::string packetData("<" + fromName + "> " + text);
	auto packet = enet_packet_create(packetData.c_str(), packetData.length(), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(to->peer, narf::net::CHAN_CHAT, packet);
}


void tellAll(const Client* from, const std::string& text) {
	for (size_t i = 0; i < maxClients; i++) {
		auto client = &clients[i];
		if (client->peer) {
			tell(client, from, text);
		}
	}
}


void sendChunkUpdate(const Client* to, const narf::World::ChunkCoord& wcc, bool dirtyOnly) {
	auto chunk = world->get_chunk(wcc);
	if (!dirtyOnly || chunk->dirty()) {
		narf::ByteStreamWriter bs;
		world->serializeChunk(bs, wcc);
		auto packet = enet_packet_create(bs.data(), bs.size(), ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(to->peer, narf::net::CHAN_CHUNK, packet);
	}
}


void markChunksClean() {
	narf::math::coord::ZYXCoordIter<narf::World::ChunkCoord> iter({ 0, 0, 0 }, { world->chunks_x(), world->chunks_y(), world->chunks_z() });
	for (const auto& wcc : iter) {
		world->get_chunk(wcc)->markClean();
	}
}


ENetHost* serverInit(size_t maxClients) {
	::maxClients = maxClients;
	clients = new Client[maxClients];
	if (!clients) {
		return nullptr;
	}

	ENetAddress bindAddress;
	bindAddress.host = ENET_HOST_ANY;
	bindAddress.port = narf::net::DEFAULT_PORT;

	narf::console->println("Binding to " + narf::net::to_string(bindAddress));
	ENetHost* server = enet_host_create(&bindAddress, maxClients, narf::net::MAX_CHANNELS, 0, 0);
	if (server == nullptr) {
		narf::console->println("Could not bind to " + narf::net::to_string(bindAddress));
	}
	return server;
}


void processConnect(ENetEvent& evt) {
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
	narf::math::coord::ZYXCoordIter<narf::World::ChunkCoord> iter({ 0, 0, 0 }, { world->chunks_x(), world->chunks_y(), world->chunks_z() });
	for (const auto& wcc : iter) {
		sendChunkUpdate(client, wcc, false);
	}
}


void processDisconnect(ENetEvent& evt) {
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


void processChat(ENetEvent& evt, Client* client) {
	std::string text((char*)evt.packet->data, evt.packet->dataLength);
	narf::console->println("Chat: " + text);
	// send the chat out to all clients
	tellAll(client, text);
}


void processPlayerCommand(ENetEvent& evt, Client* client) {
	narf::ByteStreamReader bs(evt.packet->data, evt.packet->dataLength);
	narf::PlayerCommand cmd(bs);
	cmd.exec(world);
}


void processReceive(ENetEvent& evt) {
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


void processNetEvent(ENetEvent& evt) {
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


void tick(double t, double dt) {
	world->update(t, dt);
}


void serverLoop(ENetHost* server) {
	// TODO: make these global config vars like client
	double physicsRate = 60.0;
	double physicsTickStep = 1.0 / physicsRate; // fixed time step

	while (!quitServerLoop) {
		// check for console input
		auto input = narf::console->pollInput();
		if (input != "") {
			if (input[0] == '/') { // commands begin with slash
				narf::cmd::exec(input.substr(1));
			}
		}
		ENetEvent evt;
		if (enet_host_service(server, &evt, 0) > 0) {
			processNetEvent(evt);
		}

		tick(0.0, physicsTickStep); // TODO!!!!

		// send chunk updates to all clients
		// TODO: only do this on ticks
		for (size_t i = 0; i < maxClients; i++) {
			auto client = &clients[i];
			if (client->peer) {
				narf::math::coord::ZYXCoordIter<narf::World::ChunkCoord> iter({ 0, 0, 0 }, { world->chunks_x(), world->chunks_y(), world->chunks_z() });
				for (const auto& wcc : iter) {
					sendChunkUpdate(client, wcc, true);
				}
			}
		}
		markChunksClean();
	}
}


int main(int argc, char **argv)
{
	narf::console = new narf::CursesConsole();

	narf::console->println("Hello, world - I'm a server.");
	narf::console->println("Version: " + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR) + std::string(VERSION_RELEASE) + "+" VERSION_REV);

	if (enet_initialize() != 0) {
		narf::console->println("Error initializing ENet");
		return 1;
	}

	narf::console->println("Executable filename:  " + narf::util::exeName());
	narf::console->println("Executable directory: " + narf::util::exeDir());
	narf::console->println("Data directory: " + narf::util::dataDir());
	narf::console->println("Type /quit to quit.");

	narf::cmd::cmds["quit"] = cmdQuit;

	genWorld();

	auto server = serverInit(32);
	if (server) {
		serverLoop(server);
	}

	delete narf::console;
	if (server) {
		enet_host_destroy(server);
	}
	enet_deinitialize();
	if (clients) {
		delete[] clients;
	}
	return 0;
}
