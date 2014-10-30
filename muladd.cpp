#include <stdio.h>
#include <math.h>
#include "util.hpp"
#include <xbyak/xbyak_util.h>

#ifdef _MSC_VER
	#include "fvec.h"
	#define RESTRICT __restrict
#else
	#define RESTRICT
#endif

#define REAL float

void muladd0(REAL *c, const REAL *a, const REAL *b, REAL c1, REAL c2, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		c[i] = c1 * a[i] + c2 * b[i];
	}
}

void muladd1(REAL *RESTRICT c, const REAL *RESTRICT a, const REAL *RESTRICT b, REAL c1, REAL c2, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		c[i] = c1 * a[i] + c2 * b[i];
	}
}

void muladd2(REAL *RESTRICT c, const REAL *RESTRICT a, const REAL *RESTRICT b, REAL c1, REAL c2, size_t n)
{
#if 0//#ifdef _MSC_VER
	F32vec4 vc1(c1);
	F32vec4 vc2(c2);

	for (size_t i = 0; i < n; i += 4) {
		*(__m128*)&c[i] = vc1 * F32vec4(*(const __m128*)&a[i]) + vc2 * F32vec4(*(const __m128*)&b[i]);

	}
#else
	MIE_ALIGN(16) float vc1buf[4] = { c1, c1, c1, c1 };
	MIE_ALIGN(16) float vc2buf[4] = { c2, c2, c2, c2 };
	__m128 vc1; vc1 = *(const __m128*)vc1buf;
	__m128 vc2; vc2 = *(const __m128*)vc2buf;
	for (size_t i = 0; i < n; i += 4) {
		*(__m128* RESTRICT)&c[i] = _mm_add_ps(_mm_mul_ps(*(const __m128* RESTRICT)&a[i], vc1), _mm_mul_ps(*(const __m128* RESTRICT)&b[i], vc2));
	}
#endif
}

template<class T>
void initRand(T& v)
{
	for (size_t i = 0; i < v.size(); i++) {
		v[i] = (REAL)sin((i % 1000) * 0.23);
	}
}

template<class F>
void test(F f)
{
	const int n = 500000;
	AlignedArray<REAL> a(n), b(n), c(n);
	initRand(a);
	initRand(b);
	const int N = 10000;
	Xbyak::util::Clock clk;
	for (int i = 0; i < N; i++) {
		clk.begin();
		f(&c[0], &a[0], &b[0], 5, 6, n);
		clk.end();
	}
	printf("%.2fclk\n", clk.getClock() / REAL(N) / n);
}

int main()
{
	test(muladd0);
#if 0
	test(muladd1);
	test(muladd2);
#endif
}
