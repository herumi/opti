/*
gcc ; gcc-4.6.3
Xeon x5650
                i3  i7    Xeon
bitblt_jmpC    gcc  VC11  gcc
op=0 db2d2bdd 1.94  1.83 1.75
op=1 374b2e97 3.47  3.65 1.75
op=2 59acebf1 3.45  4.50 2.62
op=3 2f89b2a9 3.48  5.43 2.62
bitblt_noJmpC
op=0 db2d2bdd 0.85  0.48 0.44
op=1 374b2e97 1.10  0.54 0.66
op=2 59acebf1 1.29  1.81 0.79
op=3 2f89b2a9 1.21  1.81 0.79
bitblt_noJmp
op=0 db2d2bdd 2.10  1.10 1.75
op=1 374b2e97 3.44  1.83 2.61
op=2 59acebf1 3.48  1.83 2.61
op=3 2f89b2a9 3.42  1.83 2.61
bitblt_jmp1
op=0 db2d2bdd 5.13  2.75 2.63
op=1 374b2e97 6.71  3.62 3.48
op=2 59acebf1 8.40  4.58 4.35
op=3 2f89b2a9 8.37  4.59 5.22
bitblt_jmp2
op=0 db2d2bdd 3.43  1.82 2.62
op=1 374b2e97 5.11  2.71 3.48
op=2 59acebf1 6.72  3.62 4.35
op=3 2f89b2a9 8.34  4.55 5.23
*/
#include <stdio.h>
#include <stdint.h>
#include <numeric>
#include <vector>
#include "util.hpp"
#include <xbyak/xbyak_util.h>

const int OP_NUM = 4;

#if defined(_WIN32) && !defined(_WIN64)
	#define USE_WIN32_ASM
#endif

typedef std::vector<uint32_t> Vec;

void op0(uint32_t& dst, uint32_t) { dst = 0; }
void op1(uint32_t& dst, uint32_t src) { dst = src; }
void op2(uint32_t& dst, uint32_t src) { dst ^= src; }
void op3(uint32_t& dst, uint32_t src) { dst |= src; }

template<class F>
void bitbltT(F f, uint32_t *dst, const uint32_t *src, int n)
{
	for (int i = 0; i < n; i++) {
		f(dst[i], src[i]);
	}
}

void bitblt_jmpC(uint32_t *dst, const uint32_t *src, int n, int op)
{
	for (int i = 0; i < n; i++) {
		switch (op) {
		case 0: op0(dst[i], src[i]); break;
		case 1: op1(dst[i], src[i]); break;
		case 2: op2(dst[i], src[i]); break;
		case 3: op3(dst[i], src[i]); break;
		}
	}
}

void bitblt_noJmpC(uint32_t *dst, const uint32_t *src, int n, int op)
{
	switch (op) {
	case 0: bitbltT(op0, dst, src, n); break;
	case 1: bitbltT(op1, dst, src, n); break;
	case 2: bitbltT(op2, dst, src, n); break;
	case 3: bitbltT(op3, dst, src, n); break;
	}
}

uint32_t hash(const Vec& v)
{
	uint32_t ret = 0;
	for (size_t i = 0, n = v.size(); i < n; i++) {
		ret ^= v[i] + 0x9e3779b9 + (ret << 6) + (ret >> 2);
	}
	return ret;
}

struct CodeNoJmp : public Xbyak::CodeGenerator {

	// bitblt_sub(uint32_t *dst, const uint32_t *src, int n);
	CodeNoJmp(int op)
	{
		using namespace Xbyak;
#ifdef XBYAK32
		const Reg32& dst = ecx;
		const Reg32& src = edx;
		const Reg32& n = ebx;
		push(ebx);
		const size_t P = 4 * 1;
		mov(dst, ptr [esp + P + 4]);
		mov(src, ptr [esp + P + 8]);
		mov(n, ptr [esp + P + 12]);
#elif defined(XBYAK64_WIN)
		const Reg64& dst = rcx;
		const Reg64& src = rdx;
		const Reg64& n = r8;
#else
		const Reg64& dst = rdi;
		const Reg64& src = rsi;
		const Reg64& n = rdx;
#endif
		inLocalLabel();
		xor(eax, eax);
	L("@@");
		switch (op) {
		case 0:
			mov(ptr [dst], eax);
			break;
		case 1:
			mov(eax, ptr [src]);
			mov(ptr [dst], eax);
			break;
		case 2:
			mov(eax, ptr [src]);
			xor(ptr [dst], eax);
			break;
		case 3:
			mov(eax, ptr [src]);
			or(ptr [dst], eax);
			break;
		}
		add(dst, 4);
		add(src, 4);
		sub(n, 1);
		jnz("@b");
#ifdef XBYAK32
		pop(ebx);
#endif
		ret();
		outLocalLabel();
	}
};

