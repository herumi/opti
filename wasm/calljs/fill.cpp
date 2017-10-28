#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

extern "C" {

extern void fillJS(uint8_t *p, size_t n);
extern int call2JS(int x, int y);

void fill1(uint8_t *p, size_t n)
{
	fillJS(p, n);
}

void fill2(uint8_t *p, size_t n)
{
	EM_ASM((mod.fillJS($0, $1)), p, n);
}

int call0(int x, int y)
{
	return x + y;
}

int call1(int x, int y)
{
	return EM_ASM_INT((return $0 + $1), x, y);
}

int call2(int x, int y)
{
	return call2JS(x, y);
}

} // extern "C"
