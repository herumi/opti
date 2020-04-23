#include "api.h"

extern "C" int mulJS(int x, int y);

extern "C" {

API int add(int x, int y)
{
	return x + y;
}

API int callJS(int x, int y)
{
	return mulJS(x, y);
}

API int getPtr(int x)
{
	char buf[x];
	return (int)buf;
}

}

