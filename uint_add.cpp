/*
	WinXp(64bit) Core 2 Duo 1.8GHz
add1
0 clk 205.59 13.71/u ret=20be08be7406450
1 clk 194.69 12.98/u ret=20be08be7406450
2 clk 200.21 13.35/u ret=20be08be7406450
3 clk  59.16  3.94/u ret=20be08be7406450
addn
0 clk 215.57 13.47/u ret=c7e3fd489320ca
1 clk 229.60 14.35/u ret=c7e3fd489320ca
2 clk 221.30 13.83/u ret=c7e3fd489320ca
3 clk  55.88  3.49/u ret=c7e3fd489320ca
i=0
1788793664 2.989500
i=1
1788793664 3.170413
i=2
1788793664 2.920048

	Linux(32bit) Xeon X5650
add1
0 clk 200.36 13.36/u ret=e47da450
1 clk 200.12 13.34/u ret=e47da450
2 clk 200.29 13.35/u ret=e47da450
3 clk  75.69  5.05/u ret=e47da450
addn
0 clk 224.47 14.03/u ret=0
1 clk 213.32 13.33/u ret=0
2 clk 214.01 13.38/u ret=0
3 clk  80.05  5.00/u ret=0

	Linux(64bit) Xeon X5650
add1
0 clk 189.31 12.62/u ret=20be08be7406450
1 clk 190.65 12.71/u ret=20be08be7406450
2 clk 190.91 12.73/u ret=20be08be7406450
3 clk  67.01  4.47/u ret=20be08be7406450
addn
0 clk 208.62 13.04/u ret=c7e3fd489320ca
1 clk 202.34 12.65/u ret=c7e3fd489320ca
2 clk 202.77 12.67/u ret=c7e3fd489320ca
3 clk  61.98  3.87/u ret=c7e3fd489320ca
4 clk  62.92  3.93/u ret=c7e3fd489320ca
i=0
1788793664 2.612118
i=1
1788793664 2.610572
i=2
1788793664 2.610871

	Linux(64bit) i7-2600
add1
0 clk  32.10  2.14/u ret=20be08be7406450
1 clk  44.66  2.98/u ret=20be08be7406450
2 clk  49.52  3.30/u ret=20be08be7406450
3 clk  50.59  3.37/u ret=20be08be7406450
addn
0 clk  48.02  3.00/u ret=c7e3fd489320ca
1 clk  49.10  3.07/u ret=c7e3fd489320ca
2 clk  33.67  2.10/u ret=c7e3fd489320ca
3 clk  52.14  3.26/u ret=c7e3fd489320ca
4 clk  64.96  4.06/u ret=c7e3fd489320ca
i=0
1788793664 2.435156
i=1
1788793664 2.423111
i=2
1788793664 2.143504

	Win7(64bit) i7-2600
add1
0 clk  31.62  2.11/u ret=20be08be7406450
1 clk  44.22  2.95/u ret=20be08be7406450
2 clk  52.93  3.53/u ret=20be08be7406450
3 clk  51.61  3.44/u ret=20be08be7406450
addn
0 clk  47.56  2.97/u ret=c7e3fd489320ca
1 clk  50.93  3.18/u ret=c7e3fd489320ca
2 clk  36.16  2.26/u ret=c7e3fd489320ca
3 clk  51.24  3.20/u ret=c7e3fd489320ca
i=0
1788793664 2.407932
i=1
1788793664 2.369505
i=2
1788793664 2.137700

*/
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>
#include <cybozu/inttype.hpp>

