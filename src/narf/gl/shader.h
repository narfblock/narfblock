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

	Shader(Context& gl, ShaderType type) : gl(gl) {
		id = gl.CreateShader(static_cast<GLenum>(type));
	}

	~Shader() {
		gl.DeleteShader(id);
	}

	bool compile(const void* data, size_t size) {
		auto dataCharPtr = static_cast<const GLchar*>(data);
		auto lengthInt = static_cast<GLint>(size);
		gl.ShaderSource(id, 1, &dataCharPtr, &lengthInt);
		gl.CompileShader(id);
		GLint status;
		gl.GetShaderiv(id, GL_COMPILE_STATUS, &status);
		return status != GL_FALSE;
	}

	std::string infoLog() {
		GLint len;
		gl.GetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
		auto info = new GLchar[len + 1];
		if (info) {
			gl.GetShaderInfoLog(id, len, nullptr, info);
			info[len] = '\0';
			std::string s(info);
			delete[] info;
			return s;
		}

		return "error retrieving info log";
	}

private:
	Context& gl;
};

class Program {
public:
	GLuint id;

	Program(Context& gl) : gl(gl) {
		id = gl.CreateProgram();
	}

	void attach(Shader& s) {
		gl.AttachShader(id, s.id);
	}

	void detach(Shader& s) {
		gl.DetachShader(id, s.id);
	}

	bool link() {
		gl.LinkProgram(id);
		GLint status;
		gl.GetProgramiv(id, GL_LINK_STATUS, &status);
		return status != GL_FALSE;
	}

	std::string infoLog() {
		GLint len;
		gl.GetProgramiv(id, GL_INFO_LOG_LENGTH, &len);
		auto info = new GLchar[len + 1];
		if (info) {
			gl.GetProgramInfoLog(id, len, nullptr, info);
			info[len] = '\0';
			std::string s(info);
			delete[] info;
			return s;
		}

		return "error retrieving info log";
	}

	~Program() {
		gl.DeleteProgram(id);
	}

private:
	Context& gl;
};

} // namespace gl
} // namespace narf

#endif // NARF_GL_SHADER_H
