#ifndef NARFBLOCK_MATH_ANGLE_H
#define NARFBLOCK_MATH_ANGLE_H

#include <cmath>
#include "narf/bytestream.h"
#include "narf/math/floats.h"
#include <type_traits>
#include <stdio.h>

namespace narf {
	template <class T>
	T toDeg(T angle) {
		static_assert(
				std::is_same<T, double>::value || std::is_same<T, float>::value, "toDeg requires float or double type");
		return angle * static_cast<T>((180.0 / M_PI));
	}

	template <class T>
	T fromDeg(T angle) {
		static_assert(
				std::is_same<T, double>::value || std::is_same<T, float>::value, "fromDeg requires float or double type");
		return angle * static_cast<T>((M_PI / 180.0));
	}

	template <class T>
	T reduceAngle(T angle) {
		return reduceAngle<T>(angle, 0, static_cast<T>(2*M_PI));
	}

	template <class T>
	T reduceAngle(T angle, T minimum, T maximum) {
		static_assert(
				std::is_same<T, double>::value || std::is_same<T, float>::value, "reduceAngle requires float or double type");
		while (angle < minimum || angle > maximum) {
			if (angle < minimum) {
			angle += static_cast<T>(2*M_PI);
			} else if (angle > maximum) {
				angle -= static_cast<T>(2*M_PI);
			} else if (almostEqual(angle, minimum)) {
				return minimum;
			} else if (almostEqual(angle, maximum)) {
				return maximum;
			}
		}
		return angle;
	}

	template <class T>
	class Angle {
	//static_assert(std::is_same<T, double>::value || std::is_same<T, float>::value, "Angle requires float or double type");
	public:
		T angle;
		T minimum;
		T maximum;
		Angle(T angle) : angle(angle), minimum(0.0), maximum(static_cast<T>(2 * M_PI)) {};
		Angle(T angle, T minimum, T maximum) : angle(angle), minimum(minimum), maximum(maximum) {};

		Angle(ByteStream& s) {
			s.read(&angle, ByteStream::Endian::LITTLE);
		}

		void serialize(ByteStream& s) const {
			s.write(angle, ByteStream::Endian::LITTLE);
		}

		Angle<T> operator+(const T add) const {
			return Angle<T>(reduceAngle<T>(angle + add, minimum, maximum), minimum, maximum);
		};
		Angle<T> operator-(const T sub) const {
			return Angle<T>(reduceAngle<T>(angle - sub, minimum, maximum), minimum, maximum);
		}
		Angle<T>& operator+=(const T add) {
			angle = reduceAngle<T>(angle + add, minimum, maximum);
			return *this;
		}
		Angle<T>& operator-=(const T sub) {
			angle = reduceAngle<T>(angle - sub, minimum, maximum);
			return *this;
		}
		T toDeg() const { return narf::toDeg(angle); }
		operator T () const { return angle; };
		bool operator==(T& rhs) const { return almostEqual(angle, rhs.angle); }
		bool operator==(Angle<T>& rhs) const { return almostEqual(angle, rhs.angle); }
	};

	typedef Angle<float> Anglef;
	typedef Angle<double> Angled;
}

#endif
