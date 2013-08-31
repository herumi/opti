#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>
#include <cybozu/array.hpp>
#include <cybozu/benchmark.hpp>
#include <cybozu/inttype.hpp>
#include <stdio.h>

#ifdef XBYAK32
	#error "64bit only"
#endif

typedef cybozu::AlignedArray<char, 32> Vec;

void (*copy_movsb)(void *dst, const void *src, size_t n);
void (*copy_xmm)(void *dst, const void *src, size_t n);

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
		rep_movsb();
		pop(rdi);
		pop(rsi);
		ret();
#else
		// (rdi, rsi, rdx)
		mov(rcx, rdx);
		rep_movsb();
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
		rep_movsb();
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
	void rep_movsb()
	{
		db(0xf3); db(0xa4);
	}
} s_code;


int main()
{
	Vec vx, vy;
	const int N = 1024 * 1024 * 16;

	vx.resize(N);
	vy.resize(N);
	char *x = &vx[0];
	char *y = &vy[0];

	{
		char buf[] = "hello this";
		char buf2[64] = "";
		copy_movsb(buf2, buf, sizeof(buf));
		printf("buf2=%s\n", buf2);
	}

	const int tbl[] = {
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
		CYBOZU_BENCH("movsb", copy_movsb, x, y, n);
		CYBOZU_BENCH("xmm  ", copy_xmm, x, y, n);
	}
}


