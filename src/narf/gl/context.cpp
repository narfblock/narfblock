#include "narf/gl/gl.h"
#include "narf/console.h"

#ifdef _WIN32
#include <SDL_syswm.h>
#include <windows.h>
#endif

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


bool narf::gl::Context::getFunctions() {
	// load required base (non-extension) OpenGL function
#define REQ(func) \
	{ offsetof(narf::gl::Context, func), "gl" #func }

	static const struct Func {
		size_t funcOffset;
		const char* name;
	} funcs[] = {
		// GL 1.5+
		REQ(GenBuffers),
		REQ(DeleteBuffers),
		REQ(BindBuffer),
		REQ(BufferData),

		// GL 2.0+
		REQ(CreateShader),
		REQ(DeleteShader),
		REQ(ShaderSource),
		REQ(CompileShader),
		REQ(GetShaderiv),
		REQ(GetShaderInfoLog),
		REQ(CreateProgram),
		REQ(LinkProgram),
		REQ(DeleteProgram),
		REQ(GetProgramiv),
		REQ(GetProgramInfoLog),
		REQ(AttachShader),
		REQ(DetachShader),
	};

	for (const auto& f : funcs) {
		void** fp = (void**)((uintptr_t)this + f.funcOffset);
		*fp = SDL_GL_GetProcAddress(f.name);
		if (!*fp) {
			console->println("Missing required OpenGL function " + std::string(f.name));
			return false;
		}
	}

	return true;
}


bool narf::gl::Context::setDisplayMode(const char *title, int32_t width, int32_t height, bool fullscreen)
{
	Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
	if (fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// request GL 2.1 context
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

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

	glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	glslVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	console->println("OpenGL version " + std::string(glVersion));
	console->println("GLSL version " + std::string(glslVersion));

	if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &glContextVersionMajor) == 0 &&
	    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &glContextVersionMinor) == 0) {
		auto glContextVersion = std::to_string(glContextVersionMajor) + "." + std::to_string(glContextVersionMinor);
		console->println("GL context version " + glContextVersion);
	} else {
		console->println("Could not determine GL context version");
		return false;
	}

	// require OpenGL 2.0+
	if (glContextVersionMajor < 2) {
		console->println("OpenGL 2.0+ required");
		return false;
	}

	if (!getFunctions()) {
		return false;
	}

	return true;
}


void narf::gl::Context::updateViewport() {
	int w, h;
	SDL_GL_GetDrawableSize(window_, &w, &h);
	glViewport(0, 0, w, h);
}


int32_t narf::gl::Context::width() const {
	int w, h;
	SDL_GetWindowSize(window_, &w, &h);
	return w;
}


int32_t narf::gl::Context::height() const {
	int w, h;
	SDL_GetWindowSize(window_, &w, &h);
	return h;
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
