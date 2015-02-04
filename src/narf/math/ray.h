#ifndef NARF_MATH_RAY_H
#define NARF_MATH_RAY_H

#include "narf/math/vector.h"
#include "narf/math/orientation.h"

namespace narf {

	// a ray in 3D space defined by an initial point and a direction
	template<class T>
	class Ray {
	private:
		Point3<T> point_; // initial point
		Vector3<T> direction_; // direction (unit vector)
		Vector3<T> inverseDirection_; // 1/direction
	public:
		Ray(const Vector3<T>& direction) :
			point_(Point3<T>(0, 0, 0)),
			direction_(direction.normalize()),
			inverseDirection_(1.0f / direction_.x, 1.0f / direction_.y, 1.0f / direction_.z) {
		}

		Ray(const Orientation<T>& direction) :
			point_(Point3<T>(0, 0, 0)),
			direction_(direction),
			inverseDirection_(1.0f / direction_.x, 1.0f / direction_.y, 1.0f / direction_.z) {
		}

		Ray(const Point3<T>& point, const Vector3<T>& direction) :
			point_(point),
			direction_(direction.normalize()),
			inverseDirection_(1.0f / direction_.x, 1.0f / direction_.y, 1.0f / direction_.z) {
		}

		Ray(const Point3<T>& point, const Orientation<T>& direction) :
			point_(point),
			direction_(direction),
			inverseDirection_(1.0f / direction_.x, 1.0f / direction_.y, 1.0f / direction_.z) {
		}

		const Point3<T>& initialPoint() const {
			return point_;
		}

		Vector3<T> direction() const {
			return direction_;
		}

		const Vector3<T>& inverseDirection() const {
			return inverseDirection_;
		}

		Point3<T> pointAtDistance(T distance) const {
			return point_ + direction_ * distance;
		}
	};
}

#endif // NARF_MATH_RAY_H
