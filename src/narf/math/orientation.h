#ifndef NARFBLOCK_MATH_ORIENTATION_H
#define NARFBLOCK_MATH_ORIENTATION_H

#include "narf/bytestream.h"
#include "narf/math/angle.h"
#include "narf/math/quaternion.h"

namespace narf {
	template<class T>
	class Point3;

	template<class T>
	class Vector3;

	template<class T>
	class Quaternion;

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

		Orientation(Quaternion<T> q) : pitch(0), yaw(0), roll(0) { // TODO: Test to see if this is right
			// http://www.sedris.org/wg8home/Documents/WG80485.pdf
			q.normalizeSelf();
			T test = q.v.x * q.v.z - q.v.y * q.w;
			if (almostEqual(test, 0.5)) {
				pitch = -(T)M_PI / 2;
				roll = 0;
				yaw = std::atan2(q.v.x * q.v.y - q.w * q.v.z, q.v.y * q.v.z + q.v.x * q.v.y);
			} else if (almostEqual(test, -0.5)) {
				pitch = (T)M_PI / 2;
				roll = 0;
				yaw = std::atan2(q.v.x * q.v.y - q.w * q.v.z, q.v.y * q.v.z + q.v.x * q.v.y);
			} else {
				pitch = -std::atan2((T)2 * (q.v.y * q.v.z + q.w * q.v.x), 1 - 2 * (q.v.x * q.v.x + q.v.y * q.v.y));
				roll = std::asin((T)2 * (q.v.x * q.v.z - q.w * q.v.y));
				yaw = std::atan2((T)2 * (q.v.x * q.v.y + q.w * q.v.z), 1 - 2 * (q.v.y * q.v.y + q.v.z * q.v.z));
			}
		}

		Orientation(ByteStream& s) :
			pitch(s),
			yaw(s),
			roll(s) {
		}

		void serialize(ByteStream& s) const {
			pitch.serialize(s);
			yaw.serialize(s);
			roll.serialize(s);
		}

		bool operator==(const T& rhs) const {
			return almostEqual(pitch, rhs.pitch) && almostEqual(yaw, rhs.yaw);
		}

		// convert direction of orientation to a vector, ignoring roll
		operator Vector3<T>() const {
			T x = std::cos(yaw) * std::cos(pitch);
			T y = std::sin(yaw) * std::cos(pitch);
			T z = std::sin(pitch);
			return Vector3<T>(x, y, z);
		}

	};

	typedef Orientation<float> Orientationf;
	typedef Orientation<double> Orientationd;
}

#endif
