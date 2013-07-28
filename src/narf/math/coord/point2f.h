#ifndef NARFBLOCK_MATH_COORD_POINT2F_H
#define NARFBLOCK_MATH_COORD_POINT2F_H

#include <math.h>

namespace narf {
	namespace math {
		namespace coord {
			class Point2f {
				public:
					float x;
					float y;
					Point2f(float x, float y) : x(x), y(y) {};
					bool operator==(Point2f& rhs) const;
					operator Polarf () const;
			}
		}
	}
}

#endif
