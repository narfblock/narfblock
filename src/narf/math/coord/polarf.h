#ifndef NARFBLOCK_MATH_COORD_POLARF_H
#define NARFBLOCK_MATH_COORD_POLARF_H

#include <math.h>

namespace narf {
	namespace util {
		namespace coord {
			class Polarf {
				public:
					float radius;
					float angle;
					PolarF(float radius, float angle) : radius(radius), angle(angle) {};
					PolarF extend(float distance);
					PolarF shrink(float distance);
					bool operator==(T& rhs) const;
					operator Point2f () const;
			}
		}
	}
}

#endif
