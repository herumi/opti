#include <stdio.h>
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

const int N = 0x1000000;

struct Code : Xbyak::CodeGenerator {
	explicit Code(int count, int mode)
		: CodeGenerator(1024 * 1024 * 64)
	{
		using namespace Xbyak;
		mov(ecx, N / count);
		xor(eax, eax);
	L(".lp");
		for (int i = 0; i < count; i++) {
			if (mode == 0) {
				add(eax, 1);
			} else {
				L(Label::toStr(i).c_str());
				add(eax, 1);
				int to = 0;
				if (i < count / 2) {
					to = count - 1 - i;
				} else {
					to = count  - i;
				}
				if (i == count / 2) {
					jmp(".exit", T_NEAR);
				} else {
					jmp(Label::toStr(to).c_str(), T_NEAR);
				}
			}
		}
	L(".exit");
		sub(ecx, 1);
		jnz(".lp", T_NEAR);
		ret();
	}
};

void test(int count, int mode)
{
	printf("%6d :", count);
	Code code(count, mode);
	Xbyak::util::Clock clk;
	const int LP = 4;
	int sum = 0;
	for (int i = 0; i < LP; i++) {
		clk.begin();
		sum += ((int (*)())code.getCode())();
		clk.end();
	}
	if (sum != LP * N) printf("err sum=%d, %d\n", sum, LP * N);
	printf("%6.3f, %8.3f\n", clk.getClock() / (double)LP / N, code.getSize() / 1e3);
}


int main()
{
	for (int mode = 0; mode < 2; mode++) {
		printf("[%s]\n", mode == 0 ? "sequential" : "random");
		printf(" count  clk   code size(Kbyte)\n");
		int count = 1;
		for (int i = 0; i < 18; i++) {
			test(count, mode);
			count *= 2;
		}
	}
}

