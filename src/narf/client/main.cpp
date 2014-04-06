#include <math.h>

#include <SDL.h>
#include <SDL_image.h>

#if SDL_MAJOR_VERSION < 2
#error SDL2 required
#endif

#include <Poco/Path.h>
#include <Poco/File.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "narf/version.h"

#include "narf/camera.h"
#include "narf/color.h"
#include "narf/entity.h"
#include "narf/font.h"
#include "narf/input.h"
#include "narf/block.h"
#include "narf/cmd/cmd.h"
#include "narf/config/config.h"
#include "narf/math/math.h"
#include "narf/util/path.h"
#include "narf/util/tokenize.h"

#include "narf/client/console.h"
#include "narf/client/world.h"
#include "narf/client/chunk.h"

#include "narf/gl/gl.h"

#include "narf/notifications/ConfigNotification.h"
#include <Poco/NObserver.h>

// TODO: this is all hacky test code - refactor into nicely modularized code

narf::client::Console *clientConsole;

narf::Entity::ID playerEID;
narf::Entity::ID bouncyBlockEID; // temp hack

narf::config::ConfigManager config;

narf::Camera cam;

const float movespeed = 25.0f;

narf::client::World *world;

#define WORLD_X_MAX 512
#define WORLD_Y_MAX 512
#define WORLD_Z_MAX 64

SDL_Surface *tiles_surf;

narf::gl::Context *display;
narf::gl::Texture *tiles_tex;

narf::font::FontManager font_manager;
narf::font::TextBuffer *fps_text_buffer;
narf::font::TextBuffer *block_info_buffer;
narf::font::TextBuffer *entityInfoBuffer;
narf::font::TextBuffer *location_buffer;

narf::BlockWrapper selected_block_face;

// debug options
bool wireframe = false;
bool backface_culling = true;
bool fog = true;
int screenshot = 0;

GLfloat fogColor[4] = {0.5f, 0.5f, 0.5f, 1.0f};

double physicsRate;
double physicsTickStep;
double maxFrameTime;

bool quitGameLoop = false;
bool forceHudUpdate = false;


narf::font::Font* setFont(
	const std::string& useName, // e.g. "Console" or "HUD"
	const std::string& nameVar, const std::string& nameDefault,
	const std::string& sizeVar, int sizeDefault) {
	auto fontName = config.getString(nameVar, nameDefault);
	auto fontSize = config.getInt(sizeVar, sizeDefault);

	narf::console->println("Setting " + useName + " font to " + fontName + " " + std::to_string(fontSize) + "px");

	auto font = font_manager.getFont(fontName, fontSize);
	if (!font) {
		narf::console->println("Error: could not load font " + fontName);
	}

	return font;
}


class TestObserver {
	public:
		void handler(const Poco::AutoPtr<narf::config::ConfigUpdateNotification>& pNf) {
			// TODO: this could be done better...
			if (pNf->key == "client.video.renderDistance") {
				world->renderDistance = config.getInt(pNf->key);
				narf::console->println("Setting renderDistance to " + std::to_string(world->renderDistance));
			} else if (pNf->key == "client.video.consoleCursorShape") {
				auto shapeStr = config.getString(pNf->key);
				clientConsole->setCursorShape(narf::client::Console::cursorShapeFromString(shapeStr));
			} else if (pNf->key == "client.video.vsync") {
				auto vsync = config.getBool(pNf->key);
				display->setVsync(vsync);
				narf::console->println("Setting vsync to " + std::to_string(vsync));
			} else if (pNf->key == "client.foo.gravity") {
				world->set_gravity((float)config.getDouble(pNf->key));
				narf::console->println("Setting gravity to " + std::to_string(world->get_gravity()));
			} else if (pNf->key == "client.misc.physicsRate") {
				physicsRate = config.getDouble(pNf->key);
				physicsTickStep = 1.0 / physicsRate; // fixed time step
				narf::console->println("Setting physicsRate to " + std::to_string(physicsRate));
			} else if (pNf->key == "client.misc.maxFrameTime") {
				maxFrameTime = config.getDouble(pNf->key, 0.25);
				narf::console->println("Setting maxFrameTime to " + std::to_string(maxFrameTime));
			} else if (pNf->key == "client.video.hudFont" ||
			           pNf->key == "client.video.hudFontSize") {
				auto font = setFont(
					"HUD",
					"client.video.hudFont", "DroidSansMono",
					"client.video.hudFontSize", 30);
				if (font) {
					forceHudUpdate = true;
					fps_text_buffer->setFont(font);
					block_info_buffer->setFont(font);
					entityInfoBuffer->setFont(font);
					location_buffer->setFont(font);
				}
			} else if (pNf->key == "client.video.consoleFont" ||
			           pNf->key == "client.video.consoleFontSize") {
				auto font = setFont(
					"console",
					"client.video.consoleFont", "DroidSansMono",
					"client.video.consoleFontSize", 18);
				if (font) {
					clientConsole->setFont(font);
				}
			} else {
				narf::console->println("Config var updated: " + pNf->key);
			}
		}
};

