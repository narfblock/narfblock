#include <cmath>

#include <SDL.h>

#if SDL_MAJOR_VERSION < 2
#error SDL2 required
#endif

#include <enet/enet.h>

#include <zlib.h>
#include <png.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <queue>

#include "narf/version.h"

#include "narf/camera.h"
#include "narf/color.h"
#include "narf/embed.h"
#include "narf/entity.h"
#include "narf/file.h"
#include "narf/font.h"
#include "narf/gameloop.h"
#include "narf/ini.h"
#include "narf/input.h"
#include "narf/block.h"
#include "narf/playercmd.h"
#include "narf/time.h"
#include "narf/cmd/cmd.h"
#include "narf/math/math.h"
#include "narf/net/net.h"
#include "narf/util/path.h"
#include "narf/util/tokenize.h"

#include "narf/client/console.h"
#include "narf/client/renderer.h"

#include "narf/gl/gl.h"

// TODO: this is all hacky test code - refactor into nicely modularized code

void newWorld();

narf::ClientConsole *clientConsole;

narf::Entity::ID playerEID;
narf::Entity::ID bouncyBlockEID; // temp hack

narf::INI::File config;

narf::Camera cam;

const float movespeed = 25.0f;
const float runspeed = 500.0f;

narf::World* world = nullptr;
narf::Renderer* renderer = nullptr;

#define WORLD_X_MAX 1024
#define WORLD_Y_MAX 1024
#define WORLD_Z_MAX 128

narf::gl::Context *display;
narf::gl::Texture *tilesTex;

narf::font::FontManager fontManager;
narf::font::TextBuffer *fpsTextBuffer;
narf::font::TextBuffer *blockInfoBuffer;
narf::font::TextBuffer *entityInfoBuffer;
narf::font::TextBuffer *locationBuffer;

narf::BlockWrapper selectedBlockFace;

ENetHost* client = nullptr;
ENetPeer* server = nullptr;

bool paused = false;

// stereoscopic rendering
bool stereoEnabled;
bool stereoCross;
float stereoSeparation;

enum class ConnectState {
	Unconnected,
	Connecting,
	Connected,
};

ConnectState connectState = ConnectState::Unconnected;
narf::time connectTimeoutEnd;

const narf::timediff connectTimeout(5.0);

std::queue<narf::PlayerCommand> playerCommandQueue;

// debug options
bool screenshot = false;


class ClientGameLoop : public narf::GameLoop {
public:
	ClientGameLoop(double maxFrameTime, double tickRate);

	void getInput() override;
	void tick(narf::timediff dt) override;
	void updateStatus(const std::string& status) override;
	void draw(float stateBlend) override;

	float inputDivider;
	narf::Input input;
};

ClientGameLoop* gameLoop;


narf::font::Font* setFont(
	const std::string& useName, // e.g. "Console" or "HUD"
	const std::string& nameVar, const std::string& nameDefault,
	const std::string& sizeVar, uint32_t sizeDefault) {
	auto fontName = config.getString(nameVar, nameDefault);
	auto fontSize = config.getUInt32(sizeVar, sizeDefault);

	narf::console->println("Setting " + useName + " font to " + fontName + " " + std::to_string(fontSize) + "px");

	auto font = fontManager.getFont(fontName, fontSize);
	if (!font) {
		narf::console->println("Error: could not load font " + fontName);
	}

	return font;
}


void configEvent(const std::string& key) {
	// TODO: this could be done better...
	narf::console->println("onPropertyChanged(" + key + ")");
	if (key == "video.renderDistance") {
		renderer->setRenderDistance(config.getInt32(key));
		narf::console->println("Setting renderDistance to " + std::to_string(renderer->getRenderDistance()));
	} else if (key == "video.consoleCursorShape") {
		auto shapeStr = config.getString(key);
		clientConsole->setCursorShape(narf::ClientConsole::cursorShapeFromString(shapeStr));
	} else if (key == "video.vsync") {
		auto vsync = config.getBool(key);
		display->setVsync(vsync);
		narf::console->println("Setting vsync to " + std::to_string(vsync));
	} else if (key == "foo.gravity") {
		world->setGravity(config.getFloat(key));
		narf::console->println("Setting gravity to " + std::to_string(world->getGravity()));
	} else if (key == "misc.tickRate") {
		auto tickRate = config.getDouble(key);
		narf::console->println("Setting tickRate to " + std::to_string(tickRate));
		if (gameLoop) {
			gameLoop->setTickRate(tickRate);
		}
	} else if (key == "misc.maxFrameTime") {
		auto maxFrameTime = config.getDouble(key, 0.25);
		narf::console->println("Setting maxFrameTime to " + std::to_string(maxFrameTime));
		if (gameLoop) {
			gameLoop->setMaxFrameTime(maxFrameTime);
		}
	} else if (key == "video.hudFont" ||
		key == "video.hudFontSize") {
		auto font = setFont(
			"HUD",
			"video.hudFont", "DroidSansMono",
			"video.hudFontSize", 30);
		if (font) {
			if (gameLoop) {
				gameLoop->forceStatusUpdate = true;
			}
			fpsTextBuffer->setFont(font);
			blockInfoBuffer->setFont(font);
			entityInfoBuffer->setFont(font);
			locationBuffer->setFont(font);
		}
	} else if (key == "video.consoleFont" ||
		key == "video.consoleFontSize") {
		auto font = setFont(
			"console",
			"video.consoleFont", "DroidSansMono",
			"video.consoleFontSize", 18);
		if (font) {
			clientConsole->setFont(font);
		}
	} else if (key == "video.stereo.enabled") {
		stereoEnabled = config.getBool(key);
	} else if (key == "video.stereo.cross") {
		stereoCross = config.getBool(key);
	} else if (key == "video.stereo.separation") {
		stereoSeparation = config.getFloat(key);
	} else {
		narf::console->println("Config var updated: " + key);
	}
}


