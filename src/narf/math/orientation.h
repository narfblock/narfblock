#ifndef NARFBLOCK_MATH_ORIENTATION_H
#define NARFBLOCK_MATH_ORIENTATION_H

#include "narf/bytestream.h"
#include "narf/math/angle.h"

namespace narf {
	template<class T>
	class Point3;

	template<class T>
	class Vector3;

	// Euler angles describing orientation
	template<class T>
	class Orientation {
	public:
		Angle<T> pitch;
		Angle<T> yaw;
		Angle<T> roll;
		Orientation(T pitch, T yaw, T roll) : pitch(pitch), yaw(yaw), roll(roll) {};
		Orientation(T pitch, T yaw) : pitch(pitch), yaw(yaw), roll(0) {};
		Orientation(Point3<T> a, Point3<T> b) : roll(0) { };
		Orientation() : pitch(0), yaw(0), roll(0) {};

		Orientation(ByteStreamReader& s) :
			pitch(s),
			yaw(s),
			roll(s) {
		}

		void serialize(ByteStreamWriter& s) const {
			pitch.serialize(s);
			yaw.serialize(s);
			roll.serialize(s);
		}

		bool operator==(const T& rhs) const {
			return almostEqual(pitch, rhs.pitch) && almostEqual(yaw, rhs.yaw);
		}

		// convert direction of orientation to a vector, ignoring roll
		operator Vector3<T>() const {
			auto x = cos(yaw) * cos(pitch);
			auto y = sin(yaw) * cos(pitch);
			auto z = sin(pitch);
			return Vector3<T>(x, y, z);
		}
	};

	typedef Orientation<float> Orientationf;
	typedef Orientation<double> Orientationd;
}

#endif
