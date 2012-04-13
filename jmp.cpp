/*
	require http://homepage1.nifty.com/herumi/soft/xbyak_e.html

	benchmark of jmp and not jmp
	g++ -O3 -fomit-frame-pointer -fno-operator-names jmp.cpp

Pentium D
a=0, 2.769
a=1, 2.693
a, b=0, 1, 12.352
a, b=1, 0, 12.235
a, b=0, 1, 12.352
a, b=1, 0, 12.589

Core Duo
a=0, 1.954
a=1, 1.955
a, b=0, 1, 9.099
a, b=1, 0, 8.047
a, b=0, 1, 8.084
a, b=1, 0, 9.084

Xeon X5650
a=0, 1.741
a=1, 1.741
a, b=0, 1, 7.833
a, b=1, 0, 6.096
a, b=0, 1, 6.091
a, b=1, 0, 7.850

i7 2600K
a=0, 1.810
a=1, 1.801
a, b=0, 1, 8.092
a, b=1, 0, 6.300
a, b=0, 1, 6.316
a, b=1, 0, 8.103
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

struct ExpectCode : public Xbyak::CodeGenerator {
	void gen(bool likely)
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
		if (likely) {
			jle("@f");
			lea(a, ptr [p1 + 1]);
			ret();
		L("@@");
			lea(a, ptr [p1 + 2]);
			ret();
		} else {
			jg("@f");
			lea(a, ptr [p1 + 2]);
			ret();
		L("@@");
			lea(a, ptr [p1 + 1]);
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
		int (*g1)(int, int) = (int (*)(int, int))c.getCurr();
		c.gen(true);
		int (*g2)(int, int) = (int (*)(int, int))c.getCurr();
		c.gen(false);

		test2(g1, 0, 1);
		test2(g1, 1, 0);

		test2(g2, 0, 1);
		test2(g2, 1, 0);

	} catch (Xbyak::Error err) {
		printf("ERR:%s(%d)\n", Xbyak::ConvertErrorToString(err), err);
	} catch (...) {
		printf("unknown error\n");
	}
}
