#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

#define API __attribute__((used))

extern "C" {

extern void fillJS1(uint8_t *p, size_t n);
extern void fillJS2(uint8_t *p, size_t n);
extern int call2JS(int x, int y);

API void fill1(uint8_t *p, size_t n)
{
	fillJS1(p, n);
}

API int fill2(uint8_t *p, size_t n)
{
	return EM_ASM_INT({return mod.fillJS($0, $1)}, p, n);
}

API int call0(int x, int y)
{
	return x + y;
}

API int call1(int x, int y)
{
	return EM_ASM_INT({return $0 + $1}, x, y);
}

API int call2(int x, int y)
{
	return call2JS(x, y);
}

API void* malloc_(size_t n)
{
	return malloc(n);
}
API void free_(void *p)
{
	free(p);
}
} // extern "C"
