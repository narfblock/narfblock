#ifndef NARFBLOCK_MATH_FLOATS_H
#define NARFBLOCK_MATH_FLOATS_H

#include <cmath>

namespace narf {
	bool almostEqual(float a, float b);
	bool almostEqualDiff(float a, float b, float maxDiff);
	bool almostEqualUlps(float a, float b, int maxUlpsDiff);
	bool almostEqualUlpsAndAbs(float a, float b, float maxDiff, int maxUlpsDiff);
	bool almostEqualRelativeAndAbs(float a, float b, float maxDiff, float maxRelDiff);
}

#endif