struct CodeWithJmp1 : public Xbyak::CodeGenerator {

	// bitblt_sub(uint32_t *dst, const uint32_t *src, int n, int op);
	CodeWithJmp1()
	{
		using namespace Xbyak;
#ifdef XBYAK32
		const Reg32& dst = edi;
		const Reg32& src = esi;
		const Reg32& n = ecx;
		const Reg32& op = edx;
		const int P = 4 * 2;
		push(esi);
		push(edi);
		mov(dst, ptr [esp + P + 4]);
		mov(src, ptr [esp + P + 8]);
		mov(n, ptr [esp + P + 12]);
		mov(op, ptr [esp + P + 16]);
#elif defined(XBYAK64_WIN)
		const Reg64& dst = rcx;
		const Reg64& src = rdx;
		const Reg64& n = r8;
		const Reg64& op = r9;
#else
		const Reg64& dst = rdi;
		const Reg64& src = rsi;
		const Reg64& n = rdx;
		const Reg64& op = rcx;
#endif
		inLocalLabel();
		xor(eax, eax);

	L("@@");
		test(op, op);
		je(".lp0");
		cmp(op, 1);
		je(".lp1");
		cmp(op, 2);
		je(".lp2");
		jmp(".lp3");

		align(16);
	L(".lp0");
		mov(ptr [dst], eax);
		jmp(".next");

		align(16);
	L(".lp1");
		mov(eax, ptr [src]);
		mov(ptr [dst], eax);
		jmp(".next");

		align(16);
	L(".lp2");
		mov(eax, ptr [src]);
		xor(ptr [dst], eax);
		jmp(".next");

		align(16);
	L(".lp3");
		mov(eax, ptr [src]);
		or(ptr [dst], eax);
	L(".next");
		add(dst, 4);
		add(src, 4);
		sub(n, 1);
		jnz("@b");
#ifdef XBYAK32
		pop(edi);
		pop(esi);
#endif
		ret();
		outLocalLabel();
	}
};

struct CodeJmp2 : public Xbyak::CodeGenerator {

	// bitblt_sub(uint32_t *dst, const uint32_t *src, int n, int op);
	CodeJmp2()
	{
		using namespace Xbyak;
#ifdef XBYAK32
		const Reg32& dst = edi;
		const Reg32& src = esi;
		const Reg32& n = ecx;
		const Reg32& op = edx;
		const int P = 4 * 2;
		push(esi);
		push(edi);
		mov(dst, ptr [esp + P + 4]);
		mov(src, ptr [esp + P + 8]);
		mov(n, ptr [esp + P + 12]);
		mov(op, ptr [esp + P + 16]);
#elif defined(XBYAK64_WIN)
		const Reg64& dst = rcx;
		const Reg64& src = rdx;
		const Reg64& n = r8;
		const Reg64& op = r9;
#else
		const Reg64& dst = rdi;
		const Reg64& src = rsi;
		const Reg64& n = rdx;
		const Reg64& op = rcx;
#endif
		inLocalLabel();
		xor(eax, eax);

	L("@@");
		test(op, op);
		je(".lp0");
		cmp(op, 1);
		je(".lp1");
		cmp(op, 2);
		je(".lp2");
		jmp(".lp3");

		align(16);
	L(".lp0");
		mov(ptr [dst], eax);
		add(dst, 4);
		add(src, 4);
		sub(n, 1);
		jnz("@b");
		jmp(".exit");

		align(16);
	L(".lp1");
		mov(eax, ptr [src]);
		mov(ptr [dst], eax);
		add(dst, 4);
		add(src, 4);
		sub(n, 1);
		jnz("@b");
		jmp(".exit");

		align(16);
	L(".lp2");
		mov(eax, ptr [src]);
		xor(ptr [dst], eax);
		add(dst, 4);
		add(src, 4);
		sub(n, 1);
		jnz("@b");
		jmp(".exit");

		align(16);
	L(".lp3");
		mov(eax, ptr [src]);
		or(ptr [dst], eax);
		add(dst, 4);
		add(src, 4);
		sub(n, 1);
		jnz("@b");

	L(".exit");
#ifdef XBYAK32
		pop(edi);
		pop(esi);
#endif
		ret();
		outLocalLabel();
	}
};

