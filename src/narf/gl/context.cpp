#include "narf/gl/context.h"
#include "narf/console.h"

#ifdef _WIN32
#include <SDL_syswm.h>
#include <windows.h>
#endif

narf::gl::Context::Context()
{
}


void narf::gl::Context::setVsync(bool enabled) {
	int rc;
	if (enabled) {
		// try late swap tearing; if it fails, enable regular vsync
		rc = SDL_GL_SetSwapInterval(-1);
		if (rc == -1) {
			console->println("WARNING: late swap tearing not supported; using regular vsync");
			rc = SDL_GL_SetSwapInterval(1);
		}
	} else {
		rc = SDL_GL_SetSwapInterval(0);
	}

	if (rc != 0) {
		console->println("WARNING: SDL_GL_SetSwapInterval failed: " + std::string(SDL_GetError()));
	}
}


bool narf::gl::Context::setDisplayMode(const char *title, uint32_t width, uint32_t height, bool fullscreen)
{
	Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
	if (fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

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

	GLenum glew_err = glewInit();
	if (glew_err != GLEW_OK) {
		console->println("Error initializing GLEW: " + std::string((const char *)glewGetErrorString(glew_err)));
		return false;
	}

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


void narf::gl::Context::updateViewport() {
	int w, h;
	SDL_GL_GetDrawableSize(window_, &w, &h);
	glViewport(0, 0, w, h);
}


uint32_t narf::gl::Context::width() const {
	int w, h;
	SDL_GetWindowSize(window_, &w, &h);
	return static_cast<uint32_t>(w);
}


uint32_t narf::gl::Context::height() const {
	int w, h;
	SDL_GetWindowSize(window_, &w, &h);
	return static_cast<uint32_t>(h);
}


void narf::gl::Context::toggleFullscreen() {
	Uint32 flags = SDL_GetWindowFlags(window_);
	bool curFullscreen = (flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP)) != 0;
	int ret;
	if (curFullscreen) {
		ret = SDL_SetWindowFullscreen(window_, 0);
	} else {
		ret = SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
	if (ret != 0) {
		console->println("WARNING: SDL_SetWindowFullscreen failed: " + std::string(SDL_GetError()));
	}
}
