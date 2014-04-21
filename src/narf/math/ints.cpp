#include "narf/math/ints.h"
#include <math.h>
#include <float.h>

uint32_t narf::math::ilog2(uint32_t v) {
	// TODO: do a better implementation
	return (uint32_t)(log((double)v) / log(2.0));
}
