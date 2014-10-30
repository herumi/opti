#include <stdio.h>

int blsi(int x)
{
	return (-x) & x;
}

int blsr(int x)
{
	return x & (x - 1);
}

int blsmsk(int x)
{
	return x ^ (x - 1);
}

void put(int x)
{
	for (int i = 8; i >= 0; i--) {
		printf("%c", x & (1u << i) ? '1' : '0');
	}
}

void putPattern(const char *msg, int f(int))
{
	puts(msg);
	for (int i = 0; i < 20; i++) {
		printf("%2d ", i);
		put(i);
		printf(" ");
		put(f(i));
		printf("\n");
	}
}

int main()
{
	putPattern("blsi", blsi);
	putPattern("blsr", blsr);
	putPattern("blsmsk", blsmsk);
}