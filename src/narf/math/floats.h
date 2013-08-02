// Borrowed from
// https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/

#ifndef NARFBLOCK_MATH_FLOATS_H
#define NARFBLOCK_MATH_FLOATS_H

#include <tgmath.h>
#include <float.h>
#include <stdint.h>

namespace narf {
	namespace math {

		union Float_t {
			Float_t(float num = 0.0f) : f(num) {}
			// Portable extraction of components.
			bool Negative() const { return (i >> 31) != 0; }
			int32_t RawMantissa() const { return i & ((1 << 23) - 1); }
			int32_t RawExponent() const { return (i >> 23) & 0xFF; }

			int32_t i;
			float f;
		};

		bool AlmostEqual(float A, float B);
		bool AlmostEqualDiff(float A, float B, float maxDiff);
		bool AlmostEqualUlps(float A, float B, int maxUlpsDiff);
		bool AlmostEqualUlpsAndAbs(float A, float B, float maxDiff, int maxUlpsDiff);
		bool AlmostEqualRelativeAndAbs(float A, float B, float maxDiff, float maxRelDiff);
	}
}

#endif