float clampf(float val, float min, float max)
{
	if (val < min) val = min;
	if (val > max) val = max;
	return val;
}


bool initVideo(int32_t w, int32_t h, bool fullscreen)
{
	display = new narf::gl::Context();
	if (!display->setDisplayMode("NarfBlock", w, h, fullscreen)) {
		return false;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	return true;
}


DECLARE_EMBED(terrain_png);

bool init_textures()
{
	const std::string terrain_file = config.getString("misc.terrain", "terrain.png");
	auto terrainFilePath = narf::util::appendPath(narf::util::dataDir(), terrain_file);

	const void* tilesData;
	size_t tilesSize;

	narf::MemoryFile tilesFile;
	if (tilesFile.read(terrainFilePath)) {
		tilesData = tilesFile.data;
		tilesSize = tilesFile.size;
	} else {
		narf::console->println("read of file " + terrainFilePath + " failed; falling back to embedded texture");
		tilesData = EMBED_DATA(terrain_png);
		tilesSize = EMBED_SIZE(terrain_png);
	}
	auto tilesImage = narf::loadPNG(tilesData, tilesSize);

	if (!tilesImage) {
		narf::console->println("loading terrain texture failed");
		SDL_Quit();
		return false;
	}

	tilesTex = new narf::gl::Texture(display);
	if (!tilesTex->upload(tilesImage)) {
		delete tilesImage;
		assert(0);
		return false;
	}

	delete tilesImage;

	return true;
}


void drawHighlightQuad(const float *quad)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	// TODO: get this stuff working so depth test can be enabled
	//glEnable(GL_POLYGON_OFFSET_LINE);
	//glPolygonOffset(0.0f, -2.5f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(5.0f);

	glBegin(GL_LINE_LOOP);
	glVertex3fv(&quad[0 * 3]);
	glVertex3fv(&quad[1 * 3]);
	glVertex3fv(&quad[2 * 3]);
	glVertex3fv(&quad[3 * 3]);
	glEnd();

	glPopAttrib();
}

// hax - put somewhere better
void drawCubeHighlight(const narf::BlockWrapper &blockFace)
{
	auto x = static_cast<float>(blockFace.x);
	auto y = static_cast<float>(blockFace.y);
	auto z = static_cast<float>(blockFace.z);
	const float cube_quads[][4*3] = {
		{x+1,y+0,z+0, x+1,y+1,z+0, x+1,y+1,z+1, x+1,y+0,z+1}, // XPos
		{x+0,y+0,z+0, x+0,y+0,z+1, x+0,y+1,z+1, x+0,y+1,z+0}, // XNeg
		{x+1,y+1,z+0, x+0,y+1,z+0, x+0,y+1,z+1, x+1,y+1,z+1}, // YPos
		{x+1,y+0,z+0, x+1,y+0,z+1, x+0,y+0,z+1, x+0,y+0,z+0}, // YNeg
		{x+0,y+0,z+1, x+1,y+0,z+1, x+1,y+1,z+1, x+0,y+1,z+1}, // ZPos
		{x+0,y+1,z+0, x+1,y+1,z+0, x+1,y+0,z+0, x+0,y+0,z+0}, // ZNeg
	};

	assert(blockFace.face < sizeof(cube_quads) / sizeof(*cube_quads));
	drawHighlightQuad(cube_quads[blockFace.face]);
}


void draw3d(float stateBlend, narf::Matrix4x4f translate) {
	renderer->render(*display, cam, stateBlend, translate);

	if (selectedBlockFace.block) {
		// draw a selection rectangle
		drawCubeHighlight(selectedBlockFace);
	}
}


