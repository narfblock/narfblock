#ifndef NARFBLOCK_MATH_ANGLE_H
#define NARFBLOCK_MATH_ANGLE_H

#include <math.h>
#include "narf/math/math.h"
#include <type_traits>
#include <stdio.h>

namespace narf {
	namespace math {
		template <class T>
		T toDeg(T angle);
		template <class T>
		T fromDeg(T angle);
		template <class T>
		T reduceAngle(T angle);

		template <class T>
		class Angle {
			static_assert(std::is_same<T, double>::value || std::is_same<T, float>::value, "Angle requires float or double type");

			public:
				T angle;
				Angle(T angle) : angle(angle) {};
				const Angle<T> operator+(const Angle<T> &add) const {
					return Angle<T>(reduceAngle<T>(angle + add));
				};
				const Angle<T> operator-(const Angle<T> &sub) const {
					return Angle<T>(reduceAngle<T>(angle - sub));
				}
				Angle<T>& operator+=(const T &add) const {
					angle = reduceAngle<T>(angle + add);
					return *this;
				}
				Angle<T>& operator-=(const Angle<T> &sub) const {
					angle = reduceAngle<T>(angle - sub);
					return *this;
				}
				T toDeg() { return narf::math::toDeg(angle); }
				operator T () const { return angle; };
				bool operator==(T& rhs) const { return AlmostEqual(angle, rhs.angle); }
				bool operator==(Angle<T>& rhs) const { return AlmostEqual(angle, rhs.angle); }
		};

		typedef Angle<float> Anglef;
		typedef Angle<double> Angled;

		template <class T>
		T toDeg(T angle) {
			static_assert(
					std::is_same<T, double>::value || std::is_same<T, float>::value, "toDeg requires float or double type");
			return reduceAngle<T>(angle) * (180.0 / M_PI);
		}

		template <class T>
		T fromDeg(T angle) {
			static_assert(
					std::is_same<T, double>::value || std::is_same<T, float>::value, "fromDeg requires float or double type");
			return reduceAngle<T>(angle * (M_PI / 180.0));
		}

		template <class T>
		T reduceAngle(T angle) {
			static_assert(
					std::is_same<T, double>::value || std::is_same<T, float>::value, "reduceAngle requires float or double type");
			while (angle < 0 || angle > 2*M_PI) {
				if (angle < 0) {
				angle += 2*M_PI;
				} else if (angle > 2*M_PI) {
					angle -= 2*M_PI;
				} else if (AlmostEqual(angle, 0)) {
					return 0;
				} else if (AlmostEqual(angle, 2*M_PI)) {
					return 2*M_PI;
				}
			}
			return angle;
		}
	}
}

#endif
