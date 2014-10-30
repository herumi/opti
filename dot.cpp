/*
	nehalem 10clk
	sandy 7.5clk
	ivy 7.6clk
	has 7.78clk
*/
#include <stdio.h>
#include <xbyak/xbyak_util.h>

MIE_ALIGN(16) float a[4] = { 1, 2, 3, 4 };
MIE_ALIGN(16) float b[4] = { 1.2, 3.4, 5.2, -1.2 };

const int N = 100000;

struct Code : Xbyak::CodeGenerator {
	Code()
	{
		mov(ecx, N);
		mov(rax, (size_t)a);
		mov(rdx, (size_t)b);
		xorps(xm0, xm0);
	L("@@");
		movaps(xm1, ptr [rax]);
		dpps(xm1, ptr [rdx], 0xf1);
		addps(xm0, xm1);
		sub(ecx, 1);
		jnz("@b");
		ret();
	}
};

int main()
{
	Code c;
	void (*f)() = c.getCode<void (*)()>();
	Xbyak::util::Clock clk;
	for (int i = 0; i < 100; i++) {
		clk.begin();
		f();
		clk.end();
	}
	printf("%.2f\n", clk.getClock() / double(N) / clk.getCount());
}