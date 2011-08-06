/*
	require http://homepage1.nifty.com/herumi/soft/xbyak_e.html
	g++ -O3 -fomit-frame-pointer -march=core2 -msse4 -fno-operator-names strlen_sse42.cpp && ./a.out

Xeon X5650 2.67GHz + Linux 2.6.32 + gcc 4.6.0
strchrLIBC
ret=32132, 0.851
strchr_C
ret=32132, 3.157
strchrSSE42_C
ret=32132, 0.479
strchrSSE42
ret=32132, 0.495

Core i7-2600 CPU 3.40GHz + Linux 2.6.35 + gcc 4.4.5
strchrLIBC
ret=32132, 0.854
strchr_C
ret=32132, 3.759
strchrSSE42_C
ret=32132, 0.482
strchrSSE42
ret=32132, 0.492

Core i7-2600 CPU 3.40GHz + Windows 7 + VC2010
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
#else
	#include <x86intrin.h>
#endif

const int MaxChar = 254;

const char *strchr_C(const char *p, int c)
{
	while (*p) {
		if (*p == (char)c) return p;
		p++;
	}
	return 0;
}

struct StrchrSSE42 : Xbyak::CodeGenerator {
	StrchrSSE42()
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
		const Reg64& c1 = rsi;
		const Reg64& c = rcx;
		const Reg64& a = rax;
#else
		const Reg32& p = edx;
		const Reg32& c = ecx;
		const Reg32& a = eax;
		mov(edx, ptr [esp + 4]);
#endif
		mov(a, c1);
		and(a, 0xff);
		movd(xm0, eax);

		mov(a, p);
		jmp(".in");
	L("@@");
		add(a, 16);
	L(".in");
		pcmpistri(xm0, ptr [a], 0);
		ja("@b");
		jnc(".notfound");
		add(a, c);
		ret();
	L(".notfound");
		xor(a, a);
		ret();
		outLocalLabel();
	}
} strchrSSE42_code;

const char *strchrSSE42_C(const char* p, int c)
{
	const __m128i im = _mm_set1_epi32(c & 0xff);
	while (_mm_cmpistra(im, *(const __m128i*)p, 0)) {
		p += 16;
	}
	if (_mm_cmpistrc(im, *(const __m128i*)p, 0)) {
		return p + _mm_cmpistri(im, *(const __m128i*)p, 0);
	}
	return 0;
}

void test(const char *str, const char *f(const char*, int))
{
	Xbyak::util::Clock clk;
	int ret = 0;
	const int count = 30000;
	for (int i = 0; i < count; i++) {
		clk.begin();
		for (int c = 1; c <= MaxChar; c++) {
			const char *p = f(str, c);
#if 1
			const char *q = strchr(str, c);
			if (p != q) {
				printf("err, c=%d, p=%p(%d), q=%p(%d)\n", c, p, int(p - str), q, int(q - str));
				exit(1);
			}
#endif
			if (p) {
				ret += p - str;
			} else {
				ret += MaxChar;
			}
		}
		clk.end();
	}
	printf("ret=%d, %.3f\n", ret / count, clk.getClock() / (double)ret);
}

const char* (*strchrSSE42)(const char*, int) = (const char* (*)(const char*, int))strchrSSE42_code.getCode();

int main(int argc, char *argv[])
{
	char str[MaxChar + 1];
	for (int i = 1; i < MaxChar; i++) {
		str[i - 1] = (char)i;
	}
	str[MaxChar] = '\0';

	static const struct {
		const char *name;
		const char *(*f)(const char*, int);
	} funcTbl[] = {
		{ "strchrLIBC   ", strchr },
		{ "strchr_C     ", strchr_C },
		{ "strchrSSE42_C", strchrSSE42_C },
		{ "strchrSSE42  ", strchrSSE42 },
	};

	for (size_t j = 0; j < NUM_OF_ARRAY(funcTbl); j++) {
		puts(funcTbl[j].name);
		test(str, funcTbl[j].f);
	}
}

