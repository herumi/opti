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
		for (int i = 0; i < 1; i++) {
			switch (b) {
			case 0:
				mov(edx, ptr [p + i]);
				add(ebx, edx);
				mov(edx, ptr [p + i + 4]);
				add(ebx, edx);
				break;
			case 1:
				mov(edx, ptr [p + i]);
				add(ebx, edx);
				mov(edx, ptr [p + i + 1]);
				add(ebx, edx);
				break;
			case 2:
				mov(edx, ptr [p + i]);
				add(ebx, edx);
				mov(edx, ptr [p + i + 2]);
				add(ebx, edx);
				break;
			case 3:
				mov(edx, ptr [p + i]);
				add(ebx, edx);
				mov(edx, ptr [p + i + 3]);
				add(ebx, edx);
				break;
			case 4:
				mov(edx, ptr [p + i]);
				add(ebx, edx);
				movzx(edx, word [p + i + 4]);
				add(ebx, edx);
				movzx(edx, byte [p + i + 4 + 2]);
				shl(edx, 16);
				add(ebx, edx);
				break;
			case 5:
				mov(edx, ptr [p + i]);
				add(ebx, edx);
				mov(edx, ptr [p + i + 3]);
				and_(edx, 0xffffff);
				add(ebx, edx);
				break;
			}
		}
		dec(eax);
		jnz("@b");
		pop(ebx);
		ret();
	}
};

void test0()
{
	puts("test0");
	Code c0(0), c1(1), c2(2), c3(3), c4(4), c5(5);
	void (*f0)() = c0.getCode<void (*)()>();
	void (*f1)() = c1.getCode<void (*)()>();
	void (*f2)() = c2.getCode<void (*)()>();
	void (*f3)() = c3.getCode<void (*)()>();
	void (*f4)() = c4.getCode<void (*)()>();
	void (*f5)() = c5.getCode<void (*)()>();
	CYBOZU_BENCH("c0", f0);
	CYBOZU_BENCH("c1", f1);
	CYBOZU_BENCH("c2", f2);
	CYBOZU_BENCH("c3", f3);
	CYBOZU_BENCH("c4", f4);
	CYBOZU_BENCH("c5", f5);
}

struct ShiftVsOr : Xbyak::CodeGenerator {
	ShiftVsOr(int b)
	{
		mov(r11, (size_t)str);
		xor_(r10, r10);
		mov(ecx, N);
	L("@@");
		switch (b) {
		case 0:
			movzx(edx, word [r11]);
			movzx(eax, byte [r11 + 2]);
			shl(eax, 16);
			or_(edx, eax);
			xor_(eax, 0x123456);
			or_(r10d, eax);
			break;
		case 1:
			movzx(edx, word [r11]);
			movzx(eax, byte [r11 + 2]);
			xor_(edx, 0x3456);
			xor_(eax, 0x12);
			or_(r10d, edx);
			or_(r10d, eax);
			break;
		}
		dec(ecx);
		jnz("@b");
		ret();
	}
};

void test1()
{
	puts("test1");
	ShiftVsOr c0(0);
	ShiftVsOr c1(1);
	void (*f0)() = c0.getCode<void (*)()>();
	void (*f1)() = c1.getCode<void (*)()>();
	CYBOZU_BENCH("0", f0);
	CYBOZU_BENCH("1", f1);
	CYBOZU_BENCH("0", f0);
	CYBOZU_BENCH("1", f1);
}

int main()
{
	test0();
	test1();
}
