#ifndef NARF_MATH_PLANE_H
#define NARF_MATH_PLANE_H

#include "narf/math/coord3D.h"
#include "narf/math/floats.h"
#include "narf/math/vector.h"

namespace narf {
	namespace math {

		template <class T>
		class Plane {
		private:
			// plane defined by the equation ax + by + cz + d = 0
			T a, b, c, d;

		public:

			// construct a plane based on the plane equation ax + by + cz + d = 0
			Plane(T a, T b, T c, T d) : a(a), b(b), c(c), d(d) { }

			// construct a plane based on a point on the plane and a normal vector to the plane
			Plane(coord::Point3<T> p, Vector3<T> normal) {
				Vector3<T> normnorm = normal.normalize();
				a = normnorm.x;
				b = normnorm.y;
				c = normnorm.z;
				d = -normnorm.dot(Vector3<T>(p));
			}

			Plane(coord::Point3<T> p1, coord::Point3<T> p2, coord::Point3<T> p3) : Plane(p1, Vector3<T>(p2 - p1).cross(Vector3<T>(p3 - p1))) {}

			const Vector3<T> normal() const {
				return Vector3<T>(a, b, c);
			}

			const T distanceToOrigin() const {
				return d;
			}

			// find distance from a point to the nearest point on the plane
			T distanceTo(coord::Point3<T> p) const {
				return a * p.x + b * p.y + c * p.z + d;
			}

			bool containsPoint(coord::Point3<T> p) const {
				return AlmostEqual(distanceTo(p), 0);
			}

			// find nearest point on plane to another point
			coord::Point3<T> nearestPoint(coord::Point3<T> p) const {
				return Vector3<T>(p) - normal() * distanceTo(p);
			}

			coord::Point3<T> intersect(coord::Point3<T> p1, coord::Point3<T> p2) {
				return intersect(p1, Vector3<T>(p2 - p1).normalize());
			}

			coord::Point3<T> intersect(narf::math::Ray<T> ray) {
				return intersect(ray.initialPoint(), ray.direction());
			}

			coord::Point3<T> intersect(coord::Point3<T> p1, Vector3<T> direction) {
				auto planePoint = narf::math::Vector3<T>(nearestPoint(p1));
				auto d = (planePoint - p1).dot(normal()) / (direction.dot(normal()));
				return direction * d + p1;
			}

		};
	}
}

#endif // NARF_MATH_PLANE_H
