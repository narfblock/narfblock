#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <boost/filesystem.hpp>

#include <math.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "test.h"

#include "narf/camera.h"
#include "narf/entity.h"
#include "narf/input.h"
#include "narf/vector.h"
#include "narf/world.h"
#include "narf/config/configmanager.h"

#include "narf/gl/gl.h"

// TODO: this is all hacky test code - refactor into nicely modularized code

narf::Entity *player;
narf::Entity *bouncy_block; // temp hack

narf::config::ConfigManager configmanager;

narf::Camera cam;

const float movespeed = 25.0f;

narf::World *world;

#define WORLD_X_MAX 64
#define WORLD_Y_MAX 64
#define WORLD_Z_MAX 64

SDL_Surface *tiles_surf;

narf::gl::Context *display;
narf::gl::Texture *tiles_tex;

boost::filesystem::path data_dir;


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

	// viewer projection
	glMatrixMode(GL_PROJECTION);
	//float fovx = 90.0f; // degrees
	float fovy = 60.0f; // degrees
	float aspect = (float)w / (float)h ; // TODO: include fovx in calculation
	gluPerspective(fovy, aspect, 0.1, 1000.0f);
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_2D);

	// for drawing outlines
	glPolygonOffset(1.0, 2);
	glEnable(GL_POLYGON_OFFSET_FILL);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);

	return true;
}


bool init_textures()
{
	tiles_surf = IMG_Load((data_dir / "terrain.png").string().c_str());
	if (!tiles_surf) {
		fprintf(stderr, "terrain.png not found!\n");
		SDL_Quit();
	}

	tiles_tex = new narf::gl::Texture(display);
	if (!tiles_tex->upload(tiles_surf)) {
		assert(0);
		return false;
	}

	return true;
}


void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	world->render(tiles_tex, &cam);

	// temp hack: draw an entity as a cube for physics demo
	void draw_cube(float x, float y, float z, uint8_t type, unsigned draw_face_mask);
	draw_cube(bouncy_block->position.x, bouncy_block->position.y, bouncy_block->position.z, 1, 0xFF);

	SDL_GL_SwapBuffers();
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
	cam.yaw += input.look_rel().x;
	cam.yaw = fmodf(cam.yaw, (float)M_PI * 2.0f);

	cam.pitch += input.look_rel().y;
	cam.pitch = clampf(cam.pitch, -(float)M_PI / 2, (float)M_PI / 2);

	narf::Vector3f vel_rel(0.0f, 0.0f, 0.0f);

	if (input.move_forward()) {
		vel_rel -= narf::Vector3f(cosf(cam.yaw + (float)M_PI / 2), 0.0f, sinf(cam.yaw + (float)M_PI / 2));
	} else if (input.move_backward()) {
		vel_rel += narf::Vector3f(cosf(cam.yaw + (float)M_PI / 2), 0.0f, sinf(cam.yaw + (float)M_PI / 2));
	}

	if (input.strafe_left()) {
		vel_rel -= narf::Vector3f(cosf(cam.yaw), 0.0f, sinf(cam.yaw));
	} else if (input.strafe_right()) {
		vel_rel += narf::Vector3f(cosf(cam.yaw), 0.0f, sinf(cam.yaw));
	}

	// normalize so that diagonal movement is not faster than cardinal directions
	vel_rel = vel_rel.normalize() * movespeed;

	if (input.jump()) {
		vel_rel += narf::Vector3f(0.0f, 8.0f, 0.0f);
	}

	// hax
	player->velocity.x = vel_rel.x;
	player->velocity.y += vel_rel.y;
	player->velocity.z = vel_rel.z;

	player->update(t, dt);

	// lock camera to player
	cam.position = player->position;
	cam.position.y += 2.0f;

	bouncy_block->update(t, dt);
}


double get_time()
{
	return (double)SDL_GetTicks() * 0.001; // SDL tick is a millisecond; convert to seconds
}


void game_loop()
{
	narf::Input input(1.0f / 1000.0f, 1.0f / 1000.0f);
	double t = 0.0;
	double t1 = get_time();

	const double physics_rate = 120.0;
	const double physics_tick_step = 1.0 / physics_rate; // fixed time step
	const double max_frame_time = 0.25;

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
			printf("%f physics steps/%f renders per second (dt %f)\n", (double)physics_steps / fps_dt, (double)draws / fps_dt, fps_dt);
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
	return (int)randf(min, max); // HAX
}


void gen_world()
{
	world = new narf::World(WORLD_X_MAX, WORLD_Y_MAX, WORLD_Z_MAX, 16, 16, 16);

	// first fill a plane at y = 0
	for (int z = 0; z < WORLD_Z_MAX; z++) {
		for (int x = 0; x < WORLD_X_MAX; x++) {
			narf::Block b;
			b.id = 2; // grass
			world->put_block(&b, x, 0, z);
		}
	}

	// generate some random blocks above the ground
	for (int i = 0; i < 1000; i++) {
		int x = randi(0, WORLD_X_MAX - 1);
		int y = randi(1, 10);
		int z = randi(0, WORLD_Z_MAX - 1);
		narf::Block b;
		b.id = randi(1, 3);
		world->put_block(&b, x, y, z);
	}

	for (int i = 0; i < 10; i++) {
		narf::Block b;
		b.id = 16;
		world->put_block(&b, 5 + i, 1, 5);
		world->put_block(&b, 5, 1, 5 + i);
		world->put_block(&b, 5 + i, 1, 15);
	}

	world->set_gravity(-9.8f);
}


extern "C" int main(int argc, char **argv)
{
	printf("Version: %d.%d%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE);

	data_dir = boost::filesystem::current_path();

	// walk up the path until data directory is found
	for (auto dir = boost::filesystem::current_path(); dir != dir.root_path(); dir = dir.parent_path()) {
		data_dir = dir / "data";
		if (boost::filesystem::is_directory(data_dir)) {
			printf("Found data directory: %s\n", data_dir.string().c_str());
			break;
		}
	}

	// Will explode if things don't exist
	configmanager.load("test", (data_dir / "test.yaml").string());
	printf("ConfigManager test: test.foo.bar = %d\n", configmanager.get<int>("test.foo.bar"));

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
	bool fullscreen = true;

	// TODO: read w, h, bpp from config file to override defaults

	SDL_WM_SetCaption("NarfBlock", "NarfBlock");

	if (!init_video(w, h, bpp, fullscreen)) {
		fprintf(stderr, "Error: could not set OpenGL video mode %dx%d@%d bpp\n", w, h, bpp);
		SDL_Quit();
		return 1;
	}

	srand(0x1234);
	gen_world();

	player = new narf::Entity(world);

	// initial player position
	player->position = narf::Vector3f(15.0f, 1.0f, 10.0f);

	// initialize camera to look at origin
	cam.yaw = atan2f(cam.position.z, cam.position.x) - (float)M_PI / 2;
	cam.pitch = 0.0f;

	bouncy_block = new narf::Entity(world);
	bouncy_block->position = narf::Vector3f(10.0f, 5.0f, 10.0f);
	bouncy_block->bouncy = true;

	init_textures();

	SDL_ShowCursor(0);
	SDL_WM_GrabInput(SDL_GRAB_ON);

	game_loop();

	SDL_Quit();
	return 0;
}
