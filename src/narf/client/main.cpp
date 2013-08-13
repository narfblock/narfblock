#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <Poco/Path.h>
#include <Poco/File.h>

#include <math.h>

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
#include "narf/config/config.h"
#include "narf/math/math.h"

#include "narf/client/world.h"
#include "narf/client/chunk.h"

#include "narf/gl/gl.h"

// TODO: this is all hacky test code - refactor into nicely modularized code

narf::Entity *player;
narf::Entity *bouncy_block; // temp hack

narf::config::ConfigManager configmanager;

narf::Camera cam;

const float movespeed = 25.0f;

narf::client::World *world;

#define WORLD_X_MAX 512
#define WORLD_Y_MAX 512
#define WORLD_Z_MAX 64

SDL_Surface *tiles_surf;

narf::gl::Context *display;
narf::gl::Texture *tiles_tex;

Poco::Path data_dir;

narf::font::FontManager font_manager;
narf::font::TextBuffer *console_text_buffer;
narf::font::TextBuffer *fps_text_buffer;
narf::font::TextBuffer *block_info_buffer;
narf::font::TextBuffer *location_buffer;

narf::BlockWrapper selected_block_face;

// debug options
bool wireframe = false;
bool backface_culling = true;
int screenshot = 0;

float clampf(float val, float min, float max)
{
	if (val < min) val = min;
	if (val > max) val = max;
	return val;
}


