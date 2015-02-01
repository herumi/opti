#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>
#include <cybozu/benchmark.hpp>

char str[] = "abcdefghijklmn";

const int N = 100;

struct Code : Xbyak::CodeGenerator {
	Code(int b)
	{
#ifdef XBYAK64
		const Xbyak::Reg64& p = rcx;
#else
		const Xbyak::Reg32& p = ecx;
#endif
		mov(p, (size_t)str);
		push(ebx);
		xor_(ebx, ebx);
		mov(eax, N);
	L("@@");
		for (int i = 0; i < 4; i++) {
			switch (b) {
			case 0:
				mov(edx, ptr [p + i]);
				add(ebx, edx);
				mov(edx, ptr [p + i + 3]);
				add(ebx, edx);
				break;
			case 1:
				mov(edx, ptr [p + i]);
				add(ebx, edx);
				mov(edx, ptr [p + i + 4]);
				add(ebx, edx);
				break;
			case 2:
				mov(edx, ptr [p + i]);
				add(ebx, edx);
				movzx(edx, word [p + i + 4]);
				add(ebx, edx);
				movzx(edx, byte [p + i + 4 + 2]);
				shl(edx, 16);
				add(ebx, edx);
			}
		}
		dec(eax);
		jnz("@b");
		pop(ebx);
		ret();
	}
};


int main()
{
	Code c0(0);
	Code c1(1);
	Code c2(2);
	void (*f0)() = c0.getCode<void (*)()>();
	void (*f1)() = c1.getCode<void (*)()>();
	void (*f2)() = c2.getCode<void (*)()>();
	CYBOZU_BENCH("c0", f0);
	CYBOZU_BENCH("c1", f1);
	CYBOZU_BENCH("c2", f2);
}
