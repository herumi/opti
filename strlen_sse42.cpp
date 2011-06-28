/*
	require http://homepage1.nifty.com/herumi/soft/xbyak_e.html
	g++ -O3 -fomit-frame-pointer -march=core2 -msse4 -fno-operator-names strlen_sse42.cpp && ./a.out

	Xeon X5650 2.67GHz + Linux 2.6.32 + gcc 4.6.0
	ave           1.98   4.96   6.76   9.75  11.60  15.23  18.86  28.65  51.63  84.67 127.23 175.75 197.63
	strlenLIBC   11.78   5.00   4.06   3.22   2.92   2.42   2.12   1.58   1.02   0.71   0.51   0.38   0.34
	strlenC      14.06   8.40   6.96   5.59   4.95   4.26   3.83   3.27   2.71   2.46   2.31   2.19   2.19
	strlenSSE2   12.77   6.40   5.11   3.92   3.42   2.77   2.35   1.73   1.15   0.79   0.52   0.33   0.28
	strlenSSE42   9.26   3.98   3.23   2.70   2.50   2.24   2.03   1.59   1.01   0.69   0.51   0.35   0.31

	Core i7-2600 CPU 3.40GHz + Linux 2.6.35 + gcc 4.4.5
	ave           1.98   4.96   6.76   9.75  11.60  15.23  18.86  28.65  51.63  84.67 127.23 175.75 197.63
	strlenLIBC   11.68   5.67   4.60   3.61   3.11   2.54   2.14   1.43   0.72   0.46   0.36   0.30   0.30
	strlenC      14.47   6.95   5.55   4.37   3.94   3.42   3.07   2.56   2.07   1.76   1.60   1.51   1.46
	strlenSSE2   12.30   5.74   4.58   3.42   2.97   2.35   1.92   1.26   0.71   0.40   0.30   0.24   0.23
	strlenSSE42  12.56   5.26   4.12   3.29   2.95   2.59   2.31   1.56   0.83   0.52   0.39   0.32   0.29


	Core i7-2600 CPU 3.40GHz + Windows 7 + VC2010
	ave           2.01   5.01   6.97  10.04  12.00  16.11  19.92  31.48  64.47 135.69 263.85 490.20 877.19
	strlenLIBC   16.42   8.31   6.46   5.09   4.46   3.58   2.92   1.85   0.96   0.59   0.45   0.39   0.36
	strlenC      14.29   6.83   5.44   4.30   3.90   3.33   3.00   2.46   1.90   1.56   1.37   1.29   1.24
	strlenSSE2   11.27   5.35   4.09   3.02   2.62   1.99   1.64   1.06   0.51   0.27   0.19   0.15   0.14
	strlenSSE42  12.64   5.27   4.10   3.26   3.03   2.58   2.27   1.39   0.69   0.40   0.30   0.24   0.21

*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <vector>
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>
#ifdef _WIN32
	#include <intrin.h>
	#define ALIGN(x) __declspec(align(x))
	#define bsf(x) (_BitScanForward(&x, x), x)
	#define bsr(x) (_BitScanReverse(&x, x), x)
#else
	#include <xmmintrin.h>
	#include <smmintrin.h>
	#define ALIGN(x) __attribute__((aligned(x)))
	#define bsf(x) __builtin_ctz(x)
#endif

size_t strlenSSE2(const char *p)
{
	const char *const top = p;
	__m128i c16 = _mm_set1_epi8(0);
	/* 16 byte alignment */
	size_t ip = reinterpret_cast<size_t>(p);
	size_t n = ip & 15;
	if (n > 0) {
		ip &= ~15;
		__m128i x = *(const __m128i*)ip;
		__m128i a = _mm_cmpeq_epi8(x, c16);
		unsigned long mask = _mm_movemask_epi8(a);
		mask &= 0xffffffffUL << n;
		if (mask) {
			return bsf(mask) - n;
		}
		p += 16 - n;
	}
	/*
		thanks to egtra-san
	*/
	assert((reinterpret_cast<size_t>(p) & 15) == 0);
	if (reinterpret_cast<size_t>(p) & 31) {
		__m128i x = *(const __m128i*)&p[0];
		__m128i a = _mm_cmpeq_epi8(x, c16);
		unsigned long mask = _mm_movemask_epi8(a);
		if (mask) {
			return p + bsf(mask) - top;
		}
		p += 16;
	}
	assert((reinterpret_cast<size_t>(p) & 31) == 0);
	for (;;) {
		__m128i x = *(const __m128i*)&p[0];
		__m128i y = *(const __m128i*)&p[16];
		__m128i a = _mm_cmpeq_epi8(x, c16);
		__m128i b = _mm_cmpeq_epi8(y, c16);
		unsigned long mask = (_mm_movemask_epi8(b) << 16) | _mm_movemask_epi8(a);
		if (mask) {
			return p + bsf(mask) - top;
		}
		p += 32;
	}
}

