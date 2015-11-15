#include "narf/version.h"
#include "narf/cursesconsole.h"
#include "narf/embed.h"
#include "narf/gameloop.h"
#include "narf/cmd/cmd.h"
#include "narf/util/paths.h"
#include "narf/tokenize.h"
#include "narf/net.h"
#include "narf/net/protocol.h"
#include "narf/net/server.h"

#include <string.h>

using namespace narf;

narf::CursesConsole *cursesConsole;

class ServerGameLoop : public narf::GameLoop {
public:
	ServerGameLoop(double maxFrameTime, double tickRate, size_t maxClients);
	~ServerGameLoop();

	void getInput() override;
	void tick(narf::timediff dt) override;
	void updateStatus(const std::string& status) override;
	void draw(float stateBlend) override;

	net::Server server;
};

ServerGameLoop* game;


void cmdQuit(const std::string &args) {
	narf::console->println("Quitting in response to user command");
	game->quit = true;
}


void cmdStats(const std::string& args) {
	game->dumpTickTimeHistogram();
}


DECLARE_EMBED(extra_credits_txt);

void cmdAbout(const std::string& args) {
	narf::console->println("");
	narf::console->println("About NarfBlock");
	narf::console->println("Version: " VERSION_STR);
	narf::console->println("");
	narf::console->println("Authors:");
	narf::console->println(VERSION_AUTHORS);
	narf::console->println("");
	auto credits = narf::util::tokenize(EMBED_STRING(extra_credits_txt), '\n');
	for (auto& c : credits) {
		narf::console->println(c);
	}
	narf::console->println("");
	narf::console->println("Library versions:");
	narf::console->println("ENet " + std::to_string(ENET_VERSION_MAJOR) + "." + std::to_string(ENET_VERSION_MINOR) + "." + std::to_string(ENET_VERSION_PATCH));
	narf::console->println(cursesConsole->version()); // version() result includes name of library
}


ServerGameLoop::ServerGameLoop(double maxFrameTime, double tickRate, size_t maxClients) :
	narf::GameLoop(maxFrameTime, tickRate),
	server(maxClients, net::DEFAULT_PORT) {
}


ServerGameLoop::~ServerGameLoop() {
}


void ServerGameLoop::getInput() {
	// check for console input
	auto input = narf::console->pollInput();
	if (input != "") {
		if (input[0] == '/') { // commands begin with slash
			narf::cmd::exec(input.substr(1));
		} else {
			narf::console->println(server.formatChat(nullptr, input));
			server.tellAll(nullptr, input);
		}
	}
}


void ServerGameLoop::tick(narf::timediff dt) {
	server.tick(dt);
}


void ServerGameLoop::updateStatus(const std::string& status) {
	cursesConsole->setStatus(status + " " + std::to_string(server.world->entityManager.getNumEntities()) + " entities");
}


void ServerGameLoop::draw(float stateBlend) {
}


int main(int argc, char **argv)
{
	cursesConsole = new narf::CursesConsole();
	narf::console = cursesConsole;

	narf::console->println("Hello, world - I'm a server.");
	narf::console->println("Version: " VERSION_STR);
	narf::console->println("Executable filename:  " + narf::util::exeName());
	narf::console->println("Executable directory: " + narf::util::exeDir());
	narf::console->println("Data directory: " + narf::util::dataDir());
	narf::console->println("Type /quit to quit.");

	narf::cmd::cmds["quit"] = cmdQuit;
	narf::cmd::cmds["stats"] = cmdStats;
	narf::cmd::cmds["about"] = cmdAbout;

	// TODO: make tick rate and max clients configurable
	game = new ServerGameLoop(0.25, 60.0, 32);
	game->callDraw = false;

	game->run();
	delete game;
	delete narf::console;
	return 0;
}
