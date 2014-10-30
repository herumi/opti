#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

const int N = 100000;
const int X = 5;
const int Y = 12;

struct Code : public Xbyak::CodeGenerator {
	explicit Code(int mode)
	{
		push(esi);
		push(ebx);
		mov(esi, N);
		mov(eax, X);
		mov(ecx, Y);
	L("@@");
		if (mode == 0) {
			lea(ebx, ptr [ecx + ecx * 2]);
			movzx(ecx, bl);
			lea(edx, ptr [eax + eax * 4]);
			movzx(eax, dl);
			lea(ebx, ptr [ecx + ecx * 2]);
			movzx(ecx, bl);
			lea(edx, ptr [eax + eax * 4]);
			movzx(eax, dl);
		} else {
			lea(ebx, ptr [ecx + ecx * 2]);
			movzx(ecx, bl);
			lea(ebx, ptr [ecx + ecx * 2]);
			movzx(ecx, bl);
			lea(edx, ptr [eax + eax * 4]);
			movzx(eax, dl);
			lea(edx, ptr [eax + eax * 4]);
			movzx(eax, dl);
		}
		sub(esi, 2);
		jg("@b");
		pop(ebx);
		pop(esi);
		ret();
	}
};

struct Test {
	explicit Test(int mode)
	{
		Code c(mode);
		int (*f)() = (int (*)())c.getCode();
		const int M = 10000;
		Xbyak::util::Clock clk;
		int ret = 0;
		for (int i = 0; i < M; i++) {
			clk.begin();
			ret += f();
			clk.end();
		}
		printf("ret=%08x %.3fclk\n", ret, clk.getClock() / double(M) / double(N));
	}
};

int main()
{
	Test(0);
	Test(1);
	Test(0);
	Test(1);
}

