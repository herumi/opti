/*
	sde -ast -skl -- nonzero-upper-penalty.exe
*/
#include <stdio.h>
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>
#include <cybozu/benchmark.hpp>

const size_t N = 100;

struct Code : public Xbyak::CodeGenerator {
	explicit Code(bool clear)
	{
		vxorpd(ym0, ym0);
		if (clear) vzeroupper();
		mov(eax, N);
	L("@@");
		addpd(xm0, xm0);
		sub(eax, 1);
		jnz("@b");
		ret();
	}
};

int main(int argc, char *[])
	try
{
	bool clear = argc == 2;
	printf("clear=%d\n", clear);
	Code c(clear);
	void (*f)() = c.getCode<void (*)()>();
	CYBOZU_BENCH("f", f);
} catch (std::exception& e) {
	printf("err %s\n", e.what());
	return 1;
}