struct Code_add1 : public Xbyak::CodeGenerator {
	// bool add1(uint32_t *out, const uint32_t *x, size_t n, uint32_t y)
	Code_add1(int mode)
	{
		using namespace Xbyak;
		inLocalLabel();
		const int S = (int)sizeof(size_t);
#ifdef XBYAK32
		const Reg32& a = eax;
		const Reg32& c = ecx;
		const Reg32& out = edi;
		const Reg32& x = esi;
		const Reg32& y = edx;
		const Reg32& t = ebx;
		const int P = 4 * 3;
		push(ebx);
		push(esi);
		push(edi);
		mov(out, ptr [esp + P + 4]);
		mov(x, ptr [esp + P + 8]);
		mov(c, ptr [esp + P + 12]);
		mov(y, ptr [esp + P + 16]);
#else
		const Reg64& a = rax;
		const Reg64& c = rcx;
#ifdef XBYAK64_WIN
		const Reg64& out = r8;
		const Reg64& x = rdx;
		const Reg64& y = r9;
		const Reg64& t = r11;
		xchg(r8, rcx);
#else
		const Reg64& out = rdi;
		const Reg64& x = rsi;
		const Reg64& y = rdx;
		const Reg64& t = r8;
		xchg(rcx, rdx);
#endif
#endif
		lea(out, ptr [out + c * S]);
		lea(x, ptr [x + c * S]);
		xor_(a, a);
		neg(c);
		mov(t, ptr [x + c * S]);
		add(t, y);
		mov(ptr [out + c * S], t);
		inc(c);
		switch (mode) {
		case 0:
			jz(".exit");
		L(".lp");
			mov(t, ptr [x + c * S]);
			adc(t, a);
			mov(ptr [out + c * S], t);
			inc(c);
			jnz(".lp");
			break;
		case 1:
			jecxz(".exit");
		L(".lp");
			mov(t, ptr [x + c * S]);
			adc(t, a);
			mov(ptr [out + c * S], t);
			inc(c);
			jecxz(".exit");
			jmp(".lp");
			break;
		case 2:
		L(".lp");
			jecxz(".exit");
			mov(t, ptr [x + c * S]);
			adc(t, a);
			mov(ptr [out + c * S], t);
			inc(c);
			jmp(".lp");
			break;
		case 3:
		L(".lp");
			jecxz(".exit");
			mov(t, ptr [x + c * S]);
			adc(t, a);
			mov(ptr [out + c * S], t);
			lea(c, ptr [c + 1]);
			jmp(".lp");
			break;
		}
	L(".exit");
		setc(al);
#ifdef XBYAK32
		pop(edi);
		pop(esi);
		pop(ebx);
#endif
		ret();
		outLocalLabel();
	}
};

struct Code_addn : public Xbyak::CodeGenerator {
	// bool addn(size_t *out, const size_t *x, const size_t *y, size_t n)
	Code_addn(int mode)
	{
		using namespace Xbyak;
		inLocalLabel();
		const int S = (int)sizeof(size_t);
#ifdef XBYAK32
		const Reg32& a = eax;
		const Reg32& c = ecx;
		const Reg32& out = edi;
		const Reg32& x = esi;
		const Reg32& y = edx;
		const Reg32& t = ebx;
		const int P = 4 * 3;
		push(ebx);
		push(esi);
		push(edi);
		mov(out, ptr [esp + P + 4]);
		mov(x, ptr [esp + P + 8]);
		mov(y, ptr [esp + P + 12]);
		mov(c, ptr [esp + P + 16]);
#else
		const Reg64& a = rax;
		const Reg64& c = rcx;
#ifdef XBYAK64_WIN
		// out = rcx, x = rdx, y = r8, n = r9
		const Reg64& out = r9;
		const Reg64& x = rdx;
		const Reg64& y = r8;
		const Reg64& t = r10;
		xchg(r9, rcx);
#else
		// out = rdi, x = rsi, y = rdx, n = rcx
		const Reg64& out = rdi;
		const Reg64& x = rsi;
		const Reg64& y = rdx;
		const Reg64& t = r8;
#endif
#endif
		lea(out, ptr [out + c * S]);
		lea(x, ptr [x + c * S]);
		lea(y, ptr [y + c * S]);
		neg(c);
		switch (mode) {
		case 0:
			jz(".exit");
			xor_(a, a);
		L(".lp");
			mov(t, ptr [x + c * S]);
			adc(t, ptr [y + c * S]);
			mov(ptr [out + c * S], t);
			inc(c);
			jnz(".lp");
			break;
		case 1:
			jz(".exit");
			xor_(a, a);
		L(".lp");
			mov(t, ptr [x + c * S]);
			adc(t, ptr [y + c * S]);
			mov(ptr [out + c * S], t);
			inc(c);
			jecxz(".exit");
			jmp(".lp");
			break;
		case 2:
#if 1
			mov(t, ptr [x + c * S]);
			add(t, ptr [y + c * S]);
			mov(ptr [out + c * S], t);
			inc(c);
			jz(".exit");
		L(".lp");
			mov(t, ptr [x + c * S]);
			adc(t, ptr [y + c * S]);
			mov(ptr [out + c * S], t);
			inc(c);
			jnz(".lp");
#else
			jz(".exit");
			xor_(a, a);
		L(".lp");
			jecxz(".exit");
			mov(t, ptr [x + c * S]);
			adc(t, ptr [y + c * S]);
			mov(ptr [out + c * S], t);
			inc(c);
			jmp(".lp");
#endif
			break;
		case 3:
			xor_(a, a);
		L(".lp");
			jecxz(".exit");
			mov(t, ptr [x + c * S]);
			adc(t, ptr [y + c * S]);
			mov(ptr [out + c * S], t);
			lea(c, ptr [c + 1]);
			jmp(".lp");
			break;
		}
	L(".exit");
		setc(al);
#ifdef XBYAK32
		pop(edi);
		pop(esi);
		pop(ebx);
#endif
		ret();
		outLocalLabel();
	}
};

