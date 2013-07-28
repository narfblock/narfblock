#ifndef NARFBLOCK_MATH_COORD_POINT3F_H
#define NARFBLOCK_MATH_COORD_POINT3F_H

#include "narf/math/coord/sphericalf.h"
#include "narf/math/coord/distance.h"
#include <math.h>

namespace narf {
	namespace math {
		namespace coord {
			class Sphericalf;
			class Point3f {
				public:
					float x;
					float y;
					float z;
					Point3f(float x, float y, float z) : x(x), y(y), z(z) {};
					bool operator==(Point3f& rhs) const;
					operator Sphericalf () const;
					float distanceTo(Point3f other) const;
					float distanceTo(float x2, float y2, float z2) const;
			};
		}
	}
}

#endif
