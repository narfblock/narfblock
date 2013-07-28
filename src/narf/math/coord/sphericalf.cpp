#include "narf/math/coord/sphericalf.h"

void narf::math::coord::Sphericalf::extend(float distance) {
	radius += distance;
}

void narf::math::coord::Sphericalf::shrink(float distance) {
	radius -= distance;
}

float narf::math::coord::Sphericalf::distanceTo(Sphericalf& other) {
	float rel_angle_cos = sin(inclination) * sin(other.inclination) + cos(inclination) * cos(other.inclination) * cos(azimuth - other.azimuth);
	return static_cast<float>(sqrt(radius * radius + other.radius * other.radius - 2 * radius * other.radius * rel_angle_cos));
}

bool narf::math::coord::Sphericalf::operator==(Sphericalf& rhs) const {
	return narf::math::AlmostEqual(radius, rhs.radius) &&
		narf::math::AlmostEqual(inclination, rhs.inclination) &&
		narf::math::AlmostEqual(azimuth, rhs.azimuth);
}

narf::math::coord::Sphericalf::operator narf::math::coord::Point3f () const {
	float x = radius * sin(inclination) * cos(azimuth);
	float y = radius * sin(inclination) * sin(azimuth);
	float z = radius * cos(inclination);
	return narf::math::coord::Point3f(x, y, z);
}

