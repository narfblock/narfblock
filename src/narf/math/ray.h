#ifndef NARF_MATH_RAY_H
#define NARF_MATH_RAY_H

#include "narf/math/coord3D.h"
#include "narf/math/vector.h"

namespace narf {
	namespace math {

		// a ray in 3D space defined by an initial point and a direction
		template<class T>
		class Ray {
		private:
			coord::Point3<T> point_; // initial point
			Vector3<T> direction_; // direction (unit vector)
		public:
			Ray(coord::Point3<T> point, Vector3<T> direction) : point_(point), direction_(direction.normalize()) { }

			coord::Point3<T> initialPoint() const {
				return point_;
			}

			Vector3<T> direction() const {
				return direction_;
			}

			coord::Point3<T> pointAtDistance(T distance) const {
				return point_ + direction_ * distance;
			}
		};
	}
}

#endif // NARF_MATH_RAY_H
