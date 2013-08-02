#include <tgmath.h>
#include "narf/math/floats.h"
#include <cmath>
#include <float.h>

bool narf::math::AlmostEqualDiff(float A, float B, float maxDiff) {
	float absDiff = (float)fabs(A - B);
	return absDiff <= maxDiff;
}

bool narf::math::AlmostEqual(float A, float B) {
	// Supposedly 4 is good...
	bool equal = narf::math::AlmostEqualUlps(A, B, 4);
	return equal;
}

bool narf::math::AlmostEqualUlps(float A, float B, int maxUlpsDiff) {
	return narf::math::AlmostEqualUlpsAndAbs(A, B, FLT_EPSILON, maxUlpsDiff);
}

bool narf::math::AlmostEqualUlpsAndAbs(float A, float B, float maxDiff, int maxUlpsDiff) {
	// Check if the numbers are really close -- needed
	// when comparing numbers near zero.
	float absDiff = (float)fabs(A - B);
	if (absDiff <= maxDiff)
		return true;

	narf::math::Float_t uA(A);
	narf::math::Float_t uB(B);

	// Different signs means they do not match.
	if (uA.Negative() != uB.Negative())
		return false;

	// Find the difference in ULPs.
	int ulpsDiff = (int)std::abs(uA.i - uB.i);
	if (ulpsDiff <= maxUlpsDiff)
		return true;

	return false;
}

bool narf::math::AlmostEqualRelativeAndAbs(float A, float B, float maxDiff, float maxRelDiff) {
	// Check if the numbers are really close -- needed
	// when comparing numbers near zero.
	float diff = (float)fabs(A - B);
	if (diff <= maxDiff)
		return true;

	A = (float)fabs(A);
	B = (float)fabs(B);
	float largest = (B > A) ? B : A;

	if (diff <= largest * maxRelDiff)
		return true;
	return false;
}
