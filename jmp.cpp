/*
	require http://homepage1.nifty.com/herumi/soft/xbyak_e.html

	benchmark of jmp and not jmp
	g++ -O3 -fomit-frame-pointer -fno-operator-names jmp.cpp

Pentium D
a=0, 2.769
a=1, 2.693
a, b=0, 1, 12.935
a, b=1, 0, 12.512
a, b=0, 1, 12.428
a, b=1, 0, 12.766
a, b=0, 1, 11.934
a, b=1, 0, 11.705
a, b=0, 1, 16.115
a, b=1, 0, 15.930

Core Duo
a=0, 1.954
a=1, 1.955
a, b=0, 1, 9.094
a, b=1, 0, 7.076
a, b=0, 1, 7.063
a, b=1, 0, 9.046
a, b=0, 1, 8.088
a, b=1, 0, 8.097
a, b=0, 1, 8.093
a, b=1, 0, 8.101

Xeon X5650
a=0, 1.741
a=1, 1.741
a, b=0, 1, 7.834
a, b=1, 0, 6.096
a, b=0, 1, 6.090
a, b=1, 0, 7.842
a, b=0, 1, 6.091
a, b=1, 0, 6.096
a, b=0, 1, 6.090
a, b=1, 0, 6.097

i7 2600K
a=0, 1.810
a=1, 1.801
a, b=0, 1, 7.247
a, b=1, 0, 5.391
a, b=0, 1, 5.384
a, b=1, 0, 7.238
a, b=0, 1, 5.426
a, b=1, 0, 5.403
a, b=0, 1, 5.387
a, b=1, 0, 5.381
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
int g1(int a, int b)
{
    if (__builtin_expect(a > b, 1)) {
        return a + 1;
    }
    return a + 2;
}

int g2(int a, int b)
{
    if (__builtin_expect(a > b, 0)) {
        return a + 1;
    }
    return a + 2;
}
*/

int g3_C(int a, int b)
{
    if (a > b) {
        return a + 1;
    }
    return a + 2;
}

struct ExpectCode : public Xbyak::CodeGenerator {
	void gen(int mode)
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
		switch (mode) {
		case 0:
			jle("@f");
			lea(a, ptr [p1 + 1]);
			ret();
		L("@@");
			lea(a, ptr [p1 + 2]);
			ret();
			break;
		case 1:
			jg("@f");
			lea(a, ptr [p1 + 2]);
			ret();
		L("@@");
			lea(a, ptr [p1 + 1]);
			ret();
			break;
		case 2:
			lea(a, ptr [p1 + 1]);
			cmp(p1, p2);
			lea(p1, ptr [p1 + 2]);
			cmovle(a, p1);
			ret();
		default:
			cmp(p2, p1);
			sbb(a, a);
			lea(a, ptr [p1 + 2 + a]);
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

template<class F>
void check(F f)
{
	for (int a = 0; a < 5; a++) {
		for (int b = 0; b < 5; b++) {
			int c = f(a, b);
			int d = g3_C(a, b);
			if (c != d) {
				printf("ERR (a, b)=(%d, %d) %d %d\n", a, b, c, d);
			}
		}
	}
	puts("ok");
}

int main()
{
	try {
		Code code;
		void (*f)(int) = (void (*)(int))code.getCode();
		test1(f, 0);
		test1(f, 1);
		ExpectCode c;
		int (*g1)(int, int) = (int (*)(int, int))c.getCurr();
		c.gen(0);
		int (*g2)(int, int) = (int (*)(int, int))c.getCurr();
		c.gen(1);
		int (*g3)(int, int) = (int (*)(int, int))c.getCurr();
		c.gen(2);
		int (*g4)(int, int) = (int (*)(int, int))c.getCurr();
		c.gen(3);
		check(g1);
		check(g2);
		check(g3);
		check(g4);

		test2(g1, 0, 1);
		test2(g1, 1, 0);

		test2(g2, 0, 1);
		test2(g2, 1, 0);

		test2(g3, 0, 1);
		test2(g3, 1, 0);

		test2(g4, 0, 1);
		test2(g4, 1, 0);
	} catch (Xbyak::Error err) {
		printf("ERR:%s(%d)\n", Xbyak::ConvertErrorToString(err), err);
	} catch (...) {
		printf("unknown error\n");
	}
}
