/*
	require http://homepage1.nifty.com/herumi/soft/xbyak_e.html
	g++ -O3 -fomit-frame-pointer -march=core2 -msse4 -fno-operator-names strlen_sse42.cpp && ./a.out

Xeon X5650 2.67GHz + Linux 2.6.32 + gcc 4.6.0
strchrLIBC
ret=32132, 0.468
strchr_C
ret=32132, 2.526
strchrSSE42_C
ret=32132, 0.249
strchrSSE42
ret=32132, 0.246
aligned str      unaligned str
findRange_C      findRange_C
ret=32132, 2.185 ret=32132, 2.227
findRange2_C     findRange2_C
ret=32132, 1.980 ret=32132, 1.980
findRangeSSE42_C findRangeSSE42_C
ret=32132, 0.259 ret=32132, 0.256

Core i7-2600 CPU 3.40GHz + Linux 2.6.35 + gcc 4.4.5
strchrLIBC
ret=32132, 0.261
strchr_C
ret=32132, 4.009
strchrSSE42_C
ret=32132, 0.238
strchrSSE42
ret=32132, 0.276
aligned str       unaligned str
findRange_C       findRange_C
ret=32132, 2.439  ret=32132, 2.441
findRange2_C      findRange2_C
ret=32132, 2.126  ret=32132, 2.131
findRangeSSE42_C  findRangeSSE42_C
ret=32132, 0.240  ret=32132, 0.243
findRangeSSE42    findRangeSSE42
ret=32132, 0.263  ret=32132, 0.265

Core i7-2600 CPU 3.40GHz + Windows 7 + VC2010
strchrLIBC
ret=32132, 2.142
strchr_C
ret=32132, 2.228
strchrSSE42_C
ret=32132, 0.239
strchrSSE42
ret=32132, 0.218
aligned str      unaligned str
findRange_C      findRange_C
ret=32132, 3.087 ret=32132, 3.083
findRange2_C     findRange2_C
ret=32132, 2.235 ret=32132, 2.247
findRangeSSE42_C findRangeSSE42_C
ret=32132, 0.231 ret=32132, 0.240
*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <vector>
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>
#include "util.hpp"

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
	// const char *strchr(const char *p, int c1);
	StrchrSSE42()
	{
		inLocalLabel();
		using namespace Xbyak;

#ifdef XBYAK64

#if defined(XBYAK64_WIN)
		const Reg64& p = rcx;
		const Reg64& c1 = rdx;
#elif defined(XBYAK64_GCC)
		const Reg64& p = rdi;
		const Reg64& c1 = rsi;
#endif
		const Reg64& c = rcx;
		const Reg64& a = rax;
		and(c1, 0xff);
		movq(xm0, c1);
		mov(a, p);
#else
		const Reg32& a = eax;
		const Reg32& c = ecx;
		movzx(eax, byte [esp + 8]);
		movd(xm0, eax);
		mov(a, ptr [esp + 4]);
#endif
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
	while (_mm_cmpistra(im, _mm_loadu_si128((const __m128i*)p), 0)) {
		p += 16;
	}
	if (_mm_cmpistrc(im, _mm_loadu_si128((const __m128i*)p), 0)) {
		return p + _mm_cmpistri(im, _mm_loadu_si128((const __m128i*)p), 0);
	}
	return 0;
}

const char *findRangeSSE42_C(const char* p, char c1, char c2)
{
	const __m128i im = _mm_set1_epi32(((unsigned char)c1) | (((unsigned char)c2) << 8));
	while (_mm_cmpistra(im, _mm_loadu_si128((const __m128i*)p), 4)) {
		p += 16;
	}
	if (_mm_cmpistrc(im, _mm_loadu_si128((const __m128i*)p), 4)) {
		return p + _mm_cmpistri(im, _mm_loadu_si128((const __m128i*)p), 4);
	}
	return 0;
}

const char *findRange_C(const char* p, char c1, char c2)
{
	while (*p) {
		if ((unsigned char)c1 <= (unsigned char)*p && (unsigned char)*p <= (unsigned char)c2) return p;
		p++;
	}
	return 0;
}

const char *findRange2_C(const char* p, char c1, char c2)
{
	while (*p) {
		if ((unsigned char)(*p - c1) <= (unsigned char)(c2 - c1)) return p;
		p++;
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
#if 0
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

void test2(const char *str, const char *f(const char*, char,char))
{
	Xbyak::util::Clock clk;
	int ret = 0;
	const int count = 30000;
	for (int i = 0; i < count; i++) {
		clk.begin();
		for (int c = 1; c <= MaxChar; c++) {
			const char *p = f(str, c, MaxChar);
#if 0
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

#ifdef _WIN32
#include <windows.h>
const char *getBoundary(bool canAccesss)
{
	DWORD old;
	const int size = 4096;
	char* top = (char*)VirtualAlloc(0, 8192, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	printf("access %s\n", canAccesss ? "ok" : "ng");
	if (!canAccesss) {
		VirtualProtect(top + size, size, PAGE_NOACCESS, &old);
	}
	char *const base = top + size - 16;
	for (int i = 0; i < 15; i++) {
		base[i] = 'x';
	}
	base[15] = '\0';
	return base;
}
#else
const char *getBoundary(bool)
{
	return 0;
}
#endif

void checkBoundary(char c)
{
	const char *base = getBoundary(c == '0');
	if (base == 0) return;

	for (int i = 0; i < 15; i++) {
		const char *p = strchrSSE42(base + i, 'x');
		if (p != base + i) {
			printf("err p=%p\n", p);
		}
		const char *q = strchrSSE42(base + i, 'y');
		if (q != 0) {
			printf("err q=%p\n", q);
		}
	}
}

int main(int argc, char *argv[])
{
	const Xbyak::util::Cpu cpu;
	const bool hasSSE42 = cpu.has(Xbyak::util::Cpu::tSSE42);

	if (argc == 3 && strcmp(argv[1], "-check") == 0) {
		checkBoundary(argv[2][0]);
		return 0;
	}

	MIE_ALIGN(16) char str[MaxChar + 1];
	for (int i = 1; i < MaxChar; i++) {
		str[i - 1] = (char)i;
	}
	str[MaxChar] = '\0';
	MIE_ALIGN(16) char str_p1[MaxChar + 2];
	char *const str2 = str_p1 + 1;
	for (int i = 1; i < MaxChar; i++) {
		str2[i - 1] = (char)i;
	}
	str2[MaxChar] = '\0';

	static const struct {
		const char *name;
		bool useSSE42;
		const char *(*f)(const char*, int);
	} funcTbl[] = {
		{ "strchrLIBC   ", false, strchr },
		{ "strchr_C     ", false, strchr_C },
		{ "strchrSSE42_C", true, strchrSSE42_C },
		{ "strchrSSE42  ", true, strchrSSE42 },
	};

	for (size_t j = 0; j < NUM_OF_ARRAY(funcTbl); j++) {
		if (funcTbl[j].useSSE42 && !hasSSE42) continue;
		puts(funcTbl[j].name);
		test(str, funcTbl[j].f);
	}
	static const struct {
		const char *name;
		bool useSSE42;
		const char *(*f)(const char*, char,char);
	} funcTbl2[] = {
		{ "findRange_C", false, findRange_C },
		{ "findRange2_C", false, findRange2_C },
		{ "findRangeSSE42_C", true, findRangeSSE42_C },
	};

	puts("aligned str");
	for (size_t j = 0; j < NUM_OF_ARRAY(funcTbl2); j++) {
		if (funcTbl2[j].useSSE42 && !hasSSE42) continue;
		puts(funcTbl2[j].name);
		test2(str, funcTbl2[j].f);
	}
	puts("unaligned str");
	for (size_t j = 0; j < NUM_OF_ARRAY(funcTbl2); j++) {
		if (funcTbl2[j].useSSE42 && !hasSSE42) continue;
		puts(funcTbl2[j].name);
		test2(str2, funcTbl2[j].f);
	}
}