void draw_cursor() {
	// draw a cursor thingy
	float cursor_size = 9.0f; // TODO

	glColor4f(1.0f, 0.5f, 1.0f, 0.7f);
	glBegin(GL_QUADS);
	glVertex2f((float)display->width() / 2.0f - cursor_size / 2.0f, (float)display->height() / 2.0f - cursor_size / 2.0f);
	glVertex2f((float)display->width() / 2.0f + cursor_size / 2.0f, (float)display->height() / 2.0f - cursor_size / 2.0f);
	glVertex2f((float)display->width() / 2.0f + cursor_size / 2.0f, (float)display->height() / 2.0f + cursor_size / 2.0f);
	glVertex2f((float)display->width() / 2.0f - cursor_size / 2.0f, (float)display->height() / 2.0f + cursor_size / 2.0f);
	glEnd();

	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_POINTS);
	glVertex2f((float)display->width() / 2.0f, (float)display->height() / 2.0f);
	glEnd();

	glColor3f(1.0f, 1.0f, 1.0f);
}


void updateConsoleSize() {
	auto consoleX = 0u;
	auto consoleY = 0u;
	auto consoleWidth = display->width();
	auto consoleHeight = 175u; // TODO: calculate dynamically based on screen size
	clientConsole->setLocation(consoleX, consoleY, consoleWidth, consoleHeight);
}


void draw2d() {
	// draw 2d overlays

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, display->width(), 0.0, display->height(), 0.0, 1.0);

	glTranslatef(0.375f, 0.375f, 0.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (stereoEnabled) {
		int height = display->height();
		int width = display->width();
		glViewport(0, height / 4, width / 2, height / 2);
		draw_cursor();
		glViewport(width / 2, height / 4, width / 2, height / 2);
		draw_cursor();
		glViewport(0, 0, width, height);
	} else {
		draw_cursor();
	}

	auto blue = narf::Color(0.0f, 0.0f, 1.0f);

	auto hudFontHeight = blockInfoBuffer->getFont()->height();

	blockInfoBuffer->clear();
	if (selectedBlockFace.block) {
		std::string block_info_str = "Block info: ";

		const char *BlockFace_str[] = {"East", "West", "North", "South", "Top", "Bottom", "Invalid"};

		block_info_str += "ID: " + std::to_string(selectedBlockFace.block->id) +
			" Pos: " + std::to_string(selectedBlockFace.x) +
			", " + std::to_string(selectedBlockFace.y) +
			", " + std::to_string(selectedBlockFace.z) +
			" " + BlockFace_str[selectedBlockFace.face] +
			" (" + std::to_string((int)selectedBlockFace.face) + ")";

		blockInfoBuffer->print(block_info_str, 0, (float)display->height() - hudFontHeight * 4.0f, blue);
	}

	entityInfoBuffer->clear();
	entityInfoBuffer->print("numEntities: " + std::to_string(world->entityManager.getNumEntities()), 0, (float)display->height() - hudFontHeight * 3.0f, blue);

	std::string location_str;
	if (playerEID == narf::Entity::InvalidID) {
		location_str = "Pos: " + std::to_string(cam.position.x) + ", " + std::to_string(cam.position.y) + ", " + std::to_string(cam.position.z);
	} else {
		narf::EntityRef player(world->entityManager, playerEID);
		location_str = "Pos: " + std::to_string(player->position.x) + ", " + std::to_string(player->position.y) + ", " + std::to_string(player->position.z);
	}
	location_str += " Yaw: " + std::to_string(cam.orientation.yaw) + " Pitch: " + std::to_string(cam.orientation.pitch);
	locationBuffer->clear();
	locationBuffer->print(location_str, 0, (float)display->height() - hudFontHeight * 2.0f, blue);

	updateConsoleSize();
	clientConsole->render();
	fpsTextBuffer->render();
	blockInfoBuffer->render();
	entityInfoBuffer->render();
	locationBuffer->render();
}


