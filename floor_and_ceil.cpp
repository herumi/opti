/*
Core i7 2600K + VC11(64bit)
CLP2
time= 4.17 ret=3074457345618258603
my_CLP2
time= 2.82 ret=3074457345618258603
FLP2
time= 9.98 ret=6148914689089033557
my_FLP2
time= 4.55 ret=6148914689089033557

Xeon X5650 + gcc 4.6.1(64bit)
CLP2
time= 7.34 ret=3074457345618258603
my_CLP2
time= 4.60 ret=3074457345618258603
FLP2
time=15.49 ret=6148914689089033557
my_FLP2
time= 8.74 ret=6148914689089033557
*/
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#ifdef _WIN32
	#include <intrin.h>
#endif

inline uint32_t FLP2(uint32_t x) {
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  return x - (x >> 1);
}

inline uint32_t CLP2(uint32_t x) {
  x = x - 1;
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  return x + 1;
}

union di {
	uint64_t i;
	double d;
} di;

inline uint32_t my_FLP2(uint32_t x)
{
#if 1
#ifdef _WIN32
	unsigned long ret;
	if (_BitScanReverse(&ret, x)) {
		return 1U << ret;
	} else {
		return 0;
	}
#else
	if (x == 0) return 0;
	return 1U << (__builtin_clz(x) ^ 0x1f);
#endif
#else
	if (x == 0) return 0;
	di.d = x;
	return 1U << ((di.i >> 52) - 1023);
#endif
}

inline uint32_t my_CLP2(uint32_t x)
{
	/* x > 0x80000000 is not necessary if return uint64_t */
	if (x == 0 || x > 0x80000000) return 0;
#if 1
	return my_FLP2(2 * x - 1);
#else
	di.d = x;
	return 1U << (((di.i + (1ULL << 52) - 1) >> 52) - 1023);
#endif
}

template<class F>
void bench(F f, uint32_t limit)
{
	uint64_t a = 0;
	time_t begin = clock();
	for (uint32_t i = 0; i < limit; i++) {
		a += f(i);
	}
	time_t end = clock();
	printf("time=%5.2f ret=%lld\n", (end - begin) / double(CLOCKS_PER_SEC), (long long)a);
}

template<class F>
bool compare(F f1, F f2, uint32_t limit)
{
	for (uint32_t i = 0; i < limit; i++) {
		uint32_t a = f1(i);
		uint32_t b = f2(i);
		if (a != b) {
			printf("ERR %u %u %u\n", i, a, b);
			return false;
		}
	}
	puts("ok");
	return true;
}

int main()
{
	puts("compare CLP2");
	compare(CLP2, my_CLP2, 0xffffffff);
	puts("CLP2");
	bench(CLP2, 0x80000001);
	puts("my_CLP2");
	bench(my_CLP2, 0x80000001);
	puts("compare FLP2");
	compare(FLP2, my_FLP2, 0xffffffff);
	puts("FLP2");
	bench(FLP2, 0xffffffff);
	puts("my_FLP2");
	bench(my_FLP2, 0xffffffff);
}

