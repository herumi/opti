/*
core i7-2600k
0 clk 101649.80 10.16/N ret=6336da62151d21c7
1 clk 173381.37 17.34/N ret=6336da62151d21c7
2 clk 144382.52 14.44/N ret=6336da62151d21c7
3 clk 156597.97 15.66/N ret=6336da62151d21c7

Xeon X5650
0 clk 128765.77 12.88/N ret=6336da62151d21c7
1 clk 199949.04 19.99/N ret=6336da62151d21c7
2 clk 183399.38 18.34/N ret=6336da62151d21c7
3 clk 192379.77 19.24/N ret=6336da62151d21c7
*/
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>
#include <cybozu/inttype.hpp>

const int N = 10000;

static uint64_t g_p[4] = {
	uint64_t(0x2523648240000001ULL),
	uint64_t(0xba344d8000000008ULL),
	uint64_t(0x6121000000000013ULL),
	uint64_t(0xa700000000000013ULL)
};

static uint64_t g_x[4], g_y[4];

using namespace Xbyak;

struct Sub4Code : Xbyak::CodeGenerator {
	void load_rm(const Reg64& z3, const Reg64& z2, const Reg64& z1, const Reg64& z0,
		const Reg32e& m)
	{
		mov(z0, ptr [m + 8 * 0]);
		mov(z1, ptr [m + 8 * 1]);
		mov(z2, ptr [m + 8 * 2]);
		mov(z3, ptr [m + 8 * 3]);
	}
	/*
		[z3:z2:z1:z0] = [m3:m2:m1:m0]
	*/
	void store_mr(const Reg32e& m, const Reg64& x3, const Reg64& x2, const Reg64& x1, const Reg64& x0)
	{
		mov(ptr [m + 8 * 0], x0);
		mov(ptr [m + 8 * 1], x1);
		mov(ptr [m + 8 * 2], x2);
		mov(ptr [m + 8 * 3], x3);
	}
	void add_rr(const Reg64& z3, const Reg64& z2, const Reg64& z1, const Reg64& z0,
		const Reg64& x3, const Reg64& x2, const Reg64& x1, const Reg64& x0)
	{
		add(z0, x0);
		adc(z1, x1);
		adc(z2, x2);
		adc(z3, x3);
	}
	void add_rm(const Reg64& z3, const Reg64& z2, const Reg64& z1, const Reg64& z0,
		const Reg32e& m)
	{
		add(z0, ptr [m + 8 * 0]);
		adc(z1, ptr [m + 8 * 1]);
		adc(z2, ptr [m + 8 * 2]);
		adc(z3, ptr [m + 8 * 3]);
	}
	void sub_rm(const Reg64& z3, const Reg64& z2, const Reg64& z1, const Reg64& z0,
		const Reg32e& m)
	{
		sub(z0, ptr [m + 8 * 0]);
		sbb(z1, ptr [m + 8 * 1]);
		sbb(z2, ptr [m + 8 * 2]);
		sbb(z3, ptr [m + 8 * 3]);
	}

	Sub4Code(int m)
	{
		push(rbx);

		push(rsi);
		push(rdi);
		push(r11);
		push(r12);
		push(r13);
		push(r14);

		mov(r8, N);
		mov(r9, (size_t)g_x);
		mov(r10, (size_t)g_y);
		mov(r14, (size_t)g_p);

	L(".lp");
		load_rm(rax, rbx, rcx, rdx, r9);
		sub_rm(rax, rbx, rcx, rdx, r10);

		switch (m) {
		case 0:
			jnc(".skip");
			add_rm(rax, rbx, rcx, rdx, r14);
		L(".skip");
			store_mr(r9, rax, rbx, rcx, rdx);
			break;
		case 1:
			sbb(rsi, rsi);
			mov(rdi, rsi);
			mov(r11, rsi);
			mov(r12, rsi);
			and_(rsi, ptr [r14 + 8 * 0]);
			and_(rdi, ptr [r14 + 8 * 1]);
			and_(r11, ptr [r14 + 8 * 2]);
			and_(r12, ptr [r14 + 8 * 3]);
			add_rr(rax, rbx, rcx, rdx, r12, r11, rdi, rsi);
			store_mr(r9, rax, rbx, rcx, rdx);
			break;
		case 2:
			load_rm(r12, r11, rdi, rsi, r14);
			sbb(r13, r13);
			and_(rsi, r13);
			and_(rdi, r13);
			and_(r11, r13);
			and_(r12, r13);
			add_rr(rax, rbx, rcx, rdx, r12, r11, rdi, rsi);
			store_mr(r9, rax, rbx, rcx, rdx);
			break;
		case 3:
			mov(rsi, 0);
			mov(rdi, rsi);
			mov(r11, rsi);
			mov(r12, rsi);
			cmovc(rsi, ptr [r14 + 8 * 0]);
			cmovc(rdi, ptr [r14 + 8 * 1]);
			cmovc(r11, ptr [r14 + 8 * 2]);
			cmovc(r12, ptr [r14 + 8 * 3]);
			add_rr(rax, rbx, rcx, rdx, r12, r11, rdi, rsi);
			store_mr(r9, rax, rbx, rcx, rdx);
			break;
		}
		dec(r8);
		jnz(".lp");

		pop(r14);
		pop(r13);
		pop(r12);
		pop(r11);
		pop(rdi);
		pop(rsi);

		pop(rbx);
		ret();
	}
};

void test1(void f())
{
	for (int i = 0; i < 4; i++) {
		g_x[i] = ~i * i + 0x12345678;
	}
	g_y[0] = uint64_t(0x2523648240000001ULL);
	g_y[1] = uint64_t(0xba344d8000000008ULL);
	g_y[2] = uint64_t(0x6121000000000013ULL);
	g_y[3] = uint64_t(0xb700000000000013ULL);

	const int C = 10000;
	Xbyak::util::Clock clk;
	clk.begin();
	for (int i = 0; i < C; i++) {
		f();
	}
	clk.end();
	long long ret = 0;
	for (int i = 0; i < 4; i++) {
		ret += g_x[i];
	}
	double c = clk.getClock() / double(C);
	printf("clk %9.2f %5.2f/N ", c, c / N);
	printf("ret=%llx\n", ret);
}

template<class Code, class F>
void test(const char *msg, void test(F))
{
	puts(msg);
	static const Code tbl[] = {
		0, 1, 2, 3,
	};
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
		printf("%d ", (int)i);
		test(tbl[i].template getCode<F>());
	}
}

int main()
{
	test<Sub4Code>("sub4", test1);
}