float clampf(float val, float min, float max)
{
	if (val < min) val = min;
	if (val > max) val = max;
	return val;
}


bool init_video(int w, int h, bool fullscreen)
{
	display = new narf::gl::Context();
	if (!display->setDisplayMode("NarfBlock", w, h, fullscreen)) {
		return false;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	return true;
}


bool init_textures()
{
	int imgFlags = IMG_INIT_PNG;
	if (IMG_Init(imgFlags) != imgFlags) {
		narf::console->println("IMG_Init failed: " + std::string(IMG_GetError()));
		return false;
	}

	const std::string terrain_file = config.getString("client.misc.terrain", "terrain.png");
	auto terrain_file_path = Poco::Path(narf::util::dataDir(), terrain_file);

	tiles_surf = IMG_Load(terrain_file_path.toString().c_str());
	if (!tiles_surf) {
		narf::console->println("IMG_Load(" + terrain_file_path.toString() + ") failed: " + std::string(IMG_GetError()));
		SDL_Quit();
		return false;
	}

	tiles_tex = new narf::gl::Texture(display);
	if (!tiles_tex->upload(tiles_surf)) {
		assert(0);
		return false;
	}

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


void draw3d() {
	// draw 3d world and objects

	// viewer projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//float fovx = 90.0f; // degrees
	float fovy = 60.0f; // degrees
	float aspect = (float)display->width() / (float)display->height() ; // TODO: include fovx in calculation
	gluPerspective(fovy, aspect, 0.1, 1000.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);

	if (backface_culling) {
		glEnable(GL_CULL_FACE);
	}

	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (fog) {
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glHint(GL_FOG_HINT, GL_DONT_CARE);
		auto renderDistance = float(world->renderDistance - 1) * 16.0f; // TODO - don't hardcode chunk size
		auto fogStart = std::max(renderDistance - 48.0f, 8.0f);
		auto fogEnd = std::max(renderDistance, 16.0f);
		glFogf(GL_FOG_START, fogStart);
		glFogf(GL_FOG_END, fogEnd);
		glEnable(GL_FOG);
	} else {
		glDisable(GL_FOG);
	}

	glEnable(GL_TEXTURE_2D);

	world->render(tiles_tex, &cam);

	if (selected_block_face.block) {
		// draw a selection rectangle
		drawCubeHighlight(selected_block_face);
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

	draw_cursor();

	auto blue = narf::Color(0.0f, 0.0f, 1.0f);

	auto hudFontHeight = block_info_buffer->getFont()->height();

	block_info_buffer->clear();
	if (selected_block_face.block) {
		std::string block_info_str = "Block info: ";

		const char *BlockFace_str[] = {"East", "West", "North", "South", "Top", "Bottom", "Invalid"};

		block_info_str += "ID: " + std::to_string(selected_block_face.block->id) +
			" Pos: " + std::to_string(selected_block_face.x) +
			", " + std::to_string(selected_block_face.y) +
			", " + std::to_string(selected_block_face.z) +
			" " + BlockFace_str[selected_block_face.face] +
			" (" + std::to_string((int)selected_block_face.face) + ")";

		block_info_buffer->print(block_info_str, 0, (float)display->height() - hudFontHeight * 4.0f, blue);
	}

	entityInfoBuffer->clear();
	entityInfoBuffer->print("numEntities: " + std::to_string(world->getNumEntities()), 0, (float)display->height() - hudFontHeight * 3.0f, blue);

	std::string location_str;
	{
		narf::EntityRef player(world, playerEID);
		location_str = "Pos: " + std::to_string(player->position.x) + ", " + std::to_string(player->position.y) + ", " + std::to_string(player->position.z);
	}
	location_str += " Yaw: " + std::to_string(cam.orientation.yaw) + " Pitch: " + std::to_string(cam.orientation.pitch);
	location_buffer->clear();
	location_buffer->print(location_str, 0, (float)display->height() - hudFontHeight * 2.0f, blue);

	clientConsole->render();
	fps_text_buffer->render();
	block_info_buffer->render();
	entityInfoBuffer->render();
	location_buffer->render();
}


void draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	draw3d();
	draw2d();

	display->swap();

	if (screenshot == 1) {
		// http://stackoverflow.com/questions/5862097/sdl-opengl-screenshot-is-black hax hax
		SDL_Surface* image = SDL_CreateRGBSurface(SDL_SWSURFACE, display->width(), display->height(), 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
		SDL_Surface* result = SDL_CreateRGBSurface(SDL_SWSURFACE, display->width(), display->height(), 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
		glReadPixels(0, 0, display->width(), display->height(), GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
		narf::console->println("Taking a screenshot. " + std::to_string(image->w) + "x" + std::to_string(image->h) + "x24");

		// Flip upside down
		for(int32_t y = 0; y < image->h; ++y) {
			for(int32_t x = 0; x < image->w; ++x) {
				for(int32_t b = 0; b < 3; ++b) {
					((uint8_t*)result->pixels)[3 * (y * image->w + x) + b] = ((uint8_t*)image->pixels)[3 * ((image->h - y - 1) * image->w + x) + b];
				}
			}
		}
		SDL_SaveBMP(result, "pic.bmp");
		SDL_FreeSurface(image);
		SDL_FreeSurface(result);
		screenshot = 2;
	}
}


void poll_input(narf::Input *input)
{
	SDL_Event e;

	input->begin_sample();
	while (SDL_PollEvent(&e)) {
		input->process_event(&e);
	}
	input->end_sample();
}


void sim_frame(const narf::Input &input, double t, double dt)
{
	if (input.text() != "") {
		narf::cmd::exec(input.text());
	}


	// tell the console whether to draw the cursor
	clientConsole->setEditState(input.state() == narf::Input::InputStateText);

	// TODO: decouple player direction and camera direction
	cam.orientation.yaw -= input.look_rel().x;
	cam.orientation.yaw = fmodf(cam.orientation.yaw, (float)M_PI * 2.0f);

	cam.orientation.pitch.minimum = -(float)M_PI;
	cam.orientation.pitch.maximum = (float)M_PI;
	cam.orientation.pitch -= input.look_rel().y;
	cam.orientation.pitch = clampf(cam.orientation.pitch, -(float)M_PI/2, (float)M_PI/2);

	narf::math::Vector3f vel_rel(0.0f, 0.0f, 0.0f);

	float movePitch = 0.0f;

	{
		narf::EntityRef player(world, playerEID);

		if (player->antigrav) {
			// if flying, move in the direction of camera including pitch
			movePitch = cam.orientation.pitch;
		}

		if (input.move_forward()) {
			vel_rel += narf::math::Orientationf(movePitch, cam.orientation.yaw);
		} else if (input.move_backward()) {
			vel_rel -= narf::math::Orientationf(movePitch, cam.orientation.yaw);
		}

		if (input.strafe_left()) {
			vel_rel += narf::math::Orientationf(0.0f, cam.orientation.yaw + (float)M_PI / 2);
		} else if (input.strafe_right()) {
			vel_rel -= narf::math::Orientationf(0.0f, cam.orientation.yaw + (float)M_PI / 2);
		}

		// normalize so that diagonal movement is not faster than cardinal directions
		vel_rel = vel_rel.normalize() * movespeed;

		if (input.jump()) {
			if (player->onGround) {
				vel_rel += narf::math::Vector3f(0.0f, 0.0f, 8.0f);
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

	world->update(t, dt);

	{
		narf::EntityRef player(world, playerEID);
		if (player->onGround && player->antigrav) {
			player->antigrav = false;
			narf::console->println("left antigrav mode");
		}

		// lock camera to player
		cam.position = player->position;
		cam.position.z += 1.6f;
	}

	// Let's see what we're looking at
	auto pos = narf::math::coord::Point3f(cam.position.x, cam.position.y, cam.position.z);
	selected_block_face = world->rayTrace(pos, cam.orientation, 7.5);

	if (selected_block_face.block != nullptr) {
		if (input.action_primary_begin() || input.action_secondary_begin()) {
			narf::console->println("got " + std::string(input.action_primary_begin() ? "left" : "right") + " click " +
				std::to_string(selected_block_face.x) + " " +
				std::to_string(selected_block_face.y) + " " +
				std::to_string(selected_block_face.z));

			narf::Block b;
			uint32_t x = selected_block_face.x;
			uint32_t y = selected_block_face.y;
			uint32_t z = selected_block_face.z;
			if (input.action_secondary_begin()) {
				// remove block at cursor
				b.id = 0;
			} else {
				// add new block next to selected face
				switch (selected_block_face.face) {
				case narf::BlockFace::XPos: x++; break;
				case narf::BlockFace::XNeg: x--; break;
				case narf::BlockFace::YPos: y++; break;
				case narf::BlockFace::YNeg: y--; break;
				case narf::BlockFace::ZPos: z++; break;
				case narf::BlockFace::ZNeg: z--; break;
				case narf::BlockFace::Invalid: assert(0); break;
				}
				b.id = 5;
			}
			world->put_block(&b, x, y, z);
		}
	}

	if (input.action_ternary()) {
		narf::console->println("got middle click");
		// fire a new entity
		auto eid = world->newEntity();

		narf::EntityRef ent(world, eid);
		narf::EntityRef player(world, playerEID);

		ent->position = cam.position;
		ent->velocity = player->velocity + narf::math::Vector3f(cam.orientation).normalize() * 20.0f;
		ent->model = true;
		ent->bouncy = false;
		ent->explodey = true;
	}

	if (input.toggle_wireframe()) {
		wireframe = !wireframe;
	}

	if (input.toggle_backface_culling()) {
		backface_culling = !backface_culling;
	}

	if (input.toggle_fog()) {
		fog = !fog;
	}

	if (input.screenshot()) { // Hack in a screenshot function
		if (screenshot == 0) {
			screenshot = 1;
		}
	} else {
		screenshot = 0;
	}
}


double get_time()
{
	return (double)SDL_GetTicks() * 0.001; // SDL tick is a millisecond; convert to seconds
}


void game_loop()
{
	const float input_divider = static_cast<float>(config.getDouble("client.misc.inputDivider", 1000));
	narf::Input input(clientConsole->getTextEditor(), 1.0f / input_divider, 1.0f / input_divider);
	double t = 0.0;
	double t1 = get_time();
	double t_accum = 0.0;

	double fps_t1 = get_time();
	unsigned physics_steps = 0;
	unsigned draws = 0;

	while (1) {
		double t2 = get_time();
		double frame_time = t2 - t1;

		if (frame_time > maxFrameTime) {
			frame_time = maxFrameTime;
		}

		t1 = t2;

		t_accum += frame_time;

		while (t_accum >= physicsTickStep)
		{
			poll_input(&input);
			if (input.exit() || quitGameLoop) {
				return;
			}
			sim_frame(input, t, physicsTickStep);
			physics_steps++;

			t_accum -= physicsTickStep;
			t += physicsTickStep;
		}

		double fps_dt = get_time() - fps_t1;
		if (fps_dt >= 1.0 || forceHudUpdate) {
			forceHudUpdate = false;
			// update fps counter
			auto hudFontHeight = fps_text_buffer->getFont()->height();
			std::string fps_str = std::to_string((double)physics_steps / fps_dt) + " physics steps/" +
				std::to_string((double)draws / fps_dt) + " renders per second (dt " + std::to_string(fps_dt) + ")";
			auto blue = narf::Color(0.0f, 0.0f, 1.0f);
			fps_text_buffer->clear();
			fps_text_buffer->print(fps_str, 0.0f, (float)display->height() - hudFontHeight, blue);
			fps_t1 = get_time();
			draws = physics_steps = 0;
		}

		draw();
		draws++;
	}
}


float randf(float min, float max)
{
	return ((float)rand() / (float)RAND_MAX) * (max - min + 1.0f) + min;
}

int randi(int min, int max)
{
	return (int)randf((float)min, (float)max); // HAX
}

void fill_rect_prism(uint32_t x1, uint32_t x2, uint32_t y1, uint32_t y2, uint32_t z1, uint32_t z2, uint8_t block_id) {
	for (uint32_t z = z1; z < z2; z++) {
		for (uint32_t y = y1; y < y2; y++) {
			for (uint32_t x = x1; x < x2; x++) {
				narf::Block b;
				b.id = block_id;
				world->put_block(&b, x, y, z);
			}
		}
	}
}

void fill_plane(uint32_t z, uint8_t block_id) {
	fill_rect_prism(0, WORLD_X_MAX, 0, WORLD_Y_MAX, z, z + 1, block_id);
}


void calcTexCoord(narf::BlockTexCoord *tc, unsigned texId) {
	unsigned texX = texId % 16;
	unsigned texY = texId / 16;

	float texCoordTileSize = 1.0f / 16.0f; // TODO: calculate from actual texture size

	tc->u1 = (float)texX * texCoordTileSize;
	tc->v1 = (float)texY * texCoordTileSize;

	tc->u2 = tc->u1 + texCoordTileSize;
	tc->v2 = tc->v1 + texCoordTileSize;
}


// calc tex coords from id
narf::BlockType genNormalBlockType(unsigned texXPos, unsigned texXNeg, unsigned texYPos, unsigned texYNeg, unsigned texZPos, unsigned texZNeg) {
	narf::BlockType bt;
	bt.solid = true;
	bt.indestructible = false;
	calcTexCoord(&bt.texCoords[narf::XPos], texXPos);
	calcTexCoord(&bt.texCoords[narf::XNeg], texXNeg);
	calcTexCoord(&bt.texCoords[narf::YPos], texYPos);
	calcTexCoord(&bt.texCoords[narf::YNeg], texYNeg);
	calcTexCoord(&bt.texCoords[narf::ZPos], texZPos);
	calcTexCoord(&bt.texCoords[narf::ZNeg], texZNeg);
	return bt;
}


void gen_world()
{
	world = new narf::client::World(WORLD_X_MAX, WORLD_Y_MAX, WORLD_Z_MAX, 16, 16, 16);

	// set up block types
	// TODO: put this in a config file
	auto airType = genNormalBlockType(0, 0, 0, 0, 0, 0); // TODO
	airType.solid = false;
	world->addBlockType(airType); // air

	auto adminiumType = genNormalBlockType(4, 4, 4, 4, 4, 4);
	adminiumType.indestructible = true;
	world->addBlockType(adminiumType); // adminium

	world->addBlockType(genNormalBlockType(2, 2, 2, 2, 2, 2)); // dirt
	world->addBlockType(genNormalBlockType(3, 3, 3, 3, 0, 2)); // dirt with grass top
	world->addBlockType(genNormalBlockType(4, 4, 4, 4, 4, 4)); // TODO
	world->addBlockType(genNormalBlockType(5, 5, 5, 5, 5, 5)); // brick
	world->addBlockType(genNormalBlockType(1, 1, 1, 1, 1, 1)); // stone1
	auto stone2 = world->addBlockType(genNormalBlockType(16, 16, 16, 16, 16, 16)); // stone2
	world->addBlockType(genNormalBlockType(17, 17, 17, 17, 17, 17)); // stone3

	for (int z = 16; z < 23; z++) {
		fill_rect_prism(30 + z, 35 + (10 - z), 30 + z, 35 + (10 - z), z, z + 1, stone2);
	}

	// generate some random blocks above the ground
	for (int i = 0; i < 1000; i++) {
		int x = randi(0, WORLD_X_MAX - 1);
		int y = randi(0, WORLD_Y_MAX - 1);
		int z = randi(16, 23);
		narf::Block b;
		b.id = (uint8_t)randi(2, 3);
		world->put_block(&b, x, y, z);
	}

	for (int i = 0; i < 10; i++) {
		narf::Block b;
		b.id = stone2;
		world->put_block(&b, 5 + i, 5, 16);
		world->put_block(&b, 5, 5 + i, 16);
		world->put_block(&b, 5 + i, 15, 16);
	}

	world->set_gravity(-24.0f);
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
		config.setRaw(key, value);
	} else {
		narf::console->println("wrong number of parameters to set");
	}
}

void cmdQuit(const std::string &args) {
	narf::console->println("Quitting in response to user command");
	quitGameLoop = true;
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

	clientConsole = new narf::client::Console();
	TestObserver testobserver;

	narf::console = clientConsole;

	narf::console->println("Version: " + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR) + std::string(VERSION_RELEASE) + "+" VERSION_REV);

	narf::cmd::cmds["set"] = cmdSet;
	narf::cmd::cmds["quit"] = cmdQuit;

	auto config_file = Poco::Path(narf::util::dataDir(), "client.ini").toString();
	narf::console->println("Client config file: " + config_file);
	config.load("client", config_file);
	config.notificationCenter.addObserver(Poco::NObserver<TestObserver, narf::config::ConfigUpdateNotification>(testobserver, &TestObserver::handler));

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
	bool fullscreen = config.getBool("client.video.fullscreen", true);
	float width_cfg = static_cast<float>(config.getDouble("client.video.width", 0.6));
	float height_cfg = static_cast<float>(config.getDouble("client.video.height", 0.6));
	if (!fullscreen) {
		if (width_cfg > 1) {
			w = (int)width_cfg;
		} else {
			w = (int)((float)w * width_cfg);
		}
		if (height_cfg > 1) {
			h = (int)height_cfg;
		} else {
			h = (int)((float)h * height_cfg);
		}
		narf::console->println("Setting video to windowed " + std::to_string(w) + "x" + std::to_string(h));
	} else {
		narf::console->println("Setting video to fullscreen");
	}

	// TODO: read w, h, bpp from config file to override defaults

	if (!init_video(w, h, fullscreen)) {
		fatalError("Error: could not set OpenGL video mode " + std::to_string(w) + "x" + std::to_string(h));
		SDL_Quit();
		return 1;
	}

	config.initBool("client.video.vsync", false);

	srand(0x1234);
	gen_world();

	config.initInt("client.video.renderDistance", 5);

	playerEID = world->newEntity();
	{
		narf::EntityRef player(world, playerEID);

		// initial player position
		player->position = narf::math::Vector3f(15.0f, 10.0f, 16.0f);
	}

	// initialize camera to look at origin
	cam.orientation.yaw = atan2f(cam.position.y, cam.position.x);
	cam.orientation.pitch = 0.0f;

	bouncyBlockEID = world->newEntity();
	{
		narf::EntityRef bouncyBlock(world, bouncyBlockEID);
		bouncyBlock->position = narf::math::Vector3f(10.0f, 10.0f, 21.0f);
		bouncyBlock->bouncy = true;
		bouncyBlock->model = true;
	}

	if (!init_textures()) {
		fatalError("init_textures() failed");
		return 1;
	}

	fps_text_buffer = new narf::font::TextBuffer(nullptr);
	block_info_buffer = new narf::font::TextBuffer(nullptr);
	entityInfoBuffer = new narf::font::TextBuffer(nullptr);
	location_buffer = new narf::font::TextBuffer(nullptr);

	config.initString("client.video.hudFont", "DroidSansMono");
	config.initInt("client.video.hudFontSize", 30);
	if (!fps_text_buffer->getFont()) {
		fatalError("Error: could not load HUD font");
		return 1;
	}

	config.initString("client.video.consoleFont", "DroidSansMono");
	config.initInt("client.video.consoleFontSize", 18);
	if (!clientConsole->getFont()) {
		fatalError("Error: could not load Console font");
		return 1;
	}

	auto consoleX = 0;
	auto consoleY = 0;
	auto consoleWidth = display->width();
	auto consoleHeight = 175; // TODO: calculate dynamically based on screen size

	config.initString("client.video.consoleCursorShape", "default");

	narf::console->println("Console location: (" +
		std::to_string(consoleX) + ", " + std::to_string(consoleY) + ") " +
		std::to_string(consoleWidth) + "x" + std::to_string(consoleHeight));

	clientConsole->setLocation(consoleX, consoleY, consoleWidth, consoleHeight);

	SDL_SetRelativeMouseMode(SDL_TRUE);

	config.initDouble("client.misc.physicsRate", 60);
	config.initDouble("client.misc.maxFrameTime", 0.25);

	game_loop();

	SDL_Quit();
	return 0;
}
