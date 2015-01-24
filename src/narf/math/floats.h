#ifndef NARFBLOCK_MATH_FLOATS_H
#define NARFBLOCK_MATH_FLOATS_H

#include <math.h>

namespace narf {
	bool almostEqual(float a, float b);
	bool almostEqualDiff(float a, float b, float maxDiff);
	bool almostEqualUlps(float a, float b, int maxUlpsDiff);
	bool almostEqualUlpsAndAbs(float a, float b, float maxDiff, int maxUlpsDiff);
	bool almostEqualRelativeAndAbs(float a, float b, float maxDiff, float maxRelDiff);

	// single-precision overloads of double-precision function names
	static inline float sin(float f) { return ::sinf(f); }
	static inline float cos(float f) { return ::cosf(f); }
	static inline float acos(float f) { return ::acosf(f); }
	static inline float atan2(float y, float x) { return ::atan2f(y, x); }
}

#endif
