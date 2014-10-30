#include <stdio.h>
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

const int N = 1000000;

struct Code : public Xbyak::CodeGenerator {
	Code()
	{
		mov(eax, N);
	L("@@");
		sub(eax, 1);
		jnz("@b");
		ret();
	}
};


int main()
{
	const int count = 1000;
	Xbyak::util::Clock clk;
	Code c;
	void (*f)() = (void (*)())c.getCode();
	for (int i = 0; i < count; i++) {
		clk.begin();
		f();
		clk.end();
	}
	printf("%.3fclk\n", clk.getClock() / double(N) / clk.getCount());
}