#include "narf/math/coord/point3f.h"
#include "narf/math/coord/distance.h"

bool narf::math::coord::Point3f::operator==(Point3f& rhs) const {
	return AlmostEqual(x, rhs.x) && AlmostEqual(y, rhs.y) && AlmostEqual(z, rhs.z);
}

narf::math::coord::Point3f::operator narf::math::coord::Sphericalf () const {
	float radius = distanceTo(0, 0, 0);
	float inclination = !narf::math::AlmostEqual(radius, 0) ? acos(z/radius) : 0;
	float azimuth = !narf::math::AlmostEqual(x, 0) ? atan2(y, x) : 0;
	return narf::math::coord::Sphericalf(radius, inclination, azimuth);
}

float narf::math::coord::Point3f::distanceTo(Point3f other) const {
	return narf::math::coord::distance(x, y, z, other.x, other.y, other.z);
}

float narf::math::coord::Point3f::distanceTo(float x2, float y2, float z2) const {
	return narf::math::coord::distance(x, y, z, x2, y2, z2);
}