struct StrlenSSE42 : Xbyak::CodeGenerator {
	StrlenSSE42()
	{
		using namespace Xbyak;
#if defined(XBYAK64_WIN)
		const Reg64& p1 = rdx;
		const Reg64& c = rcx;
		const Reg64& a = rax;
		mov(rdx, rcx);
#elif defined(XBYAK64_GCC)
		const Reg64& p1 = rdi;
		const Reg64& c = rcx;
		const Reg64& a = rax;
#else
		const Reg32& p1 = edx;
		const Reg32& c = ecx;
		const Reg32& a = eax;
		mov(edx, ptr [esp + 4]);
#endif
		mov(eax, 0xff01);
		movd(xm2, eax);

		lea(a, ptr [p1 - 16]);
		xor(c, c);
	L("@@");
		add(a, 16);
		pcmpistri(xm2, ptr [a], 0x14);
		jnz("@b");
		add(a, c);
		sub(a, p1);
		ret();
	}
} strlenSSE42_code;

size_t strlenSSE42_C(const char* top)
{
	const __m128i im = _mm_set1_epi32(0xff01);
	const char *p = top - 16;
	do {
		p += 16;
	} while (!_mm_cmpistrz(im, *(__m128i*)p, 0x14));
	p += _mm_cmpistri(im, *(__m128i*)p, 0x14);
	return p - top;
}


struct Result {
	int hit;
	double len;
	double time;
	Result() {}
	Result(int hit, double len, double time) : hit(hit), len(len), time(time) {}
	void put() const
	{
		printf("hit=%d len=%.2f time= %.2f clk\n", hit, len, time);
	}
};

void createTable(char *p, size_t num, int ave)
{
	char c = 1;
	for (size_t i = 0; i < num; i++) {
		p[i] = c++;
		if (c == 0) c = 1;
		if ((rand() % ave) == 0) p[i] = 0;
	}
	p[num - 1] = 0;
}

Result test(const char *top, size_t n, size_t count, size_t func(const char*))
{
	Xbyak::util::Clock clk;
	int hit = 0;
	for (size_t i = 0; i < count; i++) {
		clk.begin();
		const char *p = top;
		int remain = n;
		while (remain > 0) {
			size_t len = func(p);
			hit++;
			remain -= len + 1;
			p += len + 1;
		}
		clk.end();
	}
	return Result(hit, n / (double)hit * count, clk.getClock() / (double)count / n);
}

size_t strlenC(const char *p)
{
	const char *top = p;
	while (*p) {
		p++;
	}
	return p - top;
}

size_t (*strlenSSE42)(const char*) = (size_t (*)(const char*))strlenSSE42_code.getCode();

#define NUM_OF_ARRAY(x) (sizeof(x)/sizeof(x[0]))

int main(int argc, char *argv[])
{
	const size_t count = 4000;
	const size_t N = 100000;
	const int funcNum = 5;
	std::vector<char> v(N);

	char *begin = &v[0];

	static const int aveTbl[] = { 2, 5, 7, 10, 12, 16, 20, 32, 64, 128, 256, 512, 1024 };
	static const struct {
		const char *name;
		size_t (*func)(const char*);
	} funcTbl[] = {
		{ "strlenLIBC   ", strlen },
		{ "strlenC      ", strlenC },
		{ "strlenSSE2   ", strlenSSE2 },
		{ "strlenSSE42  ", strlenSSE42 },
		{ "strlenSSE42_C", strlenSSE42_C },
	};
	Result rv[NUM_OF_ARRAY(funcTbl)][NUM_OF_ARRAY(aveTbl)];

	for (size_t i = 0; i < NUM_OF_ARRAY(aveTbl); i++) {
		int ave = aveTbl[i];
		createTable(begin, N, ave);

		printf("test %d, %d\n", (int)i, ave);
		for (size_t j = 0; j < NUM_OF_ARRAY(funcTbl); j++) {
			puts(funcTbl[j].name);
			rv[j][i] = test(begin, N, count, funcTbl[j].func);
			rv[j][i].put();
			if (rv[j][i].hit != rv[0][i].hit) {
				printf("ERROR!!! ok=%d, ng=%d\n", (int)rv[0][i].hit, (int)rv[j][i].hit);
			}
		}
	}

	puts("end");

	printf("ave         ");
	for (size_t i = 0; i < NUM_OF_ARRAY(aveTbl); i++) {
//		printf("%6d ", aveTbl[i]);
		printf("%6.2f ", rv[0][i].len);
	}
	printf("\n");
	for (int i = 0; i < NUM_OF_ARRAY(funcTbl); i++) {
		printf("%s ", funcTbl[i].name);
		for (size_t j = 0; j < NUM_OF_ARRAY(aveTbl); j++) {
			printf("%6.2f ", rv[i][j].time);
		}
		printf("\n");
	}
}

