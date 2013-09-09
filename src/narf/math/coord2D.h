#ifndef NARFBLOCK_MATH_COORD_COORD2D_H
#define NARFBLOCK_MATH_COORD_COORD2D_H

#include <math.h>
#include <utility>

namespace narf {
	namespace math {
		namespace coord {

			template<class T>
			class Point2;

			template<class T>
			T distance(T x, T y, T x2, T y2);

			template<class T>
			T distance(Point2<T> A, Point2<T> B);

			template<class T>
			class Polar {
				public:
					T radius;
					T angle;
					Polar<T> (T radius, T angle) : radius(radius), angle(angle) {};
					Polar<T> extend(T distance);
					Polar<T> shrink(T distance);
					bool operator==(T& rhs) const;
					operator Point2<T> () const;
			};

			template<class T>
			class Point2 {
				public:
					T x;
					T y;
					Point2<T>(T x, T y) : x(x), y(y) {};
					bool operator==(Point2<T>& rhs) const;
					void swap() { std::swap(x, y); }
					float distanceTo(Point2<T> other) const {
						return distance(x, y, other.x, other.y);
					}
					float distanceTo(T x2, T y2) const {
						return distance(x, y, x2, y2);
					}
					operator Polar<T> () const;
			};

			template<class T>
			T distance(T x, T y, T x2, T y2) {
				T a = (x - x2) * (x - x2);
				T b = (y - y2) * (y - y2);
				return static_cast<T>(sqrt(a + b));
			}

			template<class T>
			T distance(Point2<T> A, Point2<T> B) {
				return A.distanceTo(B);
			}
		}
	}
}

#endif