bool init_video(int w, int h, int bpp, bool fullscreen)
{
	display = new narf::gl::Context();
	if (!display->set_display_mode(w, h, bpp, fullscreen)) {
		return false;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	return true;
}


bool init_textures()
{
	const std::string terrain_file = configmanager.getString("test.misc.terrain", "terrain.png");
	tiles_surf = IMG_Load((Poco::Path(data_dir, terrain_file)).toString().c_str());
	if (!tiles_surf) {
		fprintf(stderr, "%s not found!\n", terrain_file.c_str());
		SDL_Quit();
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
void drawCubeHighlight(float x, float y, float z, narf::BlockFace face)
{
	const float cube_quads[][4*3] = {
		{x+1,y+0,z+0, x+1,y+1,z+0, x+1,y+1,z+1, x+1,y+0,z+1}, // XPos
		{x+0,y+0,z+0, x+0,y+0,z+1, x+0,y+1,z+1, x+0,y+1,z+0}, // XNeg
		{x+1,y+1,z+0, x+0,y+1,z+0, x+0,y+1,z+1, x+1,y+1,z+1}, // YPos
		{x+1,y+0,z+0, x+1,y+0,z+1, x+0,y+0,z+1, x+0,y+0,z+0}, // YNeg
		{x+0,y+0,z+1, x+1,y+0,z+1, x+1,y+1,z+1, x+0,y+1,z+1}, // ZPos
		{x+0,y+1,z+0, x+1,y+1,z+0, x+1,y+0,z+0, x+0,y+0,z+0}, // ZNeg
	};

	assert(face < sizeof(cube_quads) / sizeof(*cube_quads));
	drawHighlightQuad(cube_quads[face]);
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

	glEnable(GL_TEXTURE_2D);

	world->render(tiles_tex, &cam);

	if (selected_block_face.block) {
		// draw a selection rectangle
		drawCubeHighlight(selected_block_face.x, selected_block_face.y, selected_block_face.z, selected_block_face.face);
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

	glTranslatef(0.5f, 0.5f, 0.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	draw_cursor();

	auto blue = narf::Color(0.0f, 0.0f, 1.0f);

	block_info_buffer->clear();
	if (selected_block_face.block) {
		std::wstring block_info_str = L"Block info: ";

		const wchar_t *BlockFace_str[] = {L"East", L"West", L"North", L"South", L"Top", L"Bottom", L"Invalid"};

		block_info_str += L"ID: " + std::to_wstring(selected_block_face.block->id) +
			L" Pos: " + std::to_wstring(selected_block_face.x) +
			L", " + std::to_wstring(selected_block_face.y) +
			L", " + std::to_wstring(selected_block_face.z) +
			L" " + BlockFace_str[selected_block_face.face] +
			L" (" + std::to_wstring((int)selected_block_face.face) + L")";

		block_info_buffer->print(block_info_str, 0, 35, blue);
	}

	std::wstring location_str = L"Pos: " + std::to_wstring(player->position.x) + L", " + std::to_wstring(player->position.y) + L", " + std::to_wstring(player->position.z);
	location_str += L" Yaw: " + std::to_wstring(cam.orientation.yaw) + L" Pitch: " + std::to_wstring(cam.orientation.pitch);
	location_buffer->clear();
	location_buffer->print(location_str, 0, 70, blue);

	console_text_buffer->render();
	fps_text_buffer->render();
	block_info_buffer->render();
	location_buffer->render();
}


void draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	draw3d();
	draw2d();

	SDL_GL_SwapBuffers();

	if (screenshot == 1) {
		// http://stackoverflow.com/questions/5862097/sdl-opengl-screenshot-is-black hax hax
		SDL_Surface* image = SDL_CreateRGBSurface(SDL_SWSURFACE, display->width(), display->height(), 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
		SDL_Surface* result = SDL_CreateRGBSurface(SDL_SWSURFACE, display->width(), display->height(), 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0);
		glReadPixels(0, 0, display->width(), display->height(), GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
		printf("Taking a screenshot. %dx%dx24\n", image->w, image->h);

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
	// TODO: decouple player direction and camera direction
	cam.orientation.yaw -= input.look_rel().x;
	cam.orientation.yaw = fmodf(cam.orientation.yaw, (float)M_PI * 2.0f);

	cam.orientation.pitch.minimum = -(float)M_PI;
	cam.orientation.pitch.maximum = (float)M_PI;
	cam.orientation.pitch -= input.look_rel().y;
	cam.orientation.pitch = clampf(cam.orientation.pitch, -(float)M_PI/2, (float)M_PI/2);

	narf::math::Vector3f vel_rel(0.0f, 0.0f, 0.0f);

	if (input.move_forward()) {
		vel_rel += narf::math::Orientationf(0.0f, cam.orientation.yaw);
	} else if (input.move_backward()) {
		vel_rel -= narf::math::Orientationf(0.0f, cam.orientation.yaw);
	}

	if (input.strafe_left()) {
		vel_rel += narf::math::Orientationf(0.0f, cam.orientation.yaw + (float)M_PI / 2);
	} else if (input.strafe_right()) {
		vel_rel -= narf::math::Orientationf(0.0f, cam.orientation.yaw + (float)M_PI / 2);
	}

	// normalize so that diagonal movement is not faster than cardinal directions
	vel_rel = vel_rel.normalize() * movespeed;

	if (input.jump()) {
		vel_rel += narf::math::Vector3f(0.0f, 0.0f, 8.0f);
	}

	// hax
	player->velocity.x = vel_rel.x;
	player->velocity.y = vel_rel.y;
	player->velocity.z += vel_rel.z;

	world->update(t, dt);

	// lock camera to player
	cam.position = player->position;
	cam.position.z += 1.6f;

	// Let's see what we're looking at
	auto pos = narf::math::coord::Point3f(cam.position.x, cam.position.y, cam.position.z);
	selected_block_face = world->rayTrace(pos, cam.orientation, 7.5);

	if (selected_block_face.block != nullptr) {
		if (input.action_primary_begin() || input.action_secondary_begin()) {
			printf("got left click\n");
			printf("got non-null block %d %d %d\n", selected_block_face.x, selected_block_face.y, selected_block_face.z);
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
				}
				b.id = 5;
			}
			world->put_block(&b, x, y, z);
		}
	}

	if (input.toggle_wireframe()) {
		wireframe = !wireframe;
	}

	if (input.toggle_backface_culling()) {
		backface_culling = !backface_culling;
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
	const float input_divider = static_cast<float>(configmanager.getDouble("test.misc.input_divider", 1000));
	narf::Input input(1.0f / input_divider, 1.0f / input_divider);
	double t = 0.0;
	double t1 = get_time();
	const double physics_rate = configmanager.getDouble("test.misc.physics_rate", 60);
	const double physics_tick_step = 1.0 / physics_rate; // fixed time step
	const double max_frame_time = configmanager.getDouble("test.misc.max_frame_time", 0.25);

	double t_accum = 0.0;

	double fps_t1 = get_time();
	unsigned physics_steps = 0;
	unsigned draws = 0;

	while (1) {
		double t2 = get_time();
		double frame_time = t2 - t1;

		if (frame_time > max_frame_time) {
			frame_time = max_frame_time;
		}

		t1 = t2;

		t_accum += frame_time;

		while (t_accum >= physics_tick_step)
		{
			poll_input(&input);
			if (input.exit()) {
				return;
			}
			sim_frame(input, t, physics_tick_step);
			physics_steps++;

			t_accum -= physics_tick_step;
			t += physics_tick_step;
		}

		double fps_dt = get_time() - fps_t1;
		if (fps_dt >= 1.0) {
			// update fps counter
			std::wstring fps_str = std::to_wstring((double)physics_steps / fps_dt) + L" physics steps/" +
				std::to_wstring((double)draws / fps_dt) + L" renders per second (dt " + std::to_wstring(fps_dt) + L")";
			auto blue = narf::Color(0.0f, 0.0f, 1.0f);
			fps_text_buffer->clear();
			fps_text_buffer->print(fps_str, 0.0f, (float)display->height() - 30 /* TODO */, blue);
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

void gen_world()
{
	world = new narf::client::World(WORLD_X_MAX, WORLD_Y_MAX, WORLD_Z_MAX, 16, 16, 16);

	for (int z = 16; z < 23; z++) {
		fill_rect_prism(30 + z, 35 + (10 - z), 30 + z, 35 + (10 - z), z, z + 1, 16);
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
		b.id = 16;
		world->put_block(&b, 5 + i, 5, 16);
		world->put_block(&b, 5, 5 + i, 16);
		world->put_block(&b, 5 + i, 15, 16);
	}

	world->set_gravity(-24.0f);
}


extern "C" int main(int argc, char **argv)
{
	printf("Version: %d.%d%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE);

	// walk up the path until data directory is found
	printf("Current Dir: %s\n", Poco::Path::current().c_str());
	Poco::File tmp;
	for (Poco::Path dir = Poco::Path::current(); dir.toString() != dir.parent().toString(); dir = dir.parent()) {
		data_dir = Poco::Path(dir, "data");
		printf("Checking %s (%s)\n", data_dir.toString().c_str(), dir.toString().c_str());
		tmp = data_dir;
		if (tmp.exists()) {
			printf("Found data directory: %s\n", data_dir.toString().c_str());
			break;
		}
	}

	// Will explode if things don't exist
	auto config_file = Poco::Path(data_dir, "config.ini").toString();
	printf("Config File: %s\n", config_file.c_str());
	configmanager.load("test", config_file);
	int bar = configmanager.getInt("test.foo.bar", 43);
	printf("ConfigManager test: test.foo.bar = %d\n", bar);

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("SDL_Init(SDL_INIT_EVERYTHING) failed: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	const SDL_VideoInfo *vid_info = SDL_GetVideoInfo();

	printf("Current video mode is %dx%dx%d\n", vid_info->current_w, vid_info->current_h, vid_info->vfmt->BitsPerPixel);

	int w = vid_info->current_w;
	int h = vid_info->current_h;
	int bpp = vid_info->vfmt->BitsPerPixel;
	bool fullscreen = configmanager.getBool("test.video.fullscreen", true);
	float width_cfg = static_cast<float>(configmanager.getDouble("test.video.width", 0.6));
	float height_cfg = static_cast<float>(configmanager.getDouble("test.video.height", 0.6));
	printf("Setting video to ");
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
		printf("%dx%d\n", w, h);
	} else {
		printf("fullscreen\n");
	}

	// TODO: read w, h, bpp from config file to override defaults

	SDL_WM_SetCaption("NarfBlock", "NarfBlock");

	if (!init_video(w, h, bpp, fullscreen)) {
		fprintf(stderr, "Error: could not set OpenGL video mode %dx%d@%d bpp\n", w, h, bpp);
		SDL_Quit();
		return 1;
	}

	srand(0x1234);
	gen_world();

	world->renderDistance = configmanager.getInt("test.video.render_distance", 5);

	player = world->newEntity();

	// initial player position
	player->position = narf::math::Vector3f(15.0f, 10.0f, 16.0f);

	// initialize camera to look at origin
	cam.orientation.yaw = atan2f(cam.position.y, cam.position.x);
	cam.orientation.pitch = 0.0f;

	bouncy_block = world->newEntity();
	bouncy_block->position = narf::math::Vector3f(10.0f, 10.0f, 21.0f);
	bouncy_block->bouncy = true;
	bouncy_block->model = true;

	init_textures();
	auto font_file = Poco::Path(data_dir, "DroidSansMono.ttf").toString();
	printf("Loading font from %s\n", font_file.c_str());

	auto font = font_manager.addFont("DroidSansMono", font_file, 30);
	if (!font) {
		fprintf(stderr, "Error: could not load DroidSansMono\n");
		return 1;
	}

	console_text_buffer = new narf::font::TextBuffer(font);
	fps_text_buffer = new narf::font::TextBuffer(font);
	block_info_buffer = new narf::font::TextBuffer(font);
	location_buffer = new narf::font::TextBuffer(font);

	auto red = narf::Color(1.0f, 0.0f, 0.0f, 1.0f);
	console_text_buffer->print(L"Testing 123", 0, 10, red);

	SDL_ShowCursor(0);
	SDL_WM_GrabInput(SDL_GRAB_ON);

	game_loop();

	SDL_Quit();
	return 0;
}
