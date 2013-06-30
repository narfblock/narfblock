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

// TODO: this is all hacky test code - refactor into nicely modularized code

class Display {
public:
	SDL_Surface *surf;

	int width;
	int height;
};


class Camera {
public:
	// position
	float x;
	float y;
	float z;

	// view angles in radians
	float yaw;
	float pitch;
};

class Block {
public:
	uint8_t type; // TODO
	unsigned solid:1;
};

Display display;
Camera cam;

const float movespeed = 0.1f;

Block *world;

#define WORLD_X_MAX	100
#define WORLD_Y_MAX 100
#define WORLD_Z_MAX 100

SDL_Surface *tiles_surf;
GLuint tiles_tex;

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
	Uint32 flags = SDL_HWSURFACE | SDL_OPENGL;
	if (fullscreen) {
		flags |= SDL_FULLSCREEN;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	display.surf = SDL_SetVideoMode(w, h, bpp, flags);
	if (!display.surf) {
		return false;
	}

	display.width = w;
	display.height = h;

	glViewport(0, 0, w, h);

	// viewer projection
	glMatrixMode(GL_PROJECTION);
	float fovx = 90.0f; // degrees
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
	if (type == 3) {
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


GLuint upload_texture(SDL_Surface *surf)
{
	// TODO: check power of two width and height

	// Create the target alpha surface with correct color component ordering
	SDL_Surface *surf_copy = SDL_CreateRGBSurface(
		SDL_SWSURFACE, surf->w, surf->h, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN // OpenGL RGBA masks 
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000
#else
		0xFF000000,
		0x00FF0000,
		0x0000FF00,
		0x000000FF
#endif
		);

	if (!surf_copy) {
		return -1; // TODO!
	}

	// copy the original surface into surf_copy to convert formats
	SDL_BlitSurface(surf, NULL, surf_copy, NULL);

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE_SGIS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE_SGIS);

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf_copy->w, surf_copy->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf_copy->pixels);

	SDL_FreeSurface(surf_copy);

	return tex;
}


bool init_textures()
{
	tiles_surf = IMG_Load("../data/terrain.png");
	assert(tiles_surf); // TODO: user-friendly message

	tiles_tex = upload_texture(tiles_surf);
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
	glTranslatef(-cam.x, -cam.y, -cam.z);

	// draw blocks
	glBindTexture(GL_TEXTURE_2D, tiles_tex);
	for (int z = 0; z < WORLD_Z_MAX; z++) {
		for (int y = 0; y < WORLD_Y_MAX; y++) {
			for (int x = 0; x < WORLD_X_MAX; x++) {
				Block *b = get_block(x, y, z);
				if (b->solid) {
					glPushMatrix();
					glTranslatef(x, y, z);
					// TODO: don't render sides of the cube that are obscured by other solid cubes?
					draw_cube(b->type, false);

					// TODO: highlight cube selected by mouse
					//draw_cube(0.0f, 0.0f, 0.0f, true);

					glPopMatrix();
				}
			}
		}
	}

	SDL_GL_SwapBuffers();
}


void event_loop()
{
	SDL_Event e;
	bool firstmove = true;

	while (1) {
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				return; // exit event loop

			case SDL_KEYDOWN:
				switch (e.key.keysym.sym) {
				case SDLK_ESCAPE:
					return; // exit event loop

			// TODO: move all this movement code into a time-dependent movement function
			// TODO: decouple player direction and camera direction

				case SDLK_w: // move forward
					cam.x -= cosf(cam.yaw + M_PI / 2) * movespeed;
					cam.z -= sinf(cam.yaw + M_PI / 2) * movespeed;
					break;

				case SDLK_s: // move backward
					cam.x += cosf(cam.yaw + M_PI / 2) * movespeed;
					cam.z += sinf(cam.yaw + M_PI / 2) * movespeed;
					break;

				case SDLK_a: // strafe left
					cam.x -= cosf(cam.yaw) * movespeed;
					cam.z -= sinf(cam.yaw) * movespeed;
					break;

				case SDLK_d: // strafe right
					cam.x += cosf(cam.yaw) * movespeed;
					cam.z += sinf(cam.yaw) * movespeed;
					break;

				case SDLK_SPACE: // jump
					cam.y += 3.0f; // more like jetpack... move to time-dependent function and check for solid ground before jumping
					break;
				}

				// TODO: other keyboard events
				break;

			case SDL_MOUSEMOTION:
				if (firstmove) {
					// ignore the initial motion event when the mouse is centered
					firstmove = false;
				} else {
					cam.yaw += ((float)e.motion.xrel / (float)display.width);
					cam.yaw = fmodf(cam.yaw, M_PI * 2.0f);

					cam.pitch += ((float)e.motion.yrel / (float)display.height);
					cam.pitch = clampf(cam.pitch, -M_PI / 2, M_PI / 2);
					break;
				}
			}
		}

		cam.y -= 1.0f; // crappy framerate-dependent gravity
		if (cam.y < 3.0f) {
			cam.y = 3.0f;
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
			b->type = 0;
			b->solid = 1;
		}
	}

	// generate some random blocks above the ground
	for (int i = 0; i < 1000; i++) {
		int x = randi(0, WORLD_X_MAX - 1);
		int y = randi(1, 10);
		int z = randi(0, WORLD_Z_MAX - 1);
		Block *b = get_block(x, y, z);
		b->type = randi(0, 3);
		b->solid = 1;
	}

	for (int i = 0; i < 10; i++) {
		Block *b = get_block(5 + i, 1, 5);
		b->type = 16;
		b->solid = 1;
		b = get_block(5, 1, 5 + i);
		b->type = 16;
		b->solid = 1;
		b = get_block(5 + i, 1, 15);
		b->type = 16;
		b->solid = 1;
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
	cam.x = 15.0f;
	cam.y = 3.0f;
	cam.z = 10.0f;

	// initialize camera to look at origin
	cam.yaw = atan2f(cam.z, cam.x) - M_PI / 2;
	cam.pitch = 0.0f;

	srand(0x1234);
	gen_world();

	init_textures();

	SDL_ShowCursor(0);
	SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_EnableKeyRepeat(1, 1); // hack

	event_loop();

	SDL_Quit();
	return 0;
}
