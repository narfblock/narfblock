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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "narf/bytestream.h"

narf::ByteStreamWriter::ByteStreamWriter() {
}


narf::ByteStreamWriter::~ByteStreamWriter() {
}


// TODO: use vector's resize() and write directly to data if this turns out to be slow
void narf::ByteStreamWriter::writeLE(uint16_t v) {
	data_.push_back(static_cast<uint8_t>(v));
	data_.push_back(static_cast<uint8_t>(v >> 8));
}


void narf::ByteStreamWriter::writeLE(uint32_t v) {
	data_.push_back(static_cast<uint8_t>(v));
	data_.push_back(static_cast<uint8_t>(v >> 8));
	data_.push_back(static_cast<uint8_t>(v >> 16));
	data_.push_back(static_cast<uint8_t>(v >> 24));
}


void narf::ByteStreamWriter::writeLE(float v) {
	// TODO: total hax
	writeLE(*reinterpret_cast<uint32_t*>(&v));
}


narf::ByteStreamReader::ByteStreamReader(size_t size) {
	data_ = new uint8_t[size];
	if (data_ != nullptr) {
		size_ = size;
	} else {
		size_ = 0;
	}
	reset();
}


narf::ByteStreamReader::ByteStreamReader(const void* data, size_t size) :
	ByteStreamReader(size) {
	if (data_ != nullptr) {
		memcpy(data_, data, size);
	}
}


narf::ByteStreamReader::~ByteStreamReader() {
	if (data_ != nullptr) {
		delete[] data_;
	}
}


bool narf::ByteStreamReader::readLE(uint16_t* v) {
	assert(v != nullptr);
	if (bytesLeft_ < 2) {
		return false;
	}
	*v = static_cast<uint16_t>(
		static_cast<uint16_t>(iter_[0]) |
		static_cast<uint16_t>(iter_[1]) << 8);
	iter_ += 2;
	bytesLeft_ -= 2;
	return true;
}


bool narf::ByteStreamReader::readLE(uint32_t* v) {
	if (bytesLeft_ < 4) {
		return false;
	}
	*v = static_cast<uint32_t>(
		static_cast<uint32_t>(iter_[0]) |
		static_cast<uint32_t>(iter_[1]) << 8 |
		static_cast<uint32_t>(iter_[2]) << 16 |
		static_cast<uint32_t>(iter_[3]) << 24);
	iter_ += 4;
	bytesLeft_ -= 4;
	return true;
}


bool narf::ByteStreamReader::readLE(float* v) {
	// TODO: total hax
	return readLE(reinterpret_cast<uint32_t*>(v));
}
