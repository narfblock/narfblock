#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_image.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <math.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "test.h"

#include "narf/input.h"
#include "narf/vector.h"

#include "narf/gl/gl.h"

// TODO: this is all hacky test code - refactor into nicely modularized code

class Camera {
public:

	Camera() : position(0.0f, 0.0f, 0.0f) { }

	narf::Vector3f position;

	// view angles in radians
	float yaw;
	float pitch;
};

class Block {
public:
	uint8_t id;
};

Camera cam;

const float meter = 1.0;
const float second = 1000.0; // native time unit is milliseconds

const float meters_per_second = meter * (1 / second);

const float movespeed = 25.0f * meters_per_second;

Block *world;

#define WORLD_X_MAX 100
#define WORLD_Y_MAX 100
#define WORLD_Z_MAX 100

SDL_Surface *tiles_surf;

narf::gl::Context *display;
narf::gl::Texture *tiles_tex;

Block *get_block(int x, int y, int z)
{
	assert(x >= 0 && y >= 0 && z >= 0);
	assert(x < WORLD_X_MAX && y < WORLD_Y_MAX && z < WORLD_Z_MAX);

	return &world[z * WORLD_X_MAX * WORLD_Y_MAX + y * WORLD_X_MAX + x];
}

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

	return true;
}


void draw_quad(uint8_t tex_id, bool outline, const float *quad)
{
	uint8_t tex_x = tex_id % 16;
	uint8_t tex_y = tex_id / 16;

	float texcoord_tile_size = 1.0f / 16.0f;

	float u1 = (float)tex_x * texcoord_tile_size;
	float v1 = (float)tex_y * texcoord_tile_size;
	float u2 = u1 + texcoord_tile_size;
	float v2 = v1 + texcoord_tile_size;

	// TODO: get rid of GL_QUADS, use triangle strip?
	glBegin(outline ? GL_LINE_LOOP : GL_QUADS);

	glTexCoord2f(u1, v2);
	glVertex3fv(&quad[0 * 3]);

	glTexCoord2f(u2, v2);
	glVertex3fv(&quad[1 * 3]);

	glTexCoord2f(u2, v1);
	glVertex3fv(&quad[2 * 3]);

	glTexCoord2f(u1, v1);
	glVertex3fv(&quad[3 * 3]);

	glEnd();
}


void draw_cube(uint8_t type, bool outline)
{
	static const float cube_quads[][4*3] = {
		{0,0,0, 1,0,0, 1,1,0, 0,1,0},
		{0,0,1, 1,0,1, 1,1,1, 0,1,1},
		{0,0,0, 1,0,0, 1,0,1, 0,0,1},
		{0,1,0, 1,1,0, 1,1,1, 0,1,1},
		{0,0,0, 0,0,1, 0,1,1, 0,1,0},
		{1,0,0, 1,0,1, 1,1,1, 1,1,0}
	};

	uint8_t tex_id_top, tex_id_side, tex_id_bot;
	if (type == 2) {
		tex_id_top = 0;
		tex_id_side = 3;
		tex_id_bot = 2;
	} else {
		tex_id_top = tex_id_side = tex_id_bot = type;
	}

	draw_quad(tex_id_side, outline, cube_quads[0]);
	draw_quad(tex_id_side, outline, cube_quads[1]);
	draw_quad(tex_id_bot,  outline, cube_quads[2]);
	draw_quad(tex_id_top,  outline, cube_quads[3]);
	draw_quad(tex_id_side, outline, cube_quads[4]);
	draw_quad(tex_id_side, outline, cube_quads[5]);
}


bool init_textures()
{
	tiles_surf = IMG_Load("../data/terrain.png");
	assert(tiles_surf); // TODO: user-friendly message

	tiles_tex = new narf::gl::Texture(display);
	if (!tiles_tex->upload(tiles_surf)) {
		assert(0);
		return false;
	}

	return true;
}


