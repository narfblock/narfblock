#ifndef NARFBLOCK_MATH_COORD_DISTANCE_H
#define NARFBLOCK_MATH_COORD_DISTANCE_H

#include "narf/math/coord/point3f.h"
#include "narf/math/coord/sphericalf.h"
#include <math.h>

namespace narf {
	namespace math {
		namespace coord {
			class Point3f;
			float distance(float x, float y, float z, float x2, float y2, float z2);
			float distance(Point3f A, Point3f B);
			//float distance(Sphericalf A, Sphericalf B);
		}
	}
}

#endif
