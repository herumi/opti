#include "api.h"

extern "C" {

API int add(int x, int y)
{
	return x + y;
}

API const char *get()
{
	static const char *s = "abcdefg";
	return s;
}

API const char *get2()
{
	static const char *s = "XYZW";
	return s;
}


}

