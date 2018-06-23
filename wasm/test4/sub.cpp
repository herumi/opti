#include "func.h"

extern "C" uint32_t sub(uint32_t x, uint32_t y)
{
	return x - y;
}

extern "C" void* mymemset(void *p, int v, size_t n)
{
	char *s = (char*)p;
	for (size_t i = 0; i < n; i++) {
		s[i] = (char)v;
	}
	return p;
}

/*
extern "C" int set(uint32_t *x, uint32_t y)
{
	*x = y;
	return y;
}
*/
