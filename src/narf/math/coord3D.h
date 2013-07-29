#ifndef NARFBLOCK_MATH_COORD_SPHERICALF_H
#define NARFBLOCK_MATH_COORD_SPHERICALF_H

#include <math.h>
#include "narf/math/floats.h"
#include "narf/math/orientation.h"

namespace narf {
	namespace math {
		namespace coord {

			template<class T>
			class Point3;

			template<class T>
			T distance(T x, T y, T z, T x2, T y2, T z2);

			template<class T>
			T distance(Point3<T> A, Point3<T> B);

			template<class T>
			class Spherical {
				public:
					T radius;
					T azimuth;
					T inclination;
					Spherical(T radius, T inclination, T azimuth) : radius(radius), azimuth(azimuth), inclination(inclination) {};

					T distanceTo(Spherical<T>& other) {
						T rel_angle_cos = sin(inclination) * sin(other.inclination) + cos(inclination) * cos(other.inclination) * cos(azimuth - other.azimuth);
						return static_cast<T>(sqrt(radius * radius + other.radius * other.radius - 2 * radius * other.radius * rel_angle_cos));
					}

					bool operator==(Spherical<T> rhs) const {
						return narf::math::AlmostEqual(radius, rhs.radius) &&
							narf::math::AlmostEqual(inclination, rhs.inclination) &&
							narf::math::AlmostEqual(azimuth, rhs.azimuth);
					}
					operator Point3<T> () const {
						T x = radius * sin(inclination) * cos(azimuth);
						T y = radius * sin(inclination) * sin(azimuth);
						T z = radius * cos(inclination);
						return Point3<T>(x, y, z);
					}
					Orientation<T> getOrientation() const {
						return Orientation<T>(inclination, azimuth);
					}
			};

			template<class T>
			class Point3 {
				public:
					T x;
					T y;
					T z;
					Point3(T x, T y, T z) : x(x), y(y), z(z) {};
					bool operator==(Point3<T>& rhs) const {
						return narf::math::AlmostEqual(x, rhs.x) && narf::math::AlmostEqual(y, rhs.y) && narf::math::AlmostEqual(z, rhs.z);
					}
					operator Spherical<T> () const {
						float radius = distanceTo(0, 0, 0);
						float inclination = !narf::math::AlmostEqual(radius, 0) ? acos(z/radius) : 0;
						float azimuth = !narf::math::AlmostEqual(x, 0) ? atan2(y, x) : 0;
						return Spherical<T>(radius, inclination, azimuth);
					}
					float distanceTo(Point3<T> other) const {
						return distance(x, y, z, other.x, other.y, other.z);
					}
					float distanceTo(T x2, T y2, T z2) const {
						return distance(x, y, z, x2, y2, z2);
					}
			};

			typedef Spherical<float> Sphericalf;
			typedef Spherical<double> Sphericald;
			typedef Point3<float> Point3f;
			typedef Point3<double> Point3d;

			template<class T>
			T distance(T x, T y, T z, T x2, T y2, T z2) {
				T a = (x - x2) * (x - x2);
				T b = (y - y2) * (y - y2);
				T c = (z - z2) * (z - z2);
				return static_cast<T>(sqrt(a + b + c));
			}

			template<class T>
			T distance(Point3<T> A, Point3<T> B) {
				return A.distanceTo(B);
			}

		}
	}
}

#endif
