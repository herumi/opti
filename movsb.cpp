#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>
#include <cybozu/array.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/inttype.hpp>
#include <stdio.h>
#include <memory.h>

#ifdef XBYAK32
	#error "64bit only"
#endif

typedef cybozu::AlignedArray<char, 32> Vec;

void (*copy_movsb)(void *dst, const void *src, size_t n);
void (*copy_xmm)(void *dst, const void *src, size_t n);

void memcpyC(void *dst, const void *src, size_t n)
{
	char *q = (char*)dst;
	const char *p = (const char*)src;
	for (size_t i = 0; i < n; i++) {
		q[i] = p[i];
	}
}

#ifdef __GNUC__
	#define USE_INLINE_ASM
#endif

#ifdef USE_INLINE_ASM
void asm_movsb(void *dst, const void *src, size_t n)
{
	__asm__ volatile(".byte 0xf3, 0xa4" :: "S"(src), "D"(dst), "c"(n));
}
#endif

struct Code : Xbyak::CodeGenerator {
	Code()
	{
		copy_movsb = getCurr<void (*)(void*, const void*, size_t)>();
		gen_copy_movsb();

		align(16);
		copy_xmm = getCurr<void (*)(void*, const void*, size_t)>();
		gen_copy_xmm();
	}
	void gen_copy_movsb()
	{
		// use rcx, rsi, rdi
#if 1
#ifdef XBYAK64_WIN
		// (rcx, rdx, r8)
		push(rsi);
		push(rdi);
		mov(rdi, rcx);
		mov(rsi, rdx);
		mov(rcx, r8);
		rep(); movsb();
		pop(rdi);
		pop(rsi);
		ret();
#else
		// (rdi, rsi, rdx)
		mov(rcx, rdx);
		rep(); movsb();
		ret();
#endif
#else
		Xbyak::util::StackFrame sf(this, 3, Xbyak::util::UseRCX);
		const Xbyak::Reg64& dst = sf.p[0];
		const Xbyak::Reg64& src = sf.p[1];
		const Xbyak::Reg64& n = sf.p[2];
		push(rsi);
		push(rdi);
		mov(rdi, dst);
		mov(rsi, src);
		mov(rcx, n);
		rep(); movsb();
		pop(rdi);
		pop(rsi);
#endif
	}
	void gen_copy_xmm()
	{
		Xbyak::util::StackFrame sf(this, 3);
		const Xbyak::Reg64& dst = sf.p[0];
		const Xbyak::Reg64& src = sf.p[1];
		const Xbyak::Reg64& n = sf.p[2];
	L("@@");
		vmovaps(ym0, ptr [src]);
		vmovaps(ym1, ptr [src + 32]);
		vmovaps(ptr [dst], ym0);
		vmovaps(ptr [dst + 32], ym1);
		add(src, 64);
		add(dst, 64);
		sub(n, 64);
		jnz("@b");
	}
} s_code;

void putResult(const char *msg, int n)
{
	double clk = cybozu::bench::g_clk.getClock();
	double cnt = cybozu::bench::g_clk.getCount() * cybozu::bench::g_loopNum;
	printf("%s %.2f byte/clk\n", msg, n * cnt / clk);
}

int main()
{
	Vec vx, vy;
	const int N = 1024 * 1024 * 16;

	vx.resize(N);
	vy.resize(N);
	char *x = &vx[0];
	char *y = &vy[0];

	Xbyak::util::Cpu cpu;
	printf("enhanced rep(ermsb) %s\n", cpu.has(Xbyak::util::Cpu::tENHANCED_REP) ? "on" : "off");
	{
		char buf[] = "hello this";
		char buf2[64] = "";
		copy_movsb(buf2, buf, sizeof(buf));
		printf("buf2=%s\n", buf2);
	}
	const int tbl[] = {
		64,
		128,
		256,
		512,
		1024,
		1024 * 2,
		1024 * 4,
		1024 * 16,
		1024 * 256,
		1024 * 1024,
		1024 * 1024 * 2,
		1024 * 1024 * 4,
		1024 * 1024 * 16,
	};
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
		const int n = tbl[i];
		printf("size %d byte\n", n);
		CYBOZU_BENCH("", copy_movsb, x, y, n); putResult("movsb", n);
		CYBOZU_BENCH("", copy_xmm, x, y, n); putResult("xmm  ", n);
		CYBOZU_BENCH("", memcpyC, x, y, n); putResult("memcpyC  ", n);
		CYBOZU_BENCH("", memcpy, x, y, n); putResult("memcpy  ", n);
#ifdef USE_INLINE_ASM
		CYBOZU_BENCH("", asm_movsb, x, y, n); putResult("asm_movsb  ", n);
#endif
	}
	const int tbl2[] = {
		1024 * 2,
		1024 * 1024 * 4,
	};
	puts("non align");
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl2); i++) {
		const int n = tbl2[i];
		for (int j = 0; j < 4; j++) {
			printf("n=%d ", n - j);
			CYBOZU_BENCH("", copy_movsb, x + j, y, n - j); putResult("", n - j);
		}
	}
}


