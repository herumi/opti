#include "func.h"

#if defined(__EMSCRIPTEN__) || defined(__clang__)
  #define API __attribute__((used))
#elif defined(__wasm__)
  #define API __attribute__((visibility("default")))
#else
  #define API
#endif

extern "C" API uint32_t add(uint32_t x, uint32_t y)
{
	return x + y;
}
extern "C" uint32_t addJS(uint32_t x, uint32_t y);

extern "C" API uint32_t subsub(uint32_t x, uint32_t y)
{
	return sub(x, y) - y;
}

extern "C" API uint32_t callJS(uint32_t x)
{
	return addJS(x, 999);
}

extern "C" API uint32_t addmem(const uint32_t *x, uint32_t n)
{
	uint32_t s = 0;
	for (uint32_t i = 0; i < n; i++) {
		s += x[i];
	}
	return s;
}

/*
extern "C" API uint32_t wrong_ret(uint32_t x, uint32_t y)
{
	set(&x, y);
	return x + y;
}
*/

struct Counter {
	int c;
	Counter(int c = 0) : c(c) {}
	int inc() { return c++; }
};

extern "C" API int getCount1()
{
	static Counter c(3);
	return c.inc();
}
static Counter s_c(9);

extern "C" API int getCount2()
{
	return s_c.inc();
}

extern "C" API void setn(void *p, int n)
{
	mymemset(p, 1, n);
}
