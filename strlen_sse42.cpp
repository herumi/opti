/*
	require http://homepage1.nifty.com/herumi/soft/xbyak_e.html
	g++ -O3 -fomit-frame-pointer -march=core2 -msse4 -fno-operator-names strlen_sse42.cpp && ./a.out

Xeon X5650 2.67GHz + Linux 2.6.32 + gcc 4.6.0
ave             2.00   5.04   7.03   9.95  12.03  16.35  20.04  33.08  66.62 132.98 261.78 518.13 1063.83
strlenLIBC     11.74   5.00   3.99   3.21   2.84   2.30   2.05   1.42   0.85   0.55   0.38   0.29   0.24
strlenC        13.94   8.18   6.69   5.43   4.87   4.16   3.76   3.08   2.58   2.29   2.17   2.05   2.02
strlenSSE2     12.72   6.30   4.97   3.81   3.27   2.55   2.20   1.53   0.94   0.56   0.36   0.25   0.19
strlenSSE42     8.84   3.73   3.01   2.59   2.44   2.16   1.99   1.46   0.84   0.54   0.35   0.27   0.21
strlenSSE42_C   8.93   3.76   3.03   2.59   2.43   2.12   1.94   1.42   0.84   0.54   0.35   0.26   0.21

Core i7-2600 CPU 3.40GHz + Linux 2.6.35 + gcc 4.4.5
ave             2.00   5.04   7.03   9.95  12.03  16.35  20.04  33.08  66.62 132.98 261.78 518.13 1063.83
strlenLIBC     11.58   5.69   4.59   3.55   3.04   2.37   2.03   1.21   0.56   0.36   0.27   0.22   0.20
strlenC        14.23   6.84   5.39   4.33   3.98   3.32   3.03   2.44   1.89   1.58   1.40   1.29   1.25
strlenSSE2     12.20   5.70   4.39   3.33   2.83   2.11   1.74   1.07   0.53   0.30   0.22   0.18   0.15
strlenSSE42    12.19   4.99   3.89   3.17   2.87   2.48   2.18   1.31   0.68   0.42   0.30   0.24   0.21
strlenSSE42_C  12.41   5.10   3.97   3.25   2.97   2.53   2.19   1.29   0.67   0.40   0.29   0.23   0.21

Core i7-2600 CPU 3.40GHz + Windows 7 + VC2010
ave             2.00   5.04   7.03   9.95  12.03  16.35  20.04  33.08  66.62 132.98 261.78 518.13 1063.83
strlenLIBC     16.10   8.20   6.41   5.12   4.52   3.54   2.89   1.78   0.96   0.60   0.45   0.38   0.34
strlenC        13.99   6.70   5.31   4.26   3.90   3.27   2.98   2.40   1.89   1.58   1.40   1.30   1.25
strlenSSE2     11.27   5.33   4.08   3.08   2.59   1.94   1.63   1.01   0.48   0.27   0.19   0.16   0.13
strlenSSE42    12.34   5.09   3.96   3.24   2.97   2.54   2.20   1.32   0.67   0.40   0.29   0.24   0.21
strlenSSE42_C  12.39   5.08   3.98   3.24   2.98   2.62   2.35   1.59   0.73   0.43   0.32   0.25   0.22
*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <vector>
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>
#include "util.hpp"

#ifdef _WIN32
	#include <intrin.h>
	#define ALIGN(x) __declspec(align(x))
	#define bsf(x) (_BitScanForward(&x, x), x)
	#define bsr(x) (_BitScanReverse(&x, x), x)
#else
	#include <x86intrin.h>
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
		inLocalLabel();
		using namespace Xbyak;
#if defined(XBYAK64_WIN)
		const Reg64& p = rdx;
		const Reg64& c = rcx;
		const Reg64& a = rax;
		mov(rdx, rcx);
#elif defined(XBYAK64_GCC)
		const Reg64& p = rdi;
		const Reg64& c = rcx;
		const Reg64& a = rax;
#else
		const Reg32& p = edx;
		const Reg32& c = ecx;
		const Reg32& a = eax;
		mov(edx, ptr [esp + 4]);
#endif
		mov(eax, 0xff01);
		movd(xm0, eax);

#if 0 // generated code by gcc 4.6.0
		lea(rdx, ptr [p - 16]);
		xor(c, c);
		jmp(".skip");
		align(16);
	L("@@");
		mov(rdx, rax);
	L(".skip");
		lea(rax, ptr [edx + 16]);
		pcmpistri(xm0, ptr [edx + 16], 0x14);
		jnz("@b");
		add(a, c);
		sub(a, p);
		ret();
#else
#if 0
		lea(a, ptr [p - 16]);
#else
		mov(a, p);
		jmp(".in");
#endif
	L("@@");
		add(a, 16);
	L(".in");
		pcmpistri(xm0, ptr [a], 0x14);
		jnz("@b");
		add(a, c);
		sub(a, p);
		ret();
#endif
		outLocalLabel();
	}
} strlenSSE42_code;

size_t strlenSSE42_C(const char* top)
{
	const __m128i im = _mm_set1_epi32(0xff01);
	const char *p = top;
	while (!_mm_cmpistrz(im, _mm_loadu_si128((const __m128i*)p), 0x14)) {
		p += 16;
	}
	p += _mm_cmpistri(im, _mm_loadu_si128((const __m128i*)p), 0x14);
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
	XorShift128 r;
	int c = 1;
	for (size_t i = 0; i < num; i++) {
		p[i] = (char)c++;
		if (c == 256) c = 1;
		if ((r.get() % ave) == 0) p[i] = 0;
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

int main()
{
	const size_t count = 4000;
	const size_t N = 100000;
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

	printf("ave           ");
	for (size_t i = 0; i < NUM_OF_ARRAY(aveTbl); i++) {
		printf("%6.2f ", rv[0][i].len);
	}
	printf("\n");
	for (size_t i = 0; i < NUM_OF_ARRAY(funcTbl); i++) {
		printf("%s ", funcTbl[i].name);
		for (size_t j = 0; j < NUM_OF_ARRAY(aveTbl); j++) {
			printf("%6.2f ", rv[i][j].time);
		}
		printf("\n");
	}
}

