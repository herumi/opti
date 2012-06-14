/*
	g++ -O3 -fno-operator-names -march=native -msse4 intsort_test.cpp && ./a.out
	Xeon X5650
*/
#include <stdio.h>
#include <numeric>
#include <algorithm>
#include <xbyak/xbyak_util.h>
#include "intsort.hpp"

/*
	mode = 0 : random
	       1 : all zero
           2 : almost presorted
           3 : forward presorted
           4 : reversed presorted
	       5 : 16 bit random
	       6 : 8 bit random
*/
const char *modeTbl[] = {
	"random",
	"all zero",
	"almost presorted",
	"forward presorted",
	"reversed presorted",
	"16 bit random",
	"8 bit random",
};

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
		a[0] = (uint32_t)len;
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
double test(F f, uint32_t *a, size_t N, size_t BN)
{
	AlignedArray<uint32_t> wk(N);
	memcpy(&wk[0], a, N * sizeof(a[0]));
	Xbyak::util::Clock clk;
	clk.begin();
	f(&wk[0], N, BN);
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

void STLsort(uint32_t *a, size_t N, size_t = 0)
{
	std::sort(a, a + N);
}

void test_vector_merge()
{
	using namespace mie::intsort_impl;
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
void put(const char *msg, const V128 *a, size_t N)
{
	printf("%s\n", msg);
	for (int i = 0; i < (int)N; i++) {
		printf("%2d", i);
		a[i].put(":");
	}
}

void test_merge()
{
	using namespace mie::intsort_impl;
	puts("test_merge");
	{
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
	}
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
			std::merge(a, a + N / 2, a + N / 2, a + N, &vo[0]);
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

void test_cmpswap_skew()
{
	using namespace mie::intsort_impl;
	puts("cmpswap_skew");
	V128 a(9, 7, 4, 3);
	V128 b(6, 5, 2, 1);
	a.put("a=");
	b.put("b=");
	vector_cmpswap_skew(a, b);
	puts("compswap_skew");
	a.put("a=");
	b.put("b=");
}

int main(int argc, char *argv[])
{
	argc--, argv++;
	if (argc > 0 && strcmp(*argv, "-test") == 0) {
		test_vector_merge();
		test_cmpswap_skew();
		test_merge();
		return 0;
	}
	int mode = -1;
	size_t BN = 8;
	while (argc > 0) {
		if (argc > 1 && strcmp(*argv, "-m") == 0) {
			argc--, argv++;
			mode = atoi(*argv);
		} else
		if (argc > 1 && strcmp(*argv, "-b") == 0) {
			argc--, argv++;
			BN = atoi(*argv);
		} else
		{
			fprintf(stderr, "intsort_test [-m <mode>] [-b <num>]\n");
			fprintf(stderr, " -b <num> : BN = 1024 * <num>\n");
			return 1;
		}
		argc--, argv++;
	}
	BN *= 1024;
	fprintf(stderr, "mode=%d, BN=%d\n", mode, (int)BN);
	if (mode >= (int)NUM_OF_ARRAY(modeTbl)) {
		fprintf(stderr, "too large mode=%d\n", mode);
		return 1;
	}
	printf("%8s %10s %10s %4s\n", "N", "STL", "SSE", "rate");
	puts("test");
	for (int m = 0; m < (int)NUM_OF_ARRAY(modeTbl); m++) {
		if (mode >= 0 && mode != m) continue;
		printf("mode=%s\n", modeTbl[m]);
		for (int i = 0; i < 20; i++) {
			const size_t N = 16 * (1U << i);
			AlignedArray<uint32_t> va(N);
			uint32_t *const a = &va[0];
			Init(a, N, m);
			double c1 = test(STLsort, a, N, BN);
			Init(a, N, m);
			double c2 = test(mie::intsort, a, N, BN);
			printf("%8d %11.2f %11.2f %.2f\n", (int)N, c1, c2, c1 / c2);
		}
	}
}
