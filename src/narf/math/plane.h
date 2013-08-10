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
			static Plane fromPointNormal(coord::Point3<T> p, Vector3<T> normal) {
				Vector3<T> normnorm = normal.normalize();
				return Plane(normnorm.x, normnorm.y, normnorm.z, -normnorm.dot(Vector3<T>::fromPoint(p)));
			}

			// construct a plane from three points that lie on it
			static Plane fromPoints(coord::Point3<T> p1, coord::Point3<T> p2, coord::Point3<T> p3) {
				Vector3<T> normal = Vector3<T>(p2 - p1).cross(Vector3<T>(p3 - p1));
				return fromPointNormal(p1, normal);
			}

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
				return Vector3<T>::fromPoint(p) - normal() * distanceTo(p);
			}

		};
	}
}

#endif // NARF_MATH_PLANE_H
