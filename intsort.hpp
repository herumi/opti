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
/*
	input
	a = [a3:a2:a1:a0], b = [b3:b2:b1:b0]
	output
	a = [min(ai, bi)], b = [max(ai, bi)]
*/
inline void vector_cmpswap(V128& a, V128& b)
{
	V128 t = pmaxud(a, b);
	a = pminud(a, b);
	b = t;
}

/*
	input
	a = [a3:a2:a1:a0], b = [b3:b2:b1:b0]
	output
	a = [    a3     :min(a2, b3):min(a1, b2):min(a0, b1)]
	b = [max(a2, b3):max(a1, b2):max(a0, b1):        b0 ]
*/
inline void vector_cmpswap_skew(V128& a, V128& b)
{
	V128 c = pslldq<4>(a); // [a2:a1:a0:0]
	V128 minbc = pminud(b, c); // [a2':a1':a0':0]
	b = pmaxud(b, c); // [b3':b2':b1':b0]
	a = psrldq<12>(a); // [0:0:0:a3]
	a = palignr<4>(a, minbc); // [a3:a2':a1':a0']
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

inline void sort_step1(V128 *va, size_t N)
{
	for (size_t i = 0; i < N; i += 4) {
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
	the following condition
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
#ifndef NDEBUG
	/*
		this condition is true because
		after calling vector_cmpswap_skew(va[N - 1], va[0]);
	*/
	const uint32_t *a = (const uint32_t*)va;
	const size_t e = N - 1;
	assert(a[e * 4 + 0] <= a[0 * 4 + 1]);
	assert(a[e * 4 + 1] <= a[0 * 4 + 2]);
	assert(a[e * 4 + 2] <= a[0 * 4 + 3]);
#endif
//	Xbyak::util::Clock clk;
//	clk.begin();
	for (size_t i = 0; i < N - 1; i++) {
		V128 a = va[i];
		V128 b = va[i + 1];
		V128 c = pmaxud(a, b);
		if (!ptest_cf(b, c)) {
			return false;
		}
	}
//	clk.end(); printf("clock=%.3f\n", clk.getClock() * 1e-3);
	return true;
}
inline bool sort_step2(V128 *va, size_t N)
{
	size_t gap = nextGap(N);
	while (gap > 1) {
		for (size_t i = 0; i < N - gap; i++) {
			vector_cmpswap(va[i], va[i + gap]);
		}
		for (size_t i = N - gap; i < N; i++) {
			vector_cmpswap_skew(va[i], va[i + gap - N]);
		}
		gap = nextGap(gap);
	}
	for (int i = 0; i < 15; i++) {
#if 1
		{
			V128 a = va[0];
			V128 b = va[1];
			va[0] = pminud(a, b);
			a = pmaxud(a, b);
			for (size_t i = 1; i < N - 1; i++) {
				b = va[i + 1];
				V128 t = pmaxud(a, b);
				va[i] = pminud(a, b);
				a = t;
			}
			va[N - 1] = a;
		}
#else
		for (size_t i = 0; i < N - 1; i++) {
			vector_cmpswap(va[i], va[i + 1]);
		}
#endif
		vector_cmpswap_skew(va[N - 1], va[0]);
		if (isSortedVec(va, N)) return true;
	}
	printf("!!! max loop\n");
	return false;
}

inline void sort_step3(V128 *vw, V128 *va, size_t N)
{
	for (size_t i = 0; i < N / 4; i++) {
		transpose(&va[i * 4]);
	}
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < N / 4; j++) {
			vw[j + i * (N / 4)] = va[j * 4 + i];
		}
	}
}

/*
	input
	a = [a3:a2:a1:a0], b = [b3:b2:b1:b0]
	output
	[b:a] = merge(a3, a2, a1, a0, b3, b2, b1, b0)
	mij = min(ai,bi)
	Mij = max(ai,bj)

	m00 M00 m11 M11  m22 M22 m33 M33
	     c   d   e    f   g   h
	    mcf Mcf  g    d  meh Meh
         s   t            u   v
	m00 msd Msd mtu  Mtu mgv Mgv M33

*/
inline void vector_merge(V128& a, V128& b)
{
	V128 m = pminud(a, b); // [h:f:d:m00] = [m33:m22:m11:m00]
	V128 M = pmaxud(a, b);      // [M33:g:e:c] = [M33:m22:M11:M00]
	V128 s0 = punpckhqdq(m, m); // [  h:f:h:f]
	V128 s1 = pminud(s0, M);    // [  h:f:u:s]
	V128 s2 = pmaxud(s0, M);    // [M33:g:v:t]
	V128 s3 = punpcklqdq(s1, punpckhqdq(M, M)); // [M33:g:u:s]
	V128 s4 = punpcklqdq(s2, m); // [d:m00:v:t]
	s4 = pshufd<MIE_PACK(2, 1, 0, 3)>(s4); // [m00:v:t:d]
	V128 s5 = pminud(s3, s4); // [m00:mgv:mtu:msd]
	V128 s6 = pmaxud(s3, s4); // [M33:Mgv:Mtu:Msd]
	V128 s7 = pinsrd<2>(s5, movd(s6)); // [m00:Msd:mtu:msd]
	V128 s8 = pinsrd<0>(s6, pextrd<2>(s5)); // [M33:Mgv:Mtu:mgv]
	a = pshufd<MIE_PACK(1, 2, 0, 3)>(s7);
	b = pshufd<MIE_PACK(3, 2, 0, 1)>(s8);
}

/*
	vo[aN + bN] <- merge(va[aN], vb[bN]);
*/
inline void merge(V128 *vo, const V128 *va, size_t aN, const V128 *vb, size_t bN)
{
	uint32_t aPos = 0;
	uint32_t bPos = 0;
	uint32_t outPos = 0;
	V128 vMin = va[aPos++];
	V128 vMax = vb[bPos++];
	while (aPos < aN && bPos < bN) {
		vector_merge(vMin, vMax);
		vo[outPos++] = vMin;
		if (((const uint32_t*)va)[aPos * 4] < ((const uint32_t*)vb)[bPos * 4]) {
			vMin = va[aPos++];
		} else {
			vMin = vb[bPos++];
		}
	}
}

inline void intsort(uint32_t *a, size_t N)
{
	assert((N % 16) == 0);
	V128 *va = reinterpret_cast<V128 *>(a);
	sort_step1(va, N / 4);
	sort_step2(va, N / 4);
	AlignedArray<uint32_t> work(N);
	V128 *vw = (V128*)&work[0];
	sort_step3(vw, va, N / 4);
	memcpy(va, vw, N * sizeof(a[0]));
}

