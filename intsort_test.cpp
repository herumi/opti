/*
	g++ -O3 -fno-operator-names -march=native -msse4 intsort_test.cpp && ./a.out
	Xeon X5650
N=     16, STL=     16.314Kclk SSE=     18.614Kclk(0.88)
N=     32, STL=      5.034Kclk SSE=      3.966Kclk(1.27)
N=     64, STL=      9.414Kclk SSE=      3.514Kclk(2.68)
N=    128, STL=     20.034Kclk SSE=      6.174Kclk(3.24)
N=    256, STL=     45.320Kclk SSE=     12.566Kclk(3.61)
N=    512, STL=    100.646Kclk SSE=     28.688Kclk(3.51)
N=   1024, STL=    216.054Kclk SSE=     50.520Kclk(4.28)
N=   2048, STL=    477.180Kclk SSE=    104.614Kclk(4.56)
N=   4096, STL=   1016.746Kclk SSE=    217.114Kclk(4.68)
N=   8192, STL=   2222.946Kclk SSE=    505.354Kclk(4.40)
N=  16384, STL=   4766.060Kclk SSE=   1050.340Kclk(4.54)
N=  32768, STL=  10055.274Kclk SSE=   2431.974Kclk(4.13)
N=  65536, STL=  21363.166Kclk SSE=   5497.000Kclk(3.89)
N= 131072, STL=  44519.274Kclk SSE=  13496.912Kclk(3.30)
N= 262144, STL=  93955.220Kclk SSE=  31077.720Kclk(3.02)
N= 524288, STL= 196977.692Kclk SSE=  59143.712Kclk(3.33)
N=1048576, STL= 415590.628Kclk SSE= 131437.020Kclk(3.16)
N=2097152, STL= 864996.120Kclk SSE= 169437.825Kclk(5.11)
N=4194304, STL= 955930.427Kclk SSE= 399368.311Kclk(2.39)

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
	} else if (!isSorted(a, N)) {
		fprintf(stderr, "a is not sorted\n");
	}
	return clk.getClock() * 1e-3;
}

void STLsort(uint32_t *a, size_t N)
{
	std::sort(a, a + N);
}

void test_vector_merge()
{
	puts("test_vector_merge");
	if (0) {
		V128 a(7, 6, 5, 4);
		V128 b(9, 8, 3, 1);
		a.put("a=");
		b.put("b=");
		vector_merge(a, b);
		puts("compswap_skew");
		a.put("a=");
		b.put("b=");
	}
	for (int a0 = 0; a0 < 8; a0++) {
		for (int a1 = a0; a1 < 8; a1++) {
			for (int a2 = a1; a2 < 8; a2++) {
				for (int a3 = a2; a3 < 8; a3++) {
					for (int b0 = 0; b0 < 8; b0++) {
						for (int b1 = b0; b1 < 8; b1++) {
							for (int b2 = b1; b2 < 8; b2++) {
								for (int b3 = b2; b3 < 8; b3++) {
									V128 a(a3, a2, a1, a0);
									V128 b(b3, b2, b1, b0);
									vector_merge(a, b);
									MIE_ALIGN(16) uint32_t buf[8];
									a.store(buf);
									b.store(buf + 4);
									if (!isSorted(buf, 8)) {
										puts("ERR not sorted");
										a.put("a=");
										b.put("b=");
										exit(1);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	puts("ok");
}

void test_int_mergesort()
{
	for (int i = 0; i < 19; i++) {
		const size_t N = 16 * (1U << i);
		AlignedArray<uint32_t> va(N);
		AlignedArray<uint32_t> vo(N);
		uint32_t *const a = &va[0];
		Init(a, N);
		STLsort(a, N / 2);
		STLsort(a + N / 2, N / 2);
		int_mergesort((V128*)&vo[0], (const V128*)a, N / 8,(const V128*)(a + N / 2), N / 8);
		STLsort(a, N);
		for (size_t j = 0; j < N; j++) {
			if (a[i] != vo[i]) {
				printf("ERR %d %u %u\n", (int)i, a[i], vo[i]);
				break;
			}
		}
	}
}

int main()
{
	test_vector_merge();
	test_int_mergesort();
	/* i == 19 reaches max loop */
	printf("%7s %11s %11s %4s\n", "N", "STL", "SSE", "rate");
	for (int i = 0; i < 19; i++) {
		const size_t N = 16 * (1U << i);
		AlignedArray<uint32_t> va(N);
		uint32_t *const a = &va[0];
		Init(a, N);
		double c1 = test(STLsort, a, N);
		Init(a, N);
		double c2 = test(intsort, a, N);
		printf("%7d %11.3f %11.3f %.2f\n", (int)N, c1, c2, c1 / c2);
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
