#include "narf/math/coord/distance.h"

float narf::math::coord::distance(float x, float y, float z, float x2, float y2, float z2) {
	float a = (x - x2) * (x - x2);
	float b = (y - y2) * (y - y2);
	float c = (z - z2) * (z - z2);
	return sqrt(a + b + c);
}

float narf::math::coord::distance(narf::math::coord::Point3f A, narf::math::coord::Point3f B) {
	return A.distanceTo(B);
}
