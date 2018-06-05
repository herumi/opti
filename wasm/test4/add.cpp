#include <stdint.h>

#define API __attribute__((visibility("default")))
extern "C" API uint32_t add(uint32_t x, uint32_t y)
{
	return x + y;
}
extern "C" uint32_t sub(uint32_t x, uint32_t y);

extern "C" uint32_t addJS(uint32_t x, uint32_t y);

extern "C" API uint32_t subsub(uint32_t x, uint32_t y)
{
	return sub(x, y) - y;
}

extern "C" API uint32_t callJS(uint32_t x)
{
	return addJS(x, 999);
}
