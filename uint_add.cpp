/*
	WinXp(32bit) Core 2 Duo
0 clk 230.77 15.38/u ret=e47da450
1 clk 219.29 14.62/u ret=e47da450
2 clk 220.64 14.71/u ret=e47da450
3 clk  80.02  5.33/u ret=e47da450

	Linux(32bit) Xeon X5650
0 clk 200.16 13.34/u ret=e47da450
1 clk 200.11 13.34/u ret=e47da450
2 clk 200.30 13.35/u ret=e47da450
3 clk  75.71  5.05/u ret=e47da450

	Linux(64bit) Xeon X5650
0 clk 189.01 12.60/u ret=20be08be7406450
1 clk 190.55 12.70/u ret=20be08be7406450
2 clk 190.52 12.70/u ret=20be08be7406450
3 clk  67.17  4.48/u ret=20be08be7406450

	Win7(32bit) i7-2600
0 clk  34.59  2.31/u ret=e47da450
1 clk  48.56  3.24/u ret=e47da450
2 clk  45.18  3.01/u ret=e47da450
3 clk  45.77  3.05/u ret=e47da450

	Win7(64bit) i7-2600
0 clk  30.72  2.05/u ret=20be08be7406450
1 clk  44.10  2.94/u ret=20be08be7406450
2 clk  53.45  3.56/u ret=20be08be7406450
3 clk  53.47  3.56/u ret=20be08be7406450

	Linux(32bit) i7-3930K
0 clk  31.45  2.10/u ret=e47da450
1 clk  47.64  3.18/u ret=e47da450
2 clk  37.50  2.50/u ret=e47da450
3 clk  37.91  2.53/u ret=e47da450

	Linux(64bit) i7-3930K
0 clk  28.70  1.91/u ret=20be08be7406450
1 clk  41.36  2.76/u ret=20be08be7406450
2 clk  48.36  3.22/u ret=20be08be7406450
3 clk  47.28  3.15/u ret=20be08be7406450
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
		xor_(a, a);
		neg(c);
		mov(t, ptr [x + c * S]);
		add(t, ptr [y]);
		mov(ptr [out + c * S], t);
		inc(c);
		switch (mode) {
		case 0:
			jz(".exit");
		L(".lp");
			mov(t, ptr [x + c * S]);
			adc(t, ptr [y + c * S]);
			mov(ptr [out + c * S], t);
			inc(c);
			jnz(".lp");
			break;
		case 1:
			jecxz(".exit");
		L(".lp");
			mov(t, ptr [x + c * S]);
			adc(t, ptr [y + c * S]);
			mov(ptr [out + c * S], t);
			inc(c);
			jecxz(".exit");
			jmp(".lp");
			break;
		case 2:
		L(".lp");
			jecxz(".exit");
			mov(t, ptr [x + c * S]);
			adc(t, ptr [y + c * S]);
			mov(ptr [out + c * S], t);
			inc(c);
			jmp(".lp");
			break;
		case 3:
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
	const int outN = 15;
	size_t out[outN];
	for (int i = 0; i < outN; i++) {
		out[i] = 0x12345678;
	}
	const int N = 3000000;
	Xbyak::util::Clock clk;
	clk.begin();
	for (int i = 0; i < N; i++) {
		add1(out, out, outN, i);
	}
	clk.end();
	size_t ret = 0;
	for (int i = 0; i < outN; i++) {
		ret = (ret ^ out[i]) << 1;
	}
	double c = clk.getClock() / double(N);
	printf("clk %6.2f %5.2f/u ", c, c / outN);
	printf("ret=%llx\n", (long long)ret);
}

void test1(bool addn(size_t *, const size_t *, const size_t *, size_t))
{
	const int xN = 15;
	size_t x[xN];
	for (int i = 0; i < xN; i++) {
		x[i] = 0x12345678 + i;
	}
	const int N = 3000000;
	Xbyak::util::Clock clk;
	clk.begin();
	for (int i = 0; i < N; i++) {
		addn(x, x, x, xN);
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

int main()
	try
{
	test<Code_add1>("add1", test0);
	test<Code_addn>("addn", test1);
} catch (Xbyak::Error e) {
	printf("err=%s\n", Xbyak::ConvertErrorToString(e));
}
