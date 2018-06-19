#include <stdint.h>

extern "C" uint32_t sub(uint32_t x, uint32_t y)
{
	return x - y;
}

/*
extern "C" int set(uint32_t *x, uint32_t y)
{
	*x = y;
	return y;
}
*/
