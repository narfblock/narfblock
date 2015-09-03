/*
 * NarfBlock bytestream writer and reader
 *
 * Copyright (c) 2014 Daniel Verkamp
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

#ifndef NARF_BYTESTREAM_H
#define NARF_BYTESTREAM_H

#include <stdint.h>
#include <stddef.h>
#include <vector>

namespace narf {

// TODO: wrap this in a class
class ByteStreamWriter {
public:
	ByteStreamWriter();
	~ByteStreamWriter();

	void write(uint8_t v);
	void writeLE(uint16_t v);
	void writeLE(uint32_t v);
	void writeLE(int32_t v);
	void writeLE(float v);

	void* data() { return data_.data(); }
	size_t size() { return data_.size(); }

private:
	std::vector<uint8_t> data_;
};


class ByteStreamReader {
public:
	ByteStreamReader(const void* data, size_t size); // copy from data
	ByteStreamReader(size_t size); // reserve space and fill via data()
	~ByteStreamReader();

	void reset() { iter_ = data_; bytesLeft_ = size_; }

	void* data() { return data_; }

	size_t size() const { return size_; }
	size_t bytesLeft() const { return bytesLeft_; }

	bool read(uint8_t* v);
	bool readLE(uint16_t* v);
	bool readLE(uint32_t* v);
	bool readLE(int32_t* v);
	bool readLE(float* v);

private:
	uint8_t* data_;
	const uint8_t* iter_;
	size_t size_;
	size_t bytesLeft_;
};

} // namespace narf

#endif // NARF_BYTESTREAM_H
