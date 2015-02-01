#ifndef NARFBLOCK_MATH_INTS_H
#define NARFBLOCK_MATH_INTS_H

#include <stdint.h>

namespace narf {
	uint32_t ilog2(uint32_t v);
	int32_t ilog2(int32_t v);

	static inline int32_t clampi(int32_t v, int32_t minV, int32_t maxV) {
		if (v <= minV) {
			return minV;
		} else if (v > maxV) {
			return maxV;
		} else {
			return v;
		}
	}

	static inline bool almostEqual(int a, int b) {
		return a == b;
	}
}

#endif
