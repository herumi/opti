#include <string>
#include <stdio.h>
#include "mischasan_strstr.hpp"

int main()
{
	MIE_ALIGN(16) const char text[] = "0123456789abdef0123456789abcdef";
	const char *p = text + 1;
	const char key[] = "01";
	const size_t keySize = strlen(key);
	const char *q1 = scanstrN(p, key, keySize);
	const char *q2 = strstr(p, key);
	printf("q1=%p q2=%p %s\n", q1, q2, (q1 == q2) ? "ok" : "ng");
}