void test0(bool add1(size_t *, const size_t *, size_t, size_t))
{
	const int xN = 15;
	size_t x[xN];
	for (int i = 0; i < xN; i++) {
		x[i] = 0x12345678;
	}
	const int N = 3000000;
	Xbyak::util::Clock clk;
	clk.begin();
	for (int i = 0; i < N; i++) {
		add1(x, x, xN, i);
	}
	clk.end();
	size_t ret = 0;
	for (int i = 0; i < xN; i++) {
		ret = (ret ^ x[i]) << 1;
	}
	double c = clk.getClock() / double(N);
	printf("clk %6.2f %5.2f/u ", c, c / xN);
	printf("ret=%llx\n", (long long)ret);
}

void test1(bool addn(size_t *, const size_t *, const size_t *, size_t))
{
	const int xN = 16;
	size_t x[xN], y[xN];
	for (int i = 0; i < xN; i++) {
		x[i] = 0x12345678 + i;
		y[i] = x[i] * x[i] + i;
	}
	const int N = 3000000;
	Xbyak::util::Clock clk;
	clk.begin();
	for (int i = 0; i < N; i++) {
		addn(x, x, y, xN);
	}
	clk.end();
	size_t ret = 0;
	for (int i = 0; i < xN; i++) {
		ret = (ret ^ x[i]) << 1;
	}
	double c = clk.getClock() / double(N);
	printf("clk %6.2f %5.2f/u ", c, c / xN);
	printf("ret=%llx\n", (long long)ret);
}

template<class Code, class F>
void test(const char *msg, void test(F))
{
	puts(msg);
	static const Code tbl[] = {
		0, 1, 2, 3
	};
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
		printf("%d ", (int)i);
		test(tbl[i].template getCode<F>());
	}
}

struct Code_abs : Xbyak::CodeGenerator {
	// int loop_abs()
	Code_abs(int N, int mode)
	{
		mov(ecx, N);
		push(ebx);
		xor_(ebx, ebx);
	L("@@");
		switch (mode) {
		case 0:
			// gcc 4.7
			mov(edx, ecx);
			sar(edx, 31);
			mov(eax, edx);
			xor_(eax, ecx);
			sub(eax, edx);
			break;
		case 1:
			// clang 3.3
			mov(eax, ecx);
			mov(edx, ecx);
			neg(eax);
			cmovl(eax, edx);
			break;
		case 2:
			// VC 2008
			mov(eax, ecx);
			cdq();
			xor_(eax, edx);
			sub(eax, edx);
			break;
		}
		add(ebx, eax);
		dec(ecx);
		jnz("@b");
		mov(eax, ebx);
		pop(ebx);
		ret();
	}
};

/*
Xeon X5650
i=0
1788793664 2.613038
i=1
1788793664 2.612897
i=2
1788793664 2.610362

i7-2600K
i=0
1788793664 2.455864
i=1
1788793664 2.321397
i=2
1788793664 2.070405

Core i3-2120T
i=0
1788793664 4.375069
i=1
1788793664 4.218742
i=2
1788793664 3.711571
*/
void test_abs()
{
	const int N = 100000;
	for (int i = 0; i < 3; i++) {
		printf("i=%d\n", i);
		Code_abs code(N, i);
		int (*abs_loop)() = code.getCode<int (*)()>();
		const int C = 100;
		Xbyak::util::Clock clk;
		int ret = 0;
		clk.begin();
		for (int j = 0; j < C; j++) {
			ret += abs_loop();
		}
		clk.end();
		printf("%d %f\n", ret, clk.getClock() / double(N) / C);
	}
}

extern "C" bool addnLLVM(size_t *, const size_t *, const size_t *, size_t);
int main()
	try
{
	test<Code_add1>("add1", test0);
	test<Code_addn>("addn", test1);
#ifdef COMPARE_LLVM
	printf("4 ");
	test1(addnLLVM);
#endif
	test_abs();
} catch (Xbyak::Error e) {
	printf("err=%s\n", Xbyak::ConvertErrorToString(e));
}