void draw(float stateBlend) {
	display->updateViewport();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (stereoEnabled) {
		int height = display->height();
		int width = display->width();
		float sep = stereoSeparation;
		if (stereoCross) {
			sep = -sep;
		}
		glViewport(0, height / 4, width / 2, height / 2);
		draw3d(stateBlend, narf::Matrix4x4f::translate(sep, 0, 0));
		glViewport(width / 2, height / 4, width / 2, height / 2);
		draw3d(stateBlend, narf::Matrix4x4f::translate(-sep, 0, 0));
		glViewport(0, 0, width, height);
	} else {
		draw3d(stateBlend, narf::Matrix4x4f::translate(0, 0, 0));
	}

	draw2d();

	display->swap();

	if (screenshot) {
		screenshot = false;
		auto dwidth = static_cast<int>(display->width());
		auto dheight = static_cast<int>(display->height());
		SDL_Surface* image = SDL_CreateRGBSurface(SDL_SWSURFACE, dwidth, dheight, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
		glReadPixels(0, 0, dwidth, dheight, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
		narf::console->println("Taking a screenshot. " + std::to_string(image->w) + "x" + std::to_string(image->h) + "x24");

		// Flip upside down
		auto linesize = image->w * 3;
		auto pixels = static_cast<uint8_t*>(image->pixels);
		auto halfHeight = image->h / 2;
		for (int32_t y = 0; y < halfHeight; y++) {
			auto p1 = pixels + y * linesize;
			auto p2 = pixels + (image->h - y - 1) * linesize;
			std::swap_ranges(p1, p1 + linesize, p2);
		}
		SDL_SaveBMP(image, "pic.bmp");
		SDL_FreeSurface(image);
	}
}


void chat(const std::string& text) {
	if (connectState != ConnectState::Connected) {
		narf::console->println("Not connected");
		return;
	}

	auto packet = enet_packet_create(text.c_str(), text.length(), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(server, narf::net::CHAN_CHAT, packet);
	enet_host_flush(client); // TODO: probably not necessary?
}


void sendPlayerCommand(const narf::PlayerCommand& cmd) {
	narf::ByteStreamWriter bs;
	cmd.serialize(bs);
	auto packet = enet_packet_create(bs.data(), bs.size(), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(server, narf::net::CHAN_PLAYERCMD, packet);
}


void processPlayerCommandQueue(std::queue<narf::PlayerCommand>& q) {
	switch (connectState) {
	case ConnectState::Unconnected:
		while (!q.empty()) {
			q.front().exec(world);
			q.pop();
		}
		break;

	case ConnectState::Connected:
		while (!q.empty()) {
			sendPlayerCommand(q.front());
			q.pop();
		}
		enet_host_flush(client); // TODO: probably not necessary?
		break;

	case ConnectState::Connecting:
		// discard command
		break;
	}
}


void sim_frame(const narf::Input &input, narf::timediff dt)
{
	if (input.text() != "") {
		if (input.text()[0] == '/') { // commands begin with slash
			narf::cmd::exec(input.text().substr(1));
		} else {
			chat(input.text());
		}
	}

	// tell the console whether to draw the cursor
	clientConsole->setEditState(input.state() == narf::Input::InputStateText);

	if (input.state() == narf::Input::InputStateText) {
		SDL_SetRelativeMouseMode(SDL_FALSE);
	} else {
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}

	narf::Vector3f vel_rel(0.0f, 0.0f, 0.0f);

	float movePitch = 0.0f;

	if (playerEID != narf::Entity::InvalidID) {
		narf::EntityRef player(world->entityManager, playerEID);

		if (player->antigrav) {
			// if flying, move in the direction of camera including pitch
			movePitch = cam.orientation.pitch;
		}

		if (input.moveForward()) {
			vel_rel += narf::Orientationf(movePitch, cam.orientation.yaw);
		} else if (input.moveBackward()) {
			vel_rel -= narf::Orientationf(movePitch, cam.orientation.yaw);
		}

		if (input.strafeLeft()) {
			vel_rel += narf::Orientationf(0.0f, cam.orientation.yaw + (float)M_PI / 2);
		} else if (input.strafeRight()) {
			vel_rel -= narf::Orientationf(0.0f, cam.orientation.yaw + (float)M_PI / 2);
		}

		// normalize so that diagonal movement is not faster than cardinal directions
		vel_rel = vel_rel.normalize() * (input.run() ? runspeed : movespeed);

		if (input.jump()) {
			if (player->onGround) {
				vel_rel += narf::Vector3f(0.0f, 0.0f, 8.0f);
			} else if (player->velocity.z > 0.0f) {
				// still going up - double jump triggers flying
				player->antigrav = true;
				narf::console->println("entered antigrav mode");
			}
		}

		// hax
		player->velocity.x = vel_rel.x;
		player->velocity.y = vel_rel.y;
		player->velocity.z += vel_rel.z;

		if (player->antigrav) {
			player->velocity.z = vel_rel.z;
		}
	}

	world->update(dt);

	if (playerEID != narf::Entity::InvalidID) {
		narf::EntityRef player(world->entityManager, playerEID);
		if (player->onGround && player->antigrav) {
			player->antigrav = false;
			narf::console->println("left antigrav mode");
		}

		// lock camera to player
		cam.position = player->position;
		cam.position.z += 1.6f;
	}

	// Let's see what we're looking at
	auto pos = narf::Point3f(cam.position.x, cam.position.y, cam.position.z);
	auto maxInteractDistance = 7.5f;
	selectedBlockFace = {};
	bool traceHitBlock = false;
	world->rayTrace(pos, cam.orientation,
		[&](const narf::Point3f& point, const narf::BlockCoord& blockCoord, const narf::BlockFace& face){
			if (point.distanceTo(pos) >= maxInteractDistance) {
				return true; // ran out of distance
			}

			// TODO: stop the trace if it runs off the edge of the world
			auto block = world->getBlock(blockCoord);
			if (block && block->id != 0) {
				// found a solid block
				selectedBlockFace = {block, blockCoord.x, blockCoord.y, blockCoord.z, face};
				traceHitBlock = true;
				return true; // stop the trace
			}
			return false; // keep going
		});

	if (traceHitBlock) {
		if (input.actionPrimaryBegin() || input.actionSecondaryBegin()) {
			narf::BlockCoord wbc(selectedBlockFace.x, selectedBlockFace.y, selectedBlockFace.z);
			if (input.actionPrimaryBegin()) {
				// remove block at cursor
				narf::PlayerCommand cmd(narf::PlayerCommand::Type::PrimaryAction);
				cmd.wbc = wbc;
				playerCommandQueue.push(cmd);
			} else {
				// add new block next to selected face
				// TODO: move this adjustment to PlayerCommand::exec()
				switch (selectedBlockFace.face) {
				case narf::BlockFace::XPos: wbc.x++; break;
				case narf::BlockFace::XNeg: wbc.x--; break;
				case narf::BlockFace::YPos: wbc.y++; break;
				case narf::BlockFace::YNeg: wbc.y--; break;
				case narf::BlockFace::ZPos: wbc.z++; break;
				case narf::BlockFace::ZNeg: wbc.z--; break;
				case narf::BlockFace::Invalid: assert(0); break;
				}
				narf::PlayerCommand cmd(narf::PlayerCommand::Type::SecondaryAction);
				cmd.wbc = wbc;
				playerCommandQueue.push(cmd);
			}
		}
	}

	if (input.actionTernary()) {
		narf::PlayerCommand cmd(narf::PlayerCommand::Type::TernaryAction);
		cmd.velocity = narf::Vector3f(0.0f, 0.0f, 0.0f);
		if (playerEID != narf::Entity::InvalidID) {
			narf::EntityRef player(world->entityManager, playerEID);
			cmd.velocity = player->velocity;
		}
		cmd.position = cam.position;
		cmd.orientation = cam.orientation;
		playerCommandQueue.push(cmd);
	}

	if (input.toggleWireframe()) {
		renderer->wireframe = !renderer->wireframe;
	}

	if (input.toggleBackfaceCulling()) {
		renderer->backfaceCulling = !renderer->backfaceCulling;
	}

	if (input.toggleFog()) {
		renderer->fog = !renderer->fog;
	}

	if (input.toggleFullscreen()) {
		display->toggleFullscreen();
	}

	if (input.screenshot()) {
		screenshot = true;
	}

	processPlayerCommandQueue(playerCommandQueue);
}


void processConnect(ENetEvent& evt) {
	connectState = ConnectState::Connected;
	narf::console->println("Connected to server " + narf::net::to_string(evt.peer->address));

	// reset the world
	newWorld();

	// TODO: put this somewhere common
	delete renderer;
	renderer = new narf::Renderer(world, *display, tilesTex);
}


void processDisconnect(ENetEvent& evt) {
	narf::console->println("Disconnected from " + narf::net::to_string(evt.peer->address));
	evt.peer->data = nullptr;
	connectState = ConnectState::Unconnected;

	// TODO: delete world and drop back to menu
	newWorld();
}


void processChat(ENetEvent& evt) {
	//narf::console->println("Got packet from " + narf::net::to_string(evt.peer->address) + " channel " + std::to_string(evt.channelID) + " size " + std::to_string(evt.packet->dataLength));
	std::string text((char*)evt.packet->data, evt.packet->dataLength);
	narf::console->println(text);
}

void processChunk(ENetEvent& evt) {
	narf::ByteStreamReader bs(evt.packet->data, evt.packet->dataLength);
	narf::ChunkCoord wcc;
	world->deserializeChunk(bs, wcc);
}

void processReceive(ENetEvent& evt) {
	switch (evt.channelID) {
	case narf::net::CHAN_CHAT:
		processChat(evt);
		break;
	case narf::net::CHAN_CHUNK:
		processChunk(evt);
		break;
	}
}


void pollNet()
{
	if (connectState != ConnectState::Connecting &&
		connectState != ConnectState::Connected) {
		return;
	}

	// wait for the connection to succeed
	ENetEvent evt;
	if (enet_host_service(client, &evt, 0) > 0) {
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

	if (connectState == ConnectState::Connecting &&
		narf::time::now() >= connectTimeoutEnd) {
		narf::console->println("Connection attempt timed out"); // TODO: use to_string

		enet_peer_reset(server);
		// TODO: is server valid at this point? do we need to destroy it?
		server = nullptr;
		connectState = ConnectState::Unconnected;
	}
}


ClientGameLoop::ClientGameLoop(double maxFrameTime, double tickRate) :
	narf::GameLoop(maxFrameTime, tickRate),
	inputDivider(config.getFloat("misc.inputDivider", 1000)),
	input(clientConsole->getTextEditor(), 1.0f / inputDivider, 1.0f / inputDivider) {
}


void ClientGameLoop::getInput() {
	SDL_Event e;

	while (SDL_PollEvent(&e)) {
		input.processEvent(&e);
	}

	if (input.exit()) {
		quit = true;
		return;
	}

	// update camera in getInput() for smooth camera motion even if tickRate is low
	// TODO: decouple player direction and camera direction
	cam.orientation.yaw -= input.lookRel().x;
	cam.orientation.yaw = fmodf(cam.orientation.yaw, (float)M_PI * 2.0f);

	cam.orientation.pitch.minimum = -(float)M_PI;
	cam.orientation.pitch.maximum = (float)M_PI;
	cam.orientation.pitch -= input.lookRel().y;
	cam.orientation.pitch = clampf(cam.orientation.pitch, -(float)M_PI / 2, (float)M_PI / 2);

	input.resetLookRel();

	if (input.togglePause()) {
		paused = !paused;
		if (paused) {
			narf::console->println("Paused");
		} else {
			narf::console->println("Unpaused");
		}
		input.reset();
	}

	pollNet();
}

void ClientGameLoop::tick(narf::timediff dt) {
	if (!paused) {
		// TODO: merge the common sim_frame stuff into GameLoop
		sim_frame(input, dt);
		input.reset();
	}
}


void ClientGameLoop::updateStatus(const std::string& status) {
	auto hudFontHeight = fpsTextBuffer->getFont()->height();
	auto blue = narf::Color(0.0f, 0.0f, 1.0f);
	fpsTextBuffer->clear();
	fpsTextBuffer->print(status, 0.0f, (float)display->height() - hudFontHeight, blue);
}


void ClientGameLoop::draw(float stateBlend) {
	::draw(stateBlend);
}


void newWorld()
{
	if (world) {
		delete world;
	}

	playerEID = narf::Entity::InvalidID;
	bouncyBlockEID = narf::Entity::InvalidID;

	world = new narf::World(WORLD_X_MAX, WORLD_Y_MAX, WORLD_Z_MAX, 16, 16, 16);

	world->chunkUpdate = [](const narf::ChunkCoord& cc) {
		if (renderer) {
			renderer->chunkUpdate(cc);
		}
	};

	world->blockUpdate = [](const narf::BlockCoord& wbc) {
		if (renderer) {
			renderer->blockUpdate(wbc);
		}
	};
	world->setGravity(-24.0f);
}


void cmdSet(const std::string &args) {
	auto tokens = narf::util::tokenize(args, ' ');
	if (tokens.size() == 1) {
		auto key(tokens[0]);
		auto value(config.getString(key, ""));
		narf::console->println(key + " = " + value);
	} else if (tokens.size() == 2) {
		std::string key(tokens[0]);
		std::string value(tokens[1]);
		narf::console->println("Setting '" + key + "' to '" + value + "'");
		config.setString(key, value);
	} else {
		narf::console->println("wrong number of parameters to set");
	}
}

void cmdQuit(const std::string &args) {
	narf::console->println("Quitting in response to user command");
	gameLoop->quit = true;
}

void cmdConnect(const std::string& args) {
	if (connectState != ConnectState::Unconnected) {
		narf::console->println("already connected");
		// TODO: disconnect?
		return;
	}

	auto addrStr = args; // TODO: parse extra parameters?

	std::string host;
	uint16_t port;
	if (!narf::net::splitHostPort(addrStr, host, port)) {
		narf::console->println("connect: could not parse address '" + addrStr + "'");
		return;
	}

	if (port == 0) {
		port = narf::net::DEFAULT_PORT;
	}

	narf::console->println("Connecting to " + host + ":" + std::to_string(port) + "...");

	ENetAddress addr;
	// TODO: this does a blocking DNS lookup
	if (enet_address_set_host(&addr, host.c_str()) < 0) {
		narf::console->println("Name lookup failed for '" + host + "'");
		return;
	}
	addr.port = port;

	server = enet_host_connect(client, &addr, narf::net::MAX_CHANNELS, 0);
	if (!server) {
		narf::console->println("connect: enet_host_connect failed");
		// TODO: destroy client
		return;
	}

	narf::console->println("Connecting to " + narf::net::to_string(addr) + "...");
	connectState = ConnectState::Connecting;
	connectTimeoutEnd = narf::time::now() + connectTimeout;
}


void cmdDisconnect(const std::string& args) {
	if (connectState != ConnectState::Unconnected){
		narf::console->println("Disconnecting...");
		enet_peer_disconnect(server, (uint32_t)narf::net::DisconnectType::UserQuit);
		enet_host_flush(client);
	} else {
		narf::console->println("Not connected");
	}
}


void cmdSave(const std::string& args) {
	narf::console->println("Serializing world...");
	narf::ByteStreamWriter s;
	world->serialize(s);
	narf::console->println("Saving world to " + args + "...");
	FILE* f = fopen(args.c_str(), "wb");
	auto written = fwrite(s.data(), 1, s.size(), f);
	if (written != s.size()) {
		narf::console->println("Error saving (incomplete write)");
	}
	fclose(f);
	narf::console->println("Save complete");
}


void cmdLoad(const std::string& args) {
	narf::console->println("Loading world from " + args + "...");
	FILE* f = fopen(args.c_str(), "rb");
	if (f == nullptr) {
		narf::console->println("Could not open file");
		return;
	}

	fseek(f, 0, SEEK_END);
	// TODO: use ftello?
	auto size = static_cast<size_t>(ftell(f));
	fseek(f, 0, SEEK_SET);
	narf::ByteStreamReader s(size);
	if (s.size() != size) {
		narf::console->println("Error reserving space for data");
		fclose(f);
		return;
	}
	auto numRead = fread(s.data(), 1, s.size(), f);
	if (numRead != s.size()) {
		narf::console->println("Error loading (incomplete read)");
	}
	fclose(f);
	narf::console->println("Deserializing...");
	world->deserialize(s);
	narf::console->println("Load complete");
}


void cmdSaveConfig(const std::string& args) {
	auto filename = args;
	if (args.length() == 0) {
		filename = narf::util::appendPath(narf::util::userConfigDir("narfblock"), "client.ini");
		// TODO: recursively mkdir path
	}
	narf::console->println("Saving configuration to " + filename);
	FILE* f = fopen(filename.c_str(), "wb");
	if (!f) {
		narf::console->println("Couldn't open configuration file for writing");
		return;
	}

	auto configstr = config.save();
	auto written = fwrite(configstr.c_str(), 1, configstr.length(), f);
	if (written != configstr.length()) {
		narf::console->println("Error saving configuration (incomplete write)");
	}
	fclose(f);
}


void cmdStats(const std::string& args) {
	gameLoop->dumpTickTimeHistogram();
}


void cmdAbout(const std::string& args) {
	// TODO: replace this with a GUI window
	narf::console->println("");
	narf::console->println("About NarfBlock");
	narf::console->println("Version: " VERSION_STR);
	narf::console->println("");
	narf::console->println("Authors:");
	auto authors = narf::util::tokenize(VERSION_AUTHORS, '\n');
	for (auto& a : authors) {
		narf::console->println(a);
	}

	narf::console->println("");
	narf::console->println("Library versions:");

	narf::console->println("ENet " + std::to_string(ENET_VERSION_MAJOR) + "." + std::to_string(ENET_VERSION_MINOR) + "." + std::to_string(ENET_VERSION_PATCH));

	SDL_version sdl;
	SDL_GetVersion(&sdl);
	narf::console->println("SDL " + std::to_string(sdl.major) + "." + std::to_string(sdl.minor) + "." + std::to_string(sdl.patch));

	narf::console->println("zlib " + std::string(zlibVersion()));

	auto png = png_access_version_number();
	auto pngMajor = png / 10000;
	auto pngMinor = (png / 100) % 100;
	auto pngRelease = png % 100;
	narf::console->println("libpng " + std::to_string(pngMajor) + "." + std::to_string(pngMinor) + "." + std::to_string(pngRelease));

	FT_Library ftlib;
	FT_Init_FreeType(&ftlib);
	FT_Int ftMajor, ftMinor, ftPatch;
	FT_Library_Version(ftlib, &ftMajor, &ftMinor, &ftPatch);
	FT_Done_FreeType(ftlib);
	narf::console->println("FreeType " + std::to_string(ftMajor) + "." + std::to_string(ftMinor) + "." + std::to_string(ftPatch));

	narf::console->println("");
	narf::console->println("OpenGL information:");
	narf::console->println("OpenGL version " + std::string(display->glVersion));
	narf::console->println("GLSL version " + std::string(display->glslVersion));
	narf::console->println("GL context version " + std::to_string(display->glContextVersionMajor) + "." + std::to_string(display->glContextVersionMinor));
}


void fatalError(const std::string& msg) {
	if (narf::console) {
		narf::console->println(msg);
	}
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "NarfBlock: fatal error", msg.c_str(), nullptr);
}


extern "C" int main(int argc, char **argv)
{
#if _WIN32
	// hack to redirect output to text files
	freopen("stdout.txt", "w", stdout);
	freopen("stderr.txt", "w", stderr);
#endif

	clientConsole = new narf::ClientConsole();
	narf::console = clientConsole;

	narf::console->println("Version: " + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR) + std::string(VERSION_RELEASE) + "+" VERSION_REV);

	if (enet_initialize() != 0) {
		narf::console->println("Error initializing ENet");
		return 1;
	}

	narf::cmd::cmds["set"] = cmdSet;
	narf::cmd::cmds["quit"] = cmdQuit;
	narf::cmd::cmds["connect"] = cmdConnect;
	narf::cmd::cmds["disconnect"] = cmdDisconnect;
	narf::cmd::cmds["save"] = cmdSave;
	narf::cmd::cmds["saveconfig"] = cmdSaveConfig;
	narf::cmd::cmds["load"] = cmdLoad;
	narf::cmd::cmds["stats"] = cmdStats;
	narf::cmd::cmds["about"] = cmdAbout;

	narf::MemoryFile iniMem;
	auto configFile = narf::util::appendPath(narf::util::userConfigDir("narfblock"), "client.ini");
	narf::console->println("Attempting to open user config file: " + configFile);
	if (!iniMem.read(configFile)) {
		configFile = narf::util::appendPath(narf::util::dataDir(), "client.ini");
		narf::console->println("Could not load user config file; falling back to local config file: " + configFile);
		if (!iniMem.read(configFile)) {
			narf::console->println("Could not load local config file; falling back to compile-time defaults");
		}
	}

	if (iniMem.size) {
		config.load(iniMem.data, iniMem.size);
	}

	config.updateHandler = configEvent;

	client = enet_host_create(nullptr, 1, narf::net::MAX_CHANNELS, 0, 0);
	if (!client) {
		fatalError("Could not create ENet client");
		return 1;
	}

	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
		fatalError("SDL_Init(SDL_INIT_EVERYTHING) failed: " + std::string(SDL_GetError()));
		SDL_Quit();
		return 1;
	}

	SDL_DisplayMode mode;
	// TODO: iterate over monitors?
	if (SDL_GetDesktopDisplayMode(0, &mode)) {
		fatalError("SDL_GetDesktopDisplayMode failed: " + std::string(SDL_GetError()));
		SDL_Quit();
		return 1;
	}

	auto w = mode.w;
	auto h = mode.h;

	narf::console->println("Current video mode is " + std::to_string(w) + "x" + std::to_string(h));

	// TODO: convert these to be modifiable at runtime and use config.init*
	bool fullscreen = config.getBool("video.fullscreen", true);
	float width_cfg = config.getFloat("video.width", 0.6f);
	float height_cfg = config.getFloat("video.height", 0.6f);
	if (!fullscreen) {
		if (width_cfg > 1) {
			w = (int32_t)width_cfg;
		} else {
			w = (int32_t)((float)w * width_cfg);
		}
		if (height_cfg > 1) {
			h = (int32_t)height_cfg;
		} else {
			h = (int32_t)((float)h * height_cfg);
		}
		narf::console->println("Setting video to windowed " + std::to_string(w) + "x" + std::to_string(h));
	} else {
		narf::console->println("Setting video to fullscreen");
	}

	if (!initVideo(w, h, fullscreen)) {
		fatalError("Error: could not set OpenGL video mode " + std::to_string(w) + "x" + std::to_string(h));
		SDL_Quit();
		return 1;
	}

	clientConsole->setGLContext(display);

	config.initBool("video.vsync", false);

	newWorld();

	playerEID = world->entityManager.newEntity();
	{
		narf::EntityRef player(world->entityManager, playerEID);

		// initial player position
		player->position = narf::Vector3f(15.0f, 10.0f, 3.0f * 16.0f);
		player->prevPosition = player->position;
	}

	// initialize camera to look at origin
	cam.orientation.yaw = atan2f(cam.position.y, cam.position.x);
	cam.orientation.pitch = 0.0f;

	bouncyBlockEID = world->entityManager.newEntity();
	{
		narf::EntityRef bouncyBlock(world->entityManager, bouncyBlockEID);
		bouncyBlock->position = narf::Vector3f(10.0f, 10.0f, 21.0f);
		bouncyBlock->prevPosition = bouncyBlock->position;
		bouncyBlock->bouncy = true;
		bouncyBlock->model = true;
	}

	if (!init_textures()) {
		fatalError("init_textures() failed");
		return 1;
	}

	renderer = new narf::Renderer(world, *display, tilesTex);

	config.initInt32("video.renderDistance", 5);

	fpsTextBuffer = new narf::font::TextBuffer(*display, nullptr);
	blockInfoBuffer = new narf::font::TextBuffer(*display, nullptr);
	entityInfoBuffer = new narf::font::TextBuffer(*display, nullptr);
	locationBuffer = new narf::font::TextBuffer(*display, nullptr);

	config.initString("video.hudFont", "DroidSansMono");
	config.initInt32("video.hudFontSize", 30);
	if (!fpsTextBuffer->getFont()) {
		fatalError("Error: could not load HUD font");
		return 1;
	}

	config.initString("video.consoleFont", "DroidSansMono");
	config.initInt32("video.consoleFontSize", 18);
	if (!clientConsole->getFont()) {
		fatalError("Error: could not load Console font");
		return 1;
	}

	config.initString("video.consoleCursorShape", "default");

	config.initBool("video.stereo.enabled", false);
	config.initBool("video.stereo.cross", true);
	config.initFloat("video.stereo.separation", 0.1f);

	SDL_SetRelativeMouseMode(SDL_TRUE);

	config.initDouble("misc.tickRate", 60);
	config.initDouble("misc.maxFrameTime", 0.25);

	narf::console->println("Unicode test\xE2\x80\xBC pi: \xCF\x80 (2-byte sequence), square root: \xE2\x88\x9A (3-byte sequence), U+2070E: \xF0\xA0\x9C\x8E (4-byte sequence)");

	gameLoop = new ClientGameLoop(config.getDouble("misc.maxFrameTime"), config.getDouble("misc.tickRate"));
	gameLoop->run();

	if (connectState == ConnectState::Connected) {
		enet_peer_disconnect(server, (uint32_t)narf::net::DisconnectType::UserQuit);
	} else if (connectState == ConnectState::Connecting) {
		enet_peer_reset(server);
	}

	enet_host_flush(client);
	enet_host_destroy(client);
	enet_deinitialize();
	SDL_Quit();
	return 0;
}
