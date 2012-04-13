/*
	require http://homepage1.nifty.com/herumi/soft/xbyak_e.html

	benchmark of jmp and not jmp
	g++ -O3 -fomit-frame-pointer -fno-operator-names jmp.cpp

Pentium D
a=0, 2.769
a=1, 2.693
a, b=0, 1, 12.416
a, b=1, 0, 12.500
a, b=0, 1, 12.276
a, b=1, 0, 12.309
a, b=0, 1, 12.213
a, b=1, 0, 12.632
a, b=0, 1, 12.204
a, b=1, 0, 12.439

Core Duo
a=0, 1.954
a=1, 1.955
a, b=0, 1, 9.084
a, b=1, 0, 7.072
a, b=0, 1, 10.101
a, b=1, 0, 7.072
a, b=0, 1, 8.050
a, b=1, 0, 9.015
a, b=0, 1, 8.011
a, b=1, 0, 9.086

Xeon X5650
a=0, 1.741
a=1, 1.741
a, b=0, 1, 7.842
a, b=1, 0, 6.094
a, b=0, 1, 7.839
a, b=1, 0, 6.101
a, b=0, 1, 6.101
a, b=1, 0, 7.832
a, b=0, 1, 6.109
a, b=1, 0, 7.834

i7 2600K
a=0, 1.810
a=1, 1.801
a, b=0, 1, 7.201
a, b=1, 0, 5.405
a, b=0, 1, 7.185
a, b=1, 0, 5.417
a, b=0, 1, 5.401
a, b=1, 0, 7.200
a, b=0, 1, 5.395
a, b=1, 0, 7.190
*/
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "xbyak/xbyak.h"
#include "xbyak/xbyak_util.h"
#include "util.hpp"

const int N = 1000000;

struct Code : public Xbyak::CodeGenerator {
	// void loop(int a);
	Code()
	{
		using namespace Xbyak;
		inLocalLabel();
#if defined(XBYAK64_WIN)
		mov(rax, rcx);
#elif defined(XBYAK64_GCC)
		mov(rax, rdi);
#else
		mov(eax, ptr [esp + 4]);
#endif
		mov(ecx, N);
	L(".lp");
		test(eax, eax);
		jz(".jmp");
	L(".jmp");
		sub(ecx, 1);
		jnz(".lp");
		ret();
		outLocalLabel();
	}
};

/*
int f(int a, int b)
{
	if (__builtin_expect(a > b, 1)) {
		return a;
	}
	return a + b;
}

int g(int a, int b)
{
	if (__builtin_expect(a > b, 0)) {
		return a;
	}
	return a + b;
}
*/

struct ExpectCode : public Xbyak::CodeGenerator {
	void gen(bool likely, bool hint)
	{
		using namespace Xbyak;
		inLocalLabel();
#if defined(XBYAK64_WIN)
		const Reg64& p1 = rcx;
		const Reg64& p2 = rdx;
		const Reg64& a = rax;
#elif defined(XBYAK64_GCC)
		const Reg64& p1 = rdi;
		const Reg64& p2 = rsi;
		const Reg64& a = rax;
#else
		const Reg32& p1 = ecx;
		const Reg32& p2 = edx;
		const Reg32& a = eax;
		mov(ecx, ptr [esp + 4]);
		mov(edx, ptr [esp + 8]);
#endif
		cmp(p1, p2);
		mov(a, p1);
		if (likely) {
			jle("@f");
			if (hint) db(0xf3);
			ret();
		L("@@");
			add(a, p2);
			ret();
		} else {
			jg("@f");
			add(a, p2);
		L("@@");
			if (hint) db(0xf3);
			ret();
		}
		align(16);
	}
};

template<class F>
void test1(F f, int a)
{
	Xbyak::util::Clock clk;
	const int C = 100;
	for (int i = 0; i < C; i++) {
		clk.begin();
		f(a);
		clk.end();
	}
	printf("a=%d, %.3f\n", a, clk.getClock() / double(C * N));
}

template<class F>
void test2(F f, int a, int b)
{
	Xbyak::util::Clock clk;
	clk.begin();
	const int C = 100000000;
	for (int i = 0; i < C; i++) {
		f(a, b);
	}
	clk.end();
	printf("a, b=%d, %d, %.3f\n", a, b, clk.getClock() / double(C));
}

int main()
{
	try {
		Code code;
		void (*f)(int) = (void (*)(int))code.getCode();
		test1(f, 0);
		test1(f, 1);
		ExpectCode c;
		int (*gtt)(int, int) = (int (*)(int, int))c.getCurr();
		c.gen(true, true);
		int (*gtf)(int, int) = (int (*)(int, int))c.getCurr();
		c.gen(true, false);
		int (*gft)(int, int) = (int (*)(int, int))c.getCurr();
		c.gen(false, true);
		int (*gff)(int, int) = (int (*)(int, int))c.getCurr();
		c.gen(false, false);
		test2(gtt, 0, 1);
		test2(gtt, 1, 0);

		test2(gtf, 0, 1);
		test2(gtf, 1, 0);

		test2(gft, 0, 1);
		test2(gft, 1, 0);

		test2(gff, 0, 1);
		test2(gff, 1, 0);

	} catch (Xbyak::Error err) {
		printf("ERR:%s(%d)\n", Xbyak::ConvertErrorToString(err), err);
	} catch (...) {
		printf("unknown error\n");
	}
}
