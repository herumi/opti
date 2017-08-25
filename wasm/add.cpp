#include <stdint.h>
//#define MCL_SIZEOF_UNIT 4
//#include <mcl/vint.hpp>

extern "C" {

#if 0
void add128(uint32_t *z, const uint32_t *x, const uint32_t *y)
{
	mcl::vint::addN(z, x, y, 4);
}
#endif

uint32_t add(uint32_t x, uint32_t y)
{
	return x + y;
}

uint32_t str2int(const uint8_t *s)
{
	return s[0] + s[1] * (1 << 8) + s[2] * (1 << 16) + s[3] * (1 << 24);
//	return 12345;
}

} // extern "C"
