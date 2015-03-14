#include <stdint.h>
#include <intrin.h>

void add128(uint64_t* z, const uint64_t* x, const uint64_t* y)
{
	uint8_t c = _addcarry_u64(0, x[0], y[0], &z[0]);
	_addcarry_u64(c, x[1], y[1], &z[1]);
}
