/*
g++ -O3 -fno-operator-names -march=native -msse4 intsort_test.cpp && ./a.out
Xeon X5650
sort by STL
isSorted=1:bb0579b6 5378.599
sort by SIMD
isSorted=1:bb0579b6 1550.305
*/
#include <stdio.h>
#include <numeric>
#include "intsort.hpp"
#include <xbyak/xbyak_util.h>

void Init(uint32_t *a, size_t len)
{
	XorShift128 r;
	for (size_t i = 0; i < len; i++) {
		a[i] = r.get();
	}
}

void put(const uint32_t *a, size_t len)
{
	for (size_t i = 0; i < len; i += 4) {
		printf("%08x:%08x:%08x:%08x\n", a[i], a[i + 1], a[i + 2], a[i + 3]);
	}
}

uint32_t checksum(const uint32_t *a, size_t len)
{
#if 0
	return std::accumulate(a, a + len, 0);
#else
	uint32_t ret = 0x12345;
	for (size_t i = 0; i < len; i++) {
		ret = ret * 0x12345 + a[i];
	}
	return ret;
#endif
}

int main()
{
	const size_t N = 16 * 2048;
	AlignedArray<uint32_t> va(N);
	uint32_t *const a = &va[0];
	Init(a, N);
//	put(a, N);
	AlignedArray<uint32_t> vb(N);
	uint32_t *const b = &vb[0];
	Init(b, N);

#if 0
	{
		puts("cmpswap_skew");
		V128 a(4, 5, 6, 7);
		V128 b(8, 9, 3, 1);
		a.put("a=");
		b.put("b=");
		vector_cmpswap_skew(a, b);
		puts("compswap_skew");
		a.put("a=");
		b.put("b=");
	}
	combSort(a, N);
	put(a, N);
	printf("isSorted=%d\n", isSorted(a, N));
	puts("sort_step1");
	sort_step1(a, N);
	put(a, N);
#endif
	puts("sort by STL");
	{
		Xbyak::util::Clock clk;
		clk.begin();
		std::sort(b, b + N);
		clk.end();
		printf("isSorted=%d:%08x %.3f\n", isSorted(b, N), checksum(b, N), clk.getClock() * 1e-3);
	}
	puts("sort by SIMD");
	{
		Xbyak::util::Clock clk;
		clk.begin();
		intsort(a, N);
		clk.end();
//		put(a, N);
		printf("isSorted=%d:%08x %.3f\n", isSorted(a, N), checksum(a, N), clk.getClock() * 1e-3);
	}
}
