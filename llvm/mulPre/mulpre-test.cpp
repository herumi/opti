#include <stdint.h>
#include <cybozu/benchmark.hpp>
#include <gmpxx.h>
#include <cybozu/xorshift.hpp>
#include <cybozu/test.hpp>
#include <mcl/bint.hpp>
#define XBYAK_ONLY_CLASS_CPU
#include "xbyak/xbyak_util.h"
namespace mcl { namespace fp {
template <typename T>
void swap_(T& x, T& y)
{
	T t = x;
	x = y;
	y = t;
}

} }
#include "src/bint_impl.hpp"

using namespace mcl::bint;
typedef uint64_t Unit;

extern "C" {

void mcl_fpDbl_mulPre3L(Unit *z, const Unit *x, const Unit *y);
void mcl_fpDbl_mulPre4L(Unit *z, const Unit *x, const Unit *y);
void mcl_fpDbl_mulPre5L(Unit *z, const Unit *x, const Unit *y);
void mcl_fpDbl_mulPre6L(Unit *z, const Unit *x, const Unit *y);
void mcl_fpDbl_mulPre7L(Unit *z, const Unit *x, const Unit *y);
void mcl_fpDbl_mulPre8L(Unit *z, const Unit *x, const Unit *y);

}

void_ppp get_llvm_mulPre(size_t n)
{
	switch (n) {
	default: return 0;
	case 3: return mcl_fpDbl_mulPre3L;
	case 4: return mcl_fpDbl_mulPre4L;
	case 5: return mcl_fpDbl_mulPre5L;
	case 6: return mcl_fpDbl_mulPre6L;
	case 7: return mcl_fpDbl_mulPre7L;
	case 8: return mcl_fpDbl_mulPre8L;
	}
}

template<class RG>
void setRand(Unit *x, size_t n, RG& rg)
{
	for (size_t i = 0; i < n; i++) {
		x[i] = (Unit)rg.get64();
	}
}

void setArray(mpz_class& z, const Unit *buf, size_t n)
{
	mpz_import(z.get_mpz_t(), n, -1, sizeof(*buf), 0, 0, buf);
}

template<size_t N>
void testMul()
{
	cybozu::XorShift rg;
	Unit x[N], y[N], z[N * 2];
	mpz_class mx, my, mz;
	const size_t C = 100;
	for (size_t i = 0; i < C; i++) {
		setRand(x, N, rg);
		setRand(y, N, rg);
		mulT<N>(z, x, y);
		setArray(mx, x, N);
		setArray(my, y, N);
		setArray(mz, z, N * 2);
		CYBOZU_TEST_EQUAL(mx * my, mz);
	}
#ifdef NDEBUG
	const int CC = 100000;
	printf("%zd ", N);
	CYBOZU_BENCH_C("gmp ", CC, mpn_mul_n, (mp_limb_t*)z, (const mp_limb_t*)x, (const mp_limb_t*)y, (int)N);
	void_ppp f = get_llvm_mulPre(N);
	printf("  ");
	CYBOZU_BENCH_C("llvm", CC, f, z, x, y);
	printf("  ");
	CYBOZU_BENCH_C("asm ", CC, mulT<N>, z, x, y);
#endif
}

CYBOZU_TEST_AUTO(mul)
{
	testMul<3>();
	testMul<4>();
	testMul<5>();
	testMul<6>();
	testMul<7>();
	testMul<8>();
}


