#pragma once
/*
	implementation of AA-Sort with SSE4.1(not yet complete)
	see http://www.trl.ibm.com/people/inouehrs/pdf/PACT2007-SIMDsort.pdf
    @author herumi
    @note modified new BSD license
    http://opensource.org/licenses/BSD-3-Clause
*/
#include "v128.h"
#include <assert.h>
#include <string.h>
#include <xbyak/xbyak_util.h>

inline size_t nextGap(size_t N)
{
	return (N * 10) / 13;
}

template<class T>
void combSort(T *a, size_t N)
{
	size_t gap = nextGap(N);
	for (;;) {
		bool isSwapped = false;
		for (size_t i = 0; i < N - gap; i++) {
			T x = a[i];
			T y = a[i + gap];
			if (x > y) {
				a[i + gap] = x;
				a[i] = y;
				isSwapped = true;
			}
		}
		if (gap == 1) {
			if (!isSwapped) break;
		} else {
			gap = nextGap(gap);
		}
	}
}

inline void vector_cmpswap(V128& a, V128& b)
{
	V128 t = pmaxud(a, b);
	a = pminud(a, b);
	b = t;
}

inline V128 vector_cmpswap_ck(V128& a, V128& b)
{
	V128 t = pmaxud(a, b);
	a = pminud(a, b);
	V128 ret = pcmpeqd(b, t);
	b = t;
	return ret;
}

inline void vector_cmpswap_skew(V128& a, V128& b)
{
	V128 c = pslldq<4>(a); // [a2:a1:a0:0]
	V128 minbc = pminud(b, c); // [a2':a1':a0':0]
	b = pmaxud(b, c); // [b3':b2':b1':b0]
	a = psrldq<12>(a); // [0:0:0:a3]
	a = palignr<4>(a, minbc); // [a3:a2':a1':a0']
}

inline V128 vector_cmpswap_skew_ck(V128& a, V128& b)
{
	V128 c = pslldq<4>(a); // [a2:a1:a0:0]
	V128 minbc = pminud(b, c); // [a2':a1':a0':0]
	V128 new_b = pmaxud(b, c); // [b3':b2':b1':b0]
	V128 ret = pcmpeqd(b, new_b);
	b = new_b;
	a = psrldq<12>(a); // [0:0:0:a3]
	a = palignr<4>(a, minbc); // [a3:a2':a1':a0']
	return ret;
}

inline void transpose(V128 x[4])
{
	V128 x0 = x[0];
	V128 x1 = x[1];
	V128 x2 = x[2];
	V128 x3 = x[3];
	V128 t0 = unpcklps(x0, x2);
	V128 t1 = unpcklps(x1, x3);
	V128 t2 = unpckhps(x0, x2);
	V128 t3 = unpckhps(x1, x3);
	x[0] = unpcklps(t0, t1);
	x[1] = unpckhps(t0, t1);
	x[2] = unpcklps(t2, t3);
	x[3] = unpckhps(t2, t3);
}

inline void sort_step1_sub(V128 x[4])
{
	V128 min01 = pminud(x[0], x[1]);
	V128 max01 = pmaxud(x[0], x[1]);
	V128 min23 = pminud(x[2], x[3]);
	V128 max23 = pmaxud(x[2], x[3]);
	x[0] = pminud(min01, min23);
	x[3] = pmaxud(max01, max23);
	V128 s = pmaxud(min01, min23);
	V128 t = pminud(max01, max23);
	x[1] = pminud(s, t);
	x[2] = pmaxud(s, t);
}

inline void sort_step1(uint32_t *a, size_t N)
{
	assert((N % 16) == 0);
	V128 *va = reinterpret_cast<V128 *>(a);
	for (size_t i = 0; i < N / 4; i += 4) {
		sort_step1_sub(&va[i]);
		transpose(&va[i]);
	}
}

