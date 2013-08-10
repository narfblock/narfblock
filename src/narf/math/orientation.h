#ifndef NARFBLOCK_MATH_ORIENTATION_H
#define NARFBLOCK_MATH_ORIENTATION_H

#include "narf/math/angle.h"
#include "narf/math/vector.h"

namespace narf {
	namespace math {
		template<class T>
		class Orientation {
			public:
				Angle<T> pitch;
				Angle<T> yaw;
				Angle<T> roll;
				Orientation(T pitch, T yaw, T roll) : pitch(pitch), yaw(yaw), roll(roll) {};
				Orientation(T pitch, T yaw) : pitch(pitch), yaw(yaw), roll(0) {};
				Orientation() : pitch(0), yaw(0), roll(0) {};
				//Angle<T> inclination() { return M_PI - pitch; } // Zenith down
				//Angle<T>& altitude() { return pitch; } // Horizon up (same as pitch)
				//Angle<T>& azimuth() { return yaw; } // North clockwise (same as yaw)
				bool operator==(T& rhs) const {
					return AlmostEqual(pitch, rhs.pitch) && AlmostEqual(yaw, rhs.yaw);
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
}

#endif
