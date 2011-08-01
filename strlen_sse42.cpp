/*
	require http://homepage1.nifty.com/herumi/soft/xbyak_e.html
	g++ -O3 -fomit-frame-pointer -march=core2 -msse4 -fno-operator-names strlen_sse42.cpp && ./a.out

	Xeon X5650 2.67GHz + Linux 2.6.32 + gcc 4.6.0
	ave             1.98   4.96   6.76   9.75  11.60  15.23  18.86  28.65  51.63  84.67 127.23 175.75 197.63
	strlenLIBC     11.87   5.00   4.06   3.20   2.94   2.42   2.11   1.59   1.02   0.70   0.51   0.38   0.33
	strlenC        14.14   8.41   6.90   5.56   5.05   4.32   3.89   3.26   2.72   2.45   2.29   2.19   2.16
	strlenSSE2     12.86   6.42   5.14   3.93   3.42   2.76   2.36   1.72   1.15   0.77   0.51   0.33   0.28
	strlenSSE42     8.87   3.78   3.07   2.61   2.44   2.21   2.02   1.59   1.01   0.70   0.53   0.36   0.31
	strlenSSE42_C   8.98   3.82   3.10   2.62   2.45   2.21   2.01   1.57   1.00   0.71   0.52   0.36   0.31

	Core i7-2600 CPU 3.40GHz + Linux 2.6.35 + gcc 4.4.5
	ave             1.98   4.96   6.76   9.75  11.60  15.23  18.86  28.65  51.63  84.67 127.23 175.75 197.63
	strlenLIBC     11.64   5.66   4.60   3.58   3.17   2.52   2.15   1.41   0.70   0.47   0.36   0.30   0.29
	strlenC        14.25   6.96   5.59   4.36   3.93   3.41   3.08   2.57   2.07   1.78   1.61   1.51   1.50
	strlenSSE2     12.34   5.78   4.56   3.42   2.96   2.34   1.92   1.25   0.70   0.41   0.30   0.24   0.23
	strlenSSE42    12.27   5.09   4.01   3.24   2.92   2.55   2.28   1.55   0.84   0.53   0.39   0.32   0.30
	strlenSSE42_C  12.56   5.23   4.10   3.30   3.01   2.66   2.36   1.56   0.86   0.56   0.42   0.33   0.31

	Core i7-2600 CPU 3.40GHz + Windows 7 + VC2010
	ave             2.01   5.01   6.97  10.04  12.00  16.11  19.92  31.48  64.47 135.69 263.85 490.20 877.19
	strlenLIBC     16.18   8.39   6.55   5.12   4.56   3.57   2.95   1.88   0.95   0.58   0.45   0.39   0.35
	strlenC        14.04   6.71   5.32   4.25   3.81   3.26   2.98   2.44   1.89   1.55   1.40   1.29   1.26
	strlenSSE2     11.23   5.35   4.10   3.05   2.62   1.98   1.65   1.06   0.49   0.27   0.19   0.15   0.14
	strlenSSE42    12.22   5.14   4.01   3.22   2.94   2.57   2.16   1.34   0.68   0.41   0.30   0.24   0.22
	strlenSSE42_C  12.30   5.10   3.97   3.19   2.91   2.57   2.24   1.40   0.68   0.41   0.30   0.25   0.23

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
		movd(xm0, eax);

#if 0 // generated code by gcc 4.6.0
		lea(rdx, ptr [p1 - 16]);
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
		sub(a, p1);
		ret();
#else
//		lea(a, ptr [p1 - 16]);
		mov(a, p1);
		xor(c, c);
		jmp(".in");
	L("@@");
		add(a, 16);
	L(".in");
		pcmpistri(xm0, ptr [a], 0x14);
		jnz("@b");
		add(a, c);
		sub(a, p1);
		ret();
#endif
		outLocalLabel();
	}
} strlenSSE42_code;

size_t strlenSSE42_C(const char* top)
{
	const __m128i im = _mm_set1_epi32(0xff01);
#if 1
	const char *p = top;
	while (!_mm_cmpistrz(im, *(const __m128i*)p, 0x14)) {
		p += 16;
	}
#else
	const char *p = top - 16;
	do {
		p += 16;
	} while (!_mm_cmpistrz(im, *(const __m128i*)p, 0x14));
#endif
	p += _mm_cmpistri(im, *(const __m128i*)p, 0x14);
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