void bitblt_noJmp(uint32_t *dst, const uint32_t *src, int n, int op)
{
	static const CodeNoJmp cTbl[OP_NUM] = { 0, 1, 2, 3 };
	((void (*)(uint32_t*, const uint32_t*, int))cTbl[op].getCode())(dst, src, n);
}

void bitblt_jmp1(uint32_t *dst, const uint32_t *src, int n, int op)
{
	static const CodeWithJmp1 c;
	((void (*)(uint32_t*, const uint32_t*, int, int))c.getCode())(dst, src, n, op);
}

void bitblt_jmp2(uint32_t *dst, const uint32_t *src, int n, int op)
{
	static const CodeJmp2 c;
	((void (*)(uint32_t*, const uint32_t*, int, int))c.getCode())(dst, src, n, op);
}

template<class F>
uint32_t bench(F f, Vec& dst, const Vec& src, int op, const uint32_t *expect = 0)
{
	const int N = 5000;
	Xbyak::util::Clock clk;
	for (int i = 0; i < N; i++) {
		clk.begin();
		f(&dst[0], &src[0], (int)dst.size(), op);
		clk.end();
	}
	uint32_t ret = hash(dst);
	printf("op=%d %08x %4.2f", op, ret, clk.getClock() / double(N) / src.size());
	if (expect) {
		printf(" %c", *expect == ret ? 'o' : 'x');
	}
	printf("\n");
	return ret;
}

template<class RG>
void init(Vec& v, RG& rg, size_t n)
{
	v.resize(n);
	for (size_t i = 0; i < n; i++) {
		v[i] = rg.get();
	}
}

#ifdef USE_WIN32_ASM
__declspec(naked) void bitblt_vc(uint32_t* /*dst*/, const uint32_t* /*src*/, int /*n*/, int /*op*/)
{
	enum {
		P = 8
	};
	__asm {
		push	esi
		push	edi

		mov		edi, [esp + P + 4] // dst
		mov		esi, [esp + P + 8] // src
		mov		ecx, [esp + P + 12] // n
		mov		edx, [esp + P + 16] // op

		xor		eax, eax
	lp:
		test	edx, edx
		je		lp_0
		cmp		edx, 1
		je		lp_1
		cmp		edx, 2
		je		lp_2
		jmp		lp_3

		align	16
	lp_0:
		mov		[edi], eax
		jmp		next
		align	16
	lp_1:
		mov		eax, [esi]
		mov		[edi], eax
		jmp		next
		align	16
	lp_2:
		mov		eax, [esi]
		xor		[edi], eax
		jmp		next
		align	16
	lp_3:
		mov		eax, [esi]
		or		[edi], eax
		align	16
	next:
		add		edi, 4
		add		esi, 4
		sub		ecx, 1
		jnz		lp

		pop		edi
		pop		esi
		ret
	};
}
#endif

int main()
{
	XorShift128 rg;
	Vec dst, org, src;
	const size_t size = 10000;
	puts("init");
	init(org, rg, size);
	init(src, rg, size);

	uint32_t expect[OP_NUM];
	puts("bitblt_jmpC");
	for (int op = 0; op < OP_NUM; op++) {
		dst = org;
		expect[op] = bench(bitblt_jmpC, dst, src, op);
	}
	puts("bitblt_noJmpC");
	for (int op = 0; op < OP_NUM; op++) {
		dst = org;
		bench(bitblt_noJmpC, dst, src, op, &expect[op]);
	}
	puts("bitblt_noJmp");
	for (int op = 0; op < OP_NUM; op++) {
		dst = org;
		bench(bitblt_noJmp, dst, src, op, &expect[op]);
	}
	puts("bitblt_jmp1");
	for (int op = 0; op < OP_NUM; op++) {
		dst = org;
		bench(bitblt_jmp1, dst, src, op, &expect[op]);
	}
	puts("bitblt_jmp2");
	for (int op = 0; op < OP_NUM; op++) {
		dst = org;
		bench(bitblt_jmp2, dst, src, op, &expect[op]);
	}
#ifdef USE_WIN32_ASM
	puts("bitblt_vc");
	for (int op = 0; op < OP_NUM; op++) {
		dst = org;
		bench(bitblt_vc, dst, src, op, &expect[op]);
	}
#endif
}