void draw()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// camera
	glLoadIdentity();
	glRotatef(cam.pitch * 180.0f / M_PI, 1.0f, 0.0f, 0.0f);
	glRotatef(cam.yaw * 180.0f / M_PI,   0.0f, 1.0f, 0.0f);
	glTranslatef(-cam.position.x, -cam.position.y, -cam.position.z);

	// draw blocks
	glBindTexture(narf::gl::TEXTURE_2D, tiles_tex);
	for (int z = 0; z < WORLD_Z_MAX; z++) {
		for (int y = 0; y < WORLD_Y_MAX; y++) {
			for (int x = 0; x < WORLD_X_MAX; x++) {
				Block *b = get_block(x, y, z);
				if (b->id) {
					glPushMatrix();
					glTranslatef(x, y, z);
					// TODO: don't render sides of the cube that are obscured by other solid cubes?
					draw_cube(b->id, false);

					// TODO: highlight cube selected by mouse
					//draw_cube(0.0f, 0.0f, 0.0f, true);

					glPopMatrix();
				}
			}
		}
	}

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
	cam.yaw = fmodf(cam.yaw, M_PI * 2.0f);

	cam.pitch += input.look_rel().y;
	cam.pitch = clampf(cam.pitch, -M_PI / 2, M_PI / 2);

	narf::Vector3f vel_rel(0.0f, 0.0f, 0.0f);

	if (input.move_forward()) {
		vel_rel -= narf::Vector3f(cosf(cam.yaw + M_PI / 2), 0.0f, sinf(cam.yaw + M_PI / 2));
	} else if (input.move_backward()) {
		vel_rel += narf::Vector3f(cosf(cam.yaw + M_PI / 2), 0.0f, sinf(cam.yaw + M_PI / 2));
	}

	if (input.strafe_left()) {
		vel_rel -= narf::Vector3f(cosf(cam.yaw), 0.0f, sinf(cam.yaw));
	} else if (input.strafe_right()) {
		vel_rel += narf::Vector3f(cosf(cam.yaw), 0.0f, sinf(cam.yaw));
	}

	// normalize so that diagonal movement is not faster than cardinal directions
	vel_rel = vel_rel.normalize();

	if (input.jump()) {
		vel_rel += narf::Vector3f(0.0f, 15.0f, 0.0f);
	}

	vel_rel += narf::Vector3f(0.0f, -1.0f, 0.0f); // crappy framerate-dependent gravity

	cam.position += vel_rel * movespeed * dt;

	if (cam.position.y < 3.0f) {
		cam.position.y = 3.0f;
	}
}


double get_time_ms()
{
	return (double)SDL_GetTicks(); // SDL tick is a millisecond
}


void game_loop()
{
	narf::Input input(display->width(), display->height());
	double t = 0.0;
	double t1 = get_time_ms();

	const double physics_tick_step = 8.3333333333333333333333333333333; // fixed time step
	const double max_frame_time = 250.0;

	double t_accum = 0.0;

	while (1) {
		double t2 = get_time_ms();
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
			t_accum -= physics_tick_step;
			t += physics_tick_step;
		}

		draw();
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
	world = (Block*)calloc(WORLD_X_MAX * WORLD_Y_MAX * WORLD_Z_MAX, sizeof(Block));

	// first fill a plane at y = 0
	for (int z = 0; z < WORLD_Z_MAX; z++) {
		for (int x = 0; x < WORLD_X_MAX; x++) {
			Block *b = get_block(x, 0, z);
			b->id = 2; // grass
		}
	}

	// generate some random blocks above the ground
	for (int i = 0; i < 1000; i++) {
		int x = randi(0, WORLD_X_MAX - 1);
		int y = randi(1, 10);
		int z = randi(0, WORLD_Z_MAX - 1);
		Block *b = get_block(x, y, z);
		b->id = randi(1, 3);
	}

	for (int i = 0; i < 10; i++) {
		Block *b = get_block(5 + i, 1, 5);
		b->id = 16;
		b = get_block(5, 1, 5 + i);
		b->id = 16;
		b = get_block(5 + i, 1, 15);
		b->id = 16;
	}
}


extern "C" int main(int argc, char **argv)
{
	printf("Version: %d.%d%s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE);

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

	// initial camera position
	cam.position = narf::Vector3f(15.0f, 3.0f, 10.0f);

	// initialize camera to look at origin
	cam.yaw = atan2f(cam.position.z, cam.position.x) - M_PI / 2;
	cam.pitch = 0.0f;

	srand(0x1234);
	gen_world();

	init_textures();

	SDL_ShowCursor(0);
	SDL_WM_GrabInput(SDL_GRAB_ON);

	game_loop();

	SDL_Quit();
	return 0;
}
