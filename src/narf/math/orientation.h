#ifndef NARFBLOCK_MATH_ORIENTATION_H
#define NARFBLOCK_MATH_ORIENTATION_H

#include "narf/math/angle.h"

namespace narf {
	namespace math {
		template<class T>
		class Orientation {
			public:
				Angle<T> inclination;
				Angle<T> azimuth;
				Orientation(T inclination, T azimuth) : inclination(inclination), azimuth(azimuth) {};
				bool operator==(T& rhs) const {
					return AlmostEqual(inclination, rhs.inclination) && AlmostEqual(azimuth, rhs.azimuth);
				}
		};
		typedef Orientation<float> Orientationf;
		typedef Orientation<double> Orientationd;
	}
}

#endif
