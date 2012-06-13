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
#include <algorithm>
#include "intsort.hpp"
#include <xbyak/xbyak_util.h>
/*
	mode = 0 : random
	       1 : all zero
           2 : almost presorted
           3 : forward presorted
           4 : reversed presorted
	       5 : 16 bit random
	       6 : 8 bit random
*/
const char *mode2str(int mode)
{
	switch (mode) {
	case 0:
		return "random";
	case 1:
		return "all zero";
	case 2:
		return "almost presorted";
	case 3:
		return "forward presorted";
	case 4:
		return "reversed presorted";
	case 5:
		return "16 bit random";
	case 6:
		return "8 bit random";
	default:
		fprintf(stderr, "ERR mode=%d in modeStr\n", mode);
		exit(1);
	}
}
void Init(uint32_t *a, size_t len, int mode = 0)
{
	switch (mode) {
	case 0:
		{
			XorShift128 r;
			for (size_t i = 0; i < len; i++) {
				uint32_t x = r.get();
				a[i] = x;
			}
		}
		break;
	case 1:
		for (size_t i = 0; i < len; i++) {
			a[i] = 0;
		}
		break;
	case 2:
		a[0] = len;
		for (size_t i = 1; i < len; i++) {
			a[i] = 1;
		}
		break;
	case 3:
		for (size_t i = 0; i < len; i++) {
			a[i] = (int)i;
		}
		break;
	case 4:
		for (size_t i = 0; i < len; i++) {
			a[i] = (int)(len - i);
		}
		break;
	case 5:
		{
			XorShift128 r;
			for (size_t i = 0; i < len; i++) {
				uint32_t x = r.get();
				a[i] = x & 0xffff;
			}
		}
		break;
	case 6:
		{
			XorShift128 r;
			for (size_t i = 0; i < len; i++) {
				uint32_t x = r.get();
				a[i] = x & 0xff;
			}
		}
		break;
	default:
		fprintf(stderr, "ERR mode=%d\n", mode);
		exit(1);
	}
}

bool isSorted(const uint32_t *a, size_t len)
{
	if (len > 0) {
		for (size_t i = 0; i < len - 1; i++) {
			if (a[i] > a[i + 1]) return false;
		}
	}
	return true;
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
	AlignedArray<uint32_t> wk(N);
	memcpy(&wk[0], a, N * sizeof(a[0]));
	Xbyak::util::Clock clk;
	clk.begin();
	f(&wk[0], N);
	clk.end();
	std::sort(a, a + N);
	for (size_t i = 0; i < N; i++) {
		if (a[i] != wk[i]) {
			fprintf(stderr, "NG %d %08x %08x\n", (int)i, a[i], wk[i]);
			break;
		}
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

void test_merge()
{
	puts("test_merge");
	for (int i = 0; i < 19; i++) {
		const size_t N = 16 * (1U << i);
		AlignedArray<uint32_t> va(N);
		AlignedArray<uint32_t> vo(N);
		uint32_t *const a = &va[0];
		double c1, c2;
		const int mode = 0;
		Init(a, N, mode);
		STLsort(a, N / 2);
		STLsort(a + N / 2, N / 2);
		{
			// fill cache
			std::merge(a, a + N / 2, a + N / 2, a + N, &vo[0]);
		}

		Init(a, N, mode);
		std::fill(&vo[0], &vo[N], 0);
		STLsort(a, N / 2);
		STLsort(a + N / 2, N / 2);
		{
			Xbyak::util::Clock clk;
			clk.begin();
			std::set_union(a, a + N / 2, a + N / 2, a + N, &vo[0]);
			clk.end();
			c1 = clk.getClock() * 1e-3;
		}
		Init(a, N, mode);
		std::fill(&vo[0], &vo[N], 0);
		STLsort(a, N / 2);
		STLsort(a + N / 2, N / 2);
		{
			Xbyak::util::Clock clk;
			clk.begin();
			merge((V128*)&vo[0], (const V128*)a, N / 8,(const V128*)(a + N / 2), N / 8);
			clk.end();
			c2 = clk.getClock() * 1e-3;
		}

		STLsort(a, N);
		for (size_t j = 0; j < N; j++) {
			if (a[j] != vo[j]) {
				printf("ERR %d %u %u\n", (int)i, a[j], vo[j]);
				exit(1);
			}
		}
		printf("%8d %11.2f %11.2f %.2f\n", (int)N, c1, c2, c1 / c2);
	}
}

int main()
{
	if (0) {
		const int N = 1;
		MIE_ALIGN(16) uint32_t buf[N * 8];
		MIE_ALIGN(16) uint32_t wk[N * 8];
		MIE_ALIGN(16) uint32_t out[N * 8];
		for (int i = 0; i < N * 4; i++) {
			buf[i + N * 4] = i * 2;
			wk[i] = i * 2 + 1;
		}
		put("buf", (V128*)buf,N * 2);
		put("wk", (V128*)wk,N);
		merge((V128*)out, (V128*)&buf[N * 4], N, (V128*)&wk[0], N);
		put("out", (V128*)out, N * 2);
		merge((V128*)buf, (V128*)&buf[N * 4], N, (V128*)&wk[0], N);
		put("buf", (V128*)buf,N * 2);
		return 1;
	}
	printf("%8s %10s %10s %4s\n", "N", "STL", "SSE", "rate");
//	test_vector_merge();
//	test_merge();
	puts("test");
	for (int mode = 0; mode < 7; mode++) {
		printf("mode=%s\n", mode2str(mode));
		for (int i = 0; i < 20; i++) {
			const size_t N = 16 * (1U << i);
			AlignedArray<uint32_t> va(N);
			uint32_t *const a = &va[0];
			Init(a, N, mode);
			double c1 = test(STLsort, a, N);
			Init(a, N, mode);
			double c2 = test(intsort, a, N);
			printf("%8d %11.2f %11.2f %.2f\n", (int)N, c1, c2, c1 / c2);
		}
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
	put(a, N);
	printf("isSorted=%d\n", isSorted(a, N));
	puts("sort_step1");
	sort_step1(a, N);
	put(a, N);
#endif
}
