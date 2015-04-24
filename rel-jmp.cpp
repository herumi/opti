/*
	for only 64-bit mode

1000 times loop
i5-4310U@2.0GHz(on Windows)
inline       5.509Kclk
call rel32   5.173Kclk
call [rip]   5.424Kclk
call reg     4.981Kclk
call [reg]   5.789Kclk

i7-4770@3.4GHz(on Linux turbo boost off)
inline       5.353Kclk
call rel32   5.152Kclk
call [rip]   5.521Kclk
call reg     5.034Kclk
call [reg]   5.026Kclk

i7-2600@3.4GHz(on Windows)
inline       5.138Kclk
call rel32   5.741Kclk
call [rip]   5.674Kclk
call reg     5.613Kclk
call [reg]   5.644Kclk

X5650@2.6GHz(on Linux)
inline       9.515Kclk
call rel32  10.062Kclk
call [rip]  10.061Kclk
call reg    11.728Kclk
call [reg]  11.779Kclk

i3-2120T@2.60GHz(on Linux)
inline       9.030Kclk
call rel32   9.948Kclk
call [rip]   9.932Kclk
call reg     9.975Kclk
call [reg]  10.084Kclk
*/
#include <stdio.h>
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>
#include <cybozu/benchmark.hpp>

#ifdef XBYAK32
	#error "for only 64-bit mode"
#endif

const int N = 1000;

int (*f0)();
int (*f1)();
int (*f2)();
int (*f3)();
int (*f4)();

struct Code : Xbyak::CodeGenerator {
	static const int c = 12345;
	Code()
		: Xbyak::CodeGenerator(8192)
	{
		Xbyak::Label func, funcAddr;
	L(func);
		calc();
		ret();
		align(16);
	L(funcAddr);
		putL(func);

		align(16);
		f0 = getCurr<int (*)()>();
		mov(ecx, N);
		mov(eax, c);
	L("@@");
		calc();
		sub(ecx, 1);
		jnz("@b");
		ret();

		align(16);
		f1 = getCurr<int (*)()>();
		mov(ecx, N);
		mov(eax, c);
	L("@@");
		call(func);
		sub(ecx, 1);
		jnz("@b");
		ret();

		align(16);
		f2 = getCurr<int (*)()>();
		mov(ecx, N);
		mov(eax, c);
	L("@@");
		call(ptr [rip + funcAddr]);
		sub(ecx, 1);
		jnz("@b");
		ret();

		align(16);
		f3 = getCurr<int (*)()>();
		mov(ecx, N);
		mov(eax, c);
		mov(r8, func);
	L("@@");
		call(r8);
		sub(ecx, 1);
		jnz("@b");
		ret();

		align(16);
		f4 = getCurr<int (*)()>();
		mov(ecx, N);
		mov(eax, c);
		mov(r8, funcAddr);
	L("@@");
		call(ptr[r8]);
		sub(ecx, 1);
		jnz("@b");
		ret();
	}
	void calc()
	{
		mul(eax);
		add(eax, c);
	}
};

int main()
	try
{
	Code code;
	printf("%d\n", f0());
	printf("%d\n", f1());
	printf("%d\n", f2());
	printf("%d\n", f3());
	printf("%d\n", f4());
	CYBOZU_BENCH("inline    ", f0);
	CYBOZU_BENCH("call rel32", f1);
	CYBOZU_BENCH("call [rip]", f2);
	CYBOZU_BENCH("call reg  ", f3);
	CYBOZU_BENCH("call [reg]", f4);
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
}
