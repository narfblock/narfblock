#include "narf/math/floats.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <float.h>

// Borrowed from
// https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
union FloatExtract {
	int32_t i;
	float f;

	FloatExtract(float num) : f(num) {}
	bool negative() const { return (i >> 31) != 0; }
	int32_t rawMantissa() const { return i & ((1 << 23) - 1); }
	int32_t rawExponent() const { return (i >> 23) & 0xFF; }
};


bool narf::almostEqualDiff(float a, float b, float maxDiff) {
	float absDiff = fabsf(a - b);
	return absDiff <= maxDiff;
}

bool narf::almostEqual(float a, float b) {
	// Supposedly 4 is good...
	return almostEqualUlps(a, b, 4);
}

bool narf::almostEqualUlps(float a, float b, int maxUlpsDiff) {
	return almostEqualUlpsAndAbs(a, b, FLT_EPSILON, maxUlpsDiff);
}

bool narf::almostEqualUlpsAndAbs(float a, float b, float maxDiff, int maxUlpsDiff) {
	// Check if the numbers are really close -- needed
	// when comparing numbers near zero.
	float absDiff = fabsf(a - b);
	if (absDiff <= maxDiff) {
		return true;
	}

	FloatExtract uA(a);
	FloatExtract uB(b);

	// Different signs means they do not match.
	if (uA.negative() != uB.negative()) {
		return false;
	}

	// Find the difference in ULPs.
	int ulpsDiff = abs(uA.i - uB.i);
	if (ulpsDiff <= maxUlpsDiff) {
		return true;
	}

	return false;
}

bool narf::almostEqualRelativeAndAbs(float a, float b, float maxDiff, float maxRelDiff) {
	// Check if the numbers are really close -- needed
	// when comparing numbers near zero.
	float diff = fabsf(a - b);
	if (diff <= maxDiff) {
		return true;
	}

	a = fabsf(a);
	b = fabsf(b);
	float largest = (b > a) ? b : a;

	if (diff <= largest * maxRelDiff) {
		return true;
	}
	return false;
}
