#include "narf/gl/context.h"
#include "narf/console.h"

#ifdef _WIN32
#include <SDL_syswm.h>
#include <windows.h>
#endif

narf::gl::Context::Context()
{
}


bool narf::gl::Context::setDisplayMode(const char *title, int width, int height, bool fullscreen)
{
	Uint32 flags = SDL_WINDOW_OPENGL;
	if (fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
	if (!window_) {
		console->println("ERROR: setDisplayMode: SDL_CreateWindow failed: " + std::string(SDL_GetError()));
		return false;
	}

	context_ = SDL_GL_CreateContext(window_);
	if (!context_) {
		console->println("ERROR: setDisplayMode: SDL_GL_CreateContext failed: " + std::string(SDL_GetError()));
		return false;
	}

	// disable vsync
	// TODO: make this a configuration option
	if (SDL_GL_SetSwapInterval(0)) {
		console->println("WARNING: setDisplayMode: SDL_GL_SetSwapInterval failed: " + std::string(SDL_GetError()));
	}

	GLenum glew_err = glewInit();
	if (glew_err != GLEW_OK) {
		console->println("Error initializing GLEW: " + std::string((const char *)glewGetErrorString(glew_err)));
		return false;
	}

	glViewport(0, 0, width, height);

	// set window icon
#ifdef _WIN32
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (SDL_GetWindowWMInfo(window_, &info)) {
		HWND hwnd = info.info.win.window;
		HINSTANCE inst = GetModuleHandle(NULL);
		HANDLE bigIcon   = LoadImage(inst, MAKEINTRESOURCE(1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
		HANDLE smallIcon = LoadImage(inst, MAKEINTRESOURCE(1), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
		if (bigIcon) {
			SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)bigIcon);
		}
		if (smallIcon) {
			SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)smallIcon);
		}
	}
#else
	// TODO use SDL_SetWindowIcon
#endif

	return true;
}
