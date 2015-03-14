#include <stdint.h>

void add128(uint64_t *pz, const uint64_t *px, const uint64_t *py)
{
	uint64_t x0 = px[0];
	uint64_t y0 = py[0];
	uint64_t z0 = x0 + y0;
	pz[0] = z0;
	uint64_t x1 = px[1];
	uint64_t y1 = py[1];
	if (z0 >= x0) {
		pz[1] = x1 + y1;
	} else {
		pz[1] = x1 + y1 + 1;
	}
}

