#include "narf/math/ints.h"
#include <math.h>
#include <float.h>

uint32_t narf::ilog2(uint32_t v) {
	uint32_t msb = 0;
	while (v >>= 1) {
		msb++;
	}
	return msb;
}


int32_t narf::ilog2(int32_t v) {
	return (int32_t)ilog2((uint32_t)v);
}
