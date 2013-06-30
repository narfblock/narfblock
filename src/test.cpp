#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <stdlib.h>
#include <stdio.h>
#include "test.h"

// TODO: this is all hacky test code - refactor into nicely modularized code

SDL_Surface *display_surf;


bool init_video(int w, int h, int bpp, bool fullscreen)
{
	Uint32 flags = SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL;
	if (fullscreen) {
		flags |= SDL_FULLSCREEN;
	}

	display_surf = SDL_SetVideoMode(w, h, bpp, flags);
	if (!display_surf) {
		return false;
	}

	glClearColor(0, 0.0f, 0, 0);
	glClearDepth(1.0f);

	glViewport(0, 0, w, h);

	// viewer projection
	glMatrixMode(GL_PROJECTION);
	gluPerspective(20, 1, 0.1, 20);

	// viewer position
	glMatrixMode(GL_MODELVIEW);
	glTranslatef(0, 0, -20);

	// object position
	glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(30.0f, 0.0f, 1.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);

	// for drawing outlines
	glPolygonOffset(1.0, 2);
	glEnable(GL_POLYGON_OFFSET_FILL);

	return true;
}


void draw_quad(float r, float g, float b, bool outline, const float *quad)
{
	// TODO: get rid of GL_QUADS, use triangle strip?
	glBegin(outline ? GL_LINE_LOOP : GL_QUADS);
	glColor3f(r, g, b);
	glVertex3fv(&quad[0 * 3]);
	glVertex3fv(&quad[1 * 3]);
	glVertex3fv(&quad[2 * 3]);
	glVertex3fv(&quad[3 * 3]);
	glEnd();
}


void draw_cube(float r, float g, float b, bool outline)
{
	static const float cube_quads[][4*3] = {
		{0,0,0, 1,0,0, 1,1,0, 0,1,0},
		{0,0,1, 1,0,1, 1,1,1, 0,1,1},
		{0,0,0, 1,0,0, 1,0,1, 0,0,1},
		{0,1,0, 1,1,0, 1,1,1, 0,1,1},
		{0,0,0, 0,0,1, 0,1,1, 0,1,0},
		{1,0,0, 1,0,1, 1,1,1, 1,1,0}
	};

	for (int i = 0; i < 6; i++) {
		draw_quad(r, g, b, outline, cube_quads[i]);
	}
}


void draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int z = 0; z < 3; z++) {
		for (int y = 0; y < 3; y++) {
			for (int x = 0; x < 3; x++) {

				glPushMatrix();
				glTranslatef(x, y, z);

				draw_cube(1.0f, 0.0f, 0.0f, false);
				draw_cube(1.0f, 1.0f, 1.0f, true);

				glPopMatrix();
			}
		}
	}

	SDL_GL_SwapBuffers();
}


void event_loop()
{
	SDL_Event e;

    while (1) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
				return; // exit event loop
			}

			if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_ESCAPE) {
					return; // exit event loop
				}
				// TODO: other keyboard events
			}
        }

        draw();
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

	event_loop();

	SDL_Quit();
	return 0;
}
