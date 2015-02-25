#ifndef NARF_GL_SHADER_H
#define NARF_GL_SHADER_H

#include "narf/gl/gl.h"

#include <string>
#include <cstdlib>

namespace narf {
namespace gl {

class Context; // forward decl


enum class ShaderType {
	Vertex = GL_VERTEX_SHADER,
	Geometry = GL_GEOMETRY_SHADER,
	Fragment = GL_FRAGMENT_SHADER,
};

class Shader {
public:
	GLuint id;

	Shader(Context* context, ShaderType type) {
		id = glCreateShader(static_cast<GLenum>(type));
	}

	~Shader() {
		glDeleteShader(id);
	}

	bool compile(const void* data, size_t size) {
		auto dataCharPtr = static_cast<const GLchar*>(data);
		auto lengthInt = static_cast<GLint>(size);
		glShaderSource(id, 1, &dataCharPtr, &lengthInt);
		glCompileShader(id);
		GLint status;
		glGetShaderiv(id, GL_COMPILE_STATUS, &status);
		return status != GL_FALSE;
	}

	std::string infoLog() {
		GLint len;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
		auto info = new GLchar[len + 1];
		if (info) {
			glGetShaderInfoLog(id, len, nullptr, info);
			info[len] = '\0';
			std::string s(info);
			delete[] info;
			return s;
		}

		return "error retrieving info log";
	}
};

class Program {
public:
	GLuint id;

	Program(Context* context) {
		id = glCreateProgram();
	}

	void attach(Shader& s) {
		glAttachShader(id, s.id);
	}

	void detach(Shader& s) {
		glDetachShader(id, s.id);
	}

	bool link() {
		glLinkProgram(id);
		GLint status;
		glGetProgramiv(id, GL_LINK_STATUS, &status);
		return status != GL_FALSE;
	}

	std::string infoLog() {
		GLint len;
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &len);
		auto info = new GLchar[len + 1];
		if (info) {
			glGetProgramInfoLog(id, len, nullptr, info);
			info[len] = '\0';
			std::string s(info);
			delete[] info;
			return s;
		}

		return "error retrieving info log";
	}

	~Program() {
		glDeleteProgram(id);
	}
};

} // namespace gl
} // namespace narf

#endif // NARF_GL_SHADER_H
