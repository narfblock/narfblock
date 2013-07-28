#ifndef NARFBLOCK_MATH_COORD_SPHERICALF_H
#define NARFBLOCK_MATH_COORD_SPHERICALF_H

#include <math.h>
#include "narf/math/floats.h"
#include "narf/math/coord/point3f.h"

namespace narf {
	namespace math {
		namespace coord {
			class Point3f;
			class Sphericalf {
				public:
					float radius;
					float azimuth;
					float inclination;
					Sphericalf(float radius, float inclination, float azimuth) : radius(radius), azimuth(azimuth), inclination(inclination) {};
					void extend(float distance);
					void shrink(float distance);
					float distanceTo(Sphericalf& other);
					bool operator==(Sphericalf& rhs) const;
					operator narf::math::coord::Point3f () const;
			};
		}
	}
}

#endif
