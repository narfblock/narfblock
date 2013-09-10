#ifndef NARF_MATH_RAY_H
#define NARF_MATH_RAY_H

#include "narf/math/coord3D.h"
#include "narf/math/vector.h"
#include "narf/math/orientation.h"

namespace narf {
	namespace math {

		// a ray in 3D space defined by an initial point and a direction
		template<class T>
		class Ray {
		private:
			coord::Point3<T> point_; // initial point
			Vector3<T> direction_; // direction (unit vector)
		public:
			Ray(Vector3<T> direction) : point_(coord::Point3<T>(0, 0, 0)), direction_(direction.normalize()) { }
			Ray(Orientation<T> direction) : point_(coord::Point3<T>(0, 0, 0)), direction_(direction) { }
			Ray(coord::Point3<T> point, const Vector3<T> direction) : point_(point), direction_(direction.normalize()) { }
			Ray(coord::Point3<T> point, Orientation<T> direction) : point_(point), direction_(direction) { }

			coord::Point3<T> initialPoint() const {
				return point_;
			}

			Orientation<T> direction() const {
				return direction_;
			}

			coord::Point3<T> pointAtDistance(T distance) const {
				return point_ + direction_ * distance;
			}
		};
	}
}

#endif // NARF_MATH_RAY_H