bool isSorted(const uint32_t *a, size_t len)
{
	if (len > 0) {
		for (size_t i = 0; i < len - 1; i++) {
			if (a[i] > a[i + 1]) return false;
		}
	}
	return true;
}
/*
	Xeon
	all
	N =   65536 clk =  122K
	N =  131072 clk =  243K
	N =  262144 clk =  481K
	N =  524288 clk =  984K
	N = 1048576 clk = 1978K
	N = 2097152 clk = 4829K
*/
inline bool isSortedVec(const V128 *va, size_t N)
{
//	Xbyak::util::Clock clk;
//	clk.begin();
	for (size_t i = 0; i < N - 1; i++) {
		V128 a = va[i];
		V128 b = va[i + 1];
		V128 c = pmaxud(a, b);
		if (!ptest_cf(b, c)) {
//			clk.end(); printf("AAA %f\n", clk.getClock() * 1e-3);
			return false;
		}
	}
//	clk.end(); printf("BAA %f\n", clk.getClock() * 1e-3);
	return true;
}
inline bool sort_step2(uint32_t *a, size_t N)
{
	assert((N % 16) == 0);
	V128 *va = reinterpret_cast<V128 *>(a);
	size_t gap = nextGap(N / 4);
	while (gap > 1) {
		for (size_t i = 0; i < N / 4 - gap; i++) {
			vector_cmpswap(va[i], va[i + gap]);
		}
		for (size_t i = N / 4 - gap; i < N / 4; i++) {
			vector_cmpswap_skew(va[i], va[i + gap - N / 4]);
		}
		gap = nextGap(gap);
	}
	for (int i = 0; i < 15; i++) {
#if 0
#if 1
		V128 same;
		{
			V128 a = va[0];
			V128 b = va[1];
			va[0] = pminud(a, b);
			a = pmaxud(a, b);
			same = pcmpeqd(b, a);
			for (size_t i = 1; i < N / 4 - 1; i++) {
				b = va[i + 1];
				V128 t = pmaxud(a, b);
				va[i] = pminud(a, b);
				same = pand(same, pcmpeqd(b, t));
				a = t;
			}
			va[N / 4 - 1] = a;
		}
#else
		V128 same = pcmpeqd(Zero(), Zero());
		for (size_t i = 0; i < N / 4 - 1; i++) {
			V128 t = vector_cmpswap_ck(va[i], va[i + 1]);
			same = pand(same, t);
		}
#endif
		V128 t = vector_cmpswap_skew_ck(va[N / 4 - 1], va[0]);
		same = pand(same, t);
		if (pmovmskb(same) == 0xffff) {
//			fprintf(stderr, "i=%d\n", i);
			return true;
		}
#else
#if 1
		{
			V128 a = va[0];
			V128 b = va[1];
			va[0] = pminud(a, b);
			a = pmaxud(a, b);
			for (size_t i = 1; i < N / 4 - 1; i++) {
				b = va[i + 1];
				V128 t = pmaxud(a, b);
				va[i] = pminud(a, b);
				a = t;
			}
			va[N / 4 - 1] = a;
		}
#else
		for (size_t i = 0; i < N / 4 - 1; i++) {
			vector_cmpswap(va[i], va[i + 1]);
		}
#endif
		vector_cmpswap_skew(va[N / 4 - 1], va[0]);
		if (isSortedVec(va, N / 4)) return true;
#endif
	}
	printf("!!! max loop\n");
	return false;
}

inline void sort_step3(uint32_t *a, size_t N)
{
	assert((N % 16) == 0);
	V128 *va = reinterpret_cast<V128 *>(a);
	for (size_t i = 0; i < N / 16; i++) {
		transpose(&va[i * 4]);
	}
	AlignedArray<uint32_t> work(N);
	uint32_t *vw = &work[0];
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < N / 16; j++) {
			V128(&a[(j * 4 + i) * 4]).store(&vw[(j + i * (N / 16)) * 4]);
		}
	}
	memcpy(a, vw, N * sizeof(a[0]));
}

inline void intsort(uint32_t *a, size_t N)
{
	sort_step1(a, N);
	sort_step2(a, N);
	sort_step3(a, N);
}
