/*
	g++ -O3 -fno-operator-names -march=native -msse4 intsort_test.cpp && ./a.out
	Xeon X5650
	isSorted=1:bb0579b6 5320.585
	sort by SIMD
	isSorted=1:bb0579b6 1340.714

	VC11 x64
	Core i7 2600
	sort by STL
	isSorted=1:bb0579b6 6628.255
	sort by SIMD
	isSorted=1:bb0579b6 1551.794
*/
#include <stdio.h>
#include <numeric>
#include "intsort.hpp"
#include <xbyak/xbyak_util.h>

void Init(uint32_t *a, size_t len)
{
	XorShift128 r;
	for (size_t i = 0; i < len; i++) {
		uint32_t x = r.get();
		a[i] = x;
	}
}

void put(const uint32_t *a, size_t len)
{
	for (size_t i = 0; i < len; i += 4) {
		printf("%08x:%08x:%08x:%08x\n", a[i], a[i + 1], a[i + 2], a[i + 3]);
	}
}

uint64_t sum(const uint32_t *a, size_t len)
{
	return (uint64_t)std::accumulate(a, a + len, 0ULL);
}

template<class F>
double test(F f, uint32_t *a, size_t N)
{
	uint64_t pre = sum(a, N);
	Xbyak::util::Clock clk;
	clk.begin();
	f(a, N);
	clk.end();
	uint64_t cur = sum(a, N);
	if (pre != cur) {
		fprintf(stderr, "value is different\n");
		return -1;
	}
	if (!isSorted(a, N)) {
		fprintf(stderr, "a is not sorted\n");
		return -1;
	}
	return clk.getClock() * 1e-3;
}

void STLsort(uint32_t *a, size_t N)
{
	std::sort(a, a + N);
}

int main()
{
	for (int i = 0; i < 16; i++) {
		const size_t N = 16 * (1U << i);
		AlignedArray<uint32_t> va(N);
		uint32_t *const a = &va[0];
		Init(a, N);
		double c1 = test(STLsort, a, N);
		Init(a, N);
		double c2 = test(intsort, a, N);
		printf("N=%6d, STL=%11.3fKclk SSE=%11.3fKclk(%.2f)\n", (int)N, c1, c2, c1 / c2);
	}
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
}
