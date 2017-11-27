#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MY_API __attribute__((used))

extern "C" {

int MY_API itos(char *buf, size_t bufSize, int64_t x)
{
	return snprintf(buf, bufSize, "%016llx", (long long)x);
}

}
