/*
 * NarfBlock chunk cache
 *
 * Copyright (c) 2015 Daniel Verkamp
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

#ifndef NARF_CHUNK_CACHE_H
#define NARF_CHUNK_CACHE_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#include <unordered_map>
#include <vector>

#include "narf/math/coord3D.h"

namespace narf {

/*
 * ChunkCache is a data structure for storing items indexed by coordinate.
 */
template<typename CoordType, typename StoredType>
class ChunkCache {
public:
	ChunkCache() {}
	~ChunkCache() {}

	StoredType* get(const CoordType& c) {
		auto p = storage_.find(c);
		if (p == storage_.end()) {
			return nullptr;
		}
		return &p->second;
	}

	void put(const CoordType& c, StoredType&& value) {
		storage_.emplace(c, value);
	}

	void setSize(size_t numElements) {
		// TODO: drop LRU stuff if shrinking
		storage_.reserve(numElements);
	}

private:
	std::unordered_map<CoordType, StoredType> storage_;
	// TODO: keep track of LRU
};

} // namespace narf

#endif // NARF_CHUNK_CACHE_H
