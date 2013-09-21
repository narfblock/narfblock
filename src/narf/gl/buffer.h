/*
 * OpenGL buffer object management
 *
 * Copyright (c) 2013 Daniel Verkamp
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NARF_GL_BUFFER_H
#define NARF_GL_BUFFER_H

#include <vector>

#include "narf/gl/gl.h"

namespace narf {
namespace gl {

template <typename T>
class Buffer {
public:

	Buffer(GLenum target, GLenum usage) : target_(target), usage_(usage)
	{
		glGenBuffers(1, &name_);
	}

	~Buffer()
	{
		glDeleteBuffers(1, &name_);
	}

	void clear()
	{
		data_.clear();
	}

	void append(const T& obj)
	{
		data_.push_back(obj);
	}

	void bind()
	{
		glBindBuffer(target_, name_);
	}

	void unbind()
	{
		glBindBuffer(target_, 0);
	}

	void upload()
	{
		bind();
		glBufferData(target_, data_.size() * sizeof(T), data_.data(), usage_);
		unbind();
	}

	const T *data()
	{
		return data_.data();
	}

	size_t count()
	{
		return data_.size();
	}

	bool empty() {
		return data_.empty();
	}

private:
	GLenum target_;
	GLenum usage_;
	GLuint name_;
	std::vector<T> data_;

};

} // namespace gl
} // namespace narf

#endif // NARF_GL_BUFFER_H
