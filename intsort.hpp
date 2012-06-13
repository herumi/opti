#pragma once
/*
	implementation of AA-Sort with SSE4.1(not yet complete)
    @author herumi
	ref http://www.research.ibm.com/trl/people/inouehrs/pdf/SPE-SIMDsort.pdf
    @note modified new BSD license
    http://opensource.org/licenses/BSD-3-Clause
*/
#include "v128.h"
#include <assert.h>
#include <string.h>

namespace intsort_impl {

inline size_t nextGap(size_t n)
{
	n = (n * 10) / 13;
	// Combsort11. http://cs.clackamas.cc.or.us/molatore/cs260Spr03/combsort.htm
	if (n == 9 || n == 10) return 11;
	return n;
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

/*
	sort [x[i][j] | i<-[0,1,2,3]] for j = 0, 1, 2, 3
*/
inline void sort_step1_vec(V128 x[4])
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
		sort_step1_vec(&va[i]);
		transpose(&va[i]);
	}
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
	for (size_t i = 0; i < N - 1; i++) {
		V128 a = va[i];
		V128 b = va[i + 1];
		V128 c = pmaxud(a, b);
		c = psubd(c, b);
		// a <= b <=> max(a, b) == b
		// pcmpgtd is for signed dword integer
		if (!ptest_zf(c, c)) {
			return false;
		}
	}
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
	const int maxLoop = 10;
	for (int i = 0; i < maxLoop; i++) {
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
		if (isSortedVec(va, N)) {
			return true;
		}
	}
//	printf("!!! max loop %d for N=%d\n", maxLoop, (int)N);
	return false;
}

/*
	transpose and reorder va[0..N-1]
	input : va
	output : vw
*/
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
	assert(aN > 0 && bN > 0);
//printf("aN=%d, bN=%d\n", (int)aN, (int)bN);
	uint32_t aPos = 0;
	uint32_t bPos = 0;
	uint32_t outPos = 0;
	V128 vMin = va[aPos++];
	V128 vMax = vb[bPos++];
	for (;;) {
		vector_merge(vMin, vMax);
		vo[outPos++] = vMin;
		if (aPos < aN) {
			if (bPos < bN) {
				V128 ta = va[aPos];
				V128 tb = vb[bPos];
				if (movd(ta) <= movd(tb)) {
					vMin = ta;
					aPos++;
				} else {
					vMin = tb;
					bPos++;
				}
			} else {
				while (aPos < aN) {
					vMin = va[aPos++];
					vector_merge(vMin, vMax);
					vo[outPos++] = vMin;
				}
				break;
			}
		} else {
			while (bPos < bN) {
				vMin = vb[bPos++];
				vector_merge(vMin, vMax);
				vo[outPos++] = vMin;
			}
			break;
		}
	}
	vo[outPos] = vMax;
}

inline void sort_step123(V128 *vw, V128 *va, size_t N)
{
	sort_step1(va, N);
	bool isSorted = sort_step2(va, N);
	if (isSorted) {
		sort_step3(vw, va, N);
	} else {
		std::copy(va, va + N, vw);
		std::sort((uint32_t*)&vw[0], (uint32_t*)&vw[N]);
	}
}

} // intsort_impl

namespace mie {

inline void intsort(uint32_t *a, size_t N)
{
	assert((intptr_t(a) % 16) == 0);
	assert((N % 16) == 0);
	assert((N & (N - 1)) == 0); // now for only N is power of 2
	size_t BN = 8192; // maybe fastest for Xeon, i7
	const size_t N4 = N / 4;
	V128 *va = reinterpret_cast<V128 *>(a);
	if (N4 <= BN) {
		AlignedArray<uint32_t> work(N);
		V128 *vw = (V128*)&work[0];
		intsort_impl::sort_step123(vw, va, N4);
		memcpy(va, vw, N * sizeof(a[0]));
		return;
	}
	AlignedArray<uint32_t> work(N / 2);
	V128 *vw = (V128*)&work[0];
	assert((N % (N4 / BN)) == 0); // QQQ
	/*
		[ ] [<] [<] [<] [<] [<] [<] [<] ; va
		                [ ] [ ] [ ] [<] ; vw
	*/
	intsort_impl::sort_step123(&vw[N4 / 2 - BN], &va[N4 - BN], BN);

	for (size_t i = N4 - BN; i >= BN; i -= BN) {
		intsort_impl::sort_step123(&va[i], &va[i - BN], BN);
	}
	while (BN < N4 / 2) {
		intsort_impl::merge(&vw[N4 / 2 - BN * 2], &va[N4 - BN * 2], BN, &vw[N4 / 2 - BN], BN);
		for (size_t i = N4 - BN * 4; i > 0; i -= BN * 2) {
			intsort_impl::merge(&va[i + BN * 2], &va[i], BN, &va[i + BN * 3], BN);
		}
		intsort_impl::merge(&va[BN * 2], &va[BN], BN, &va[BN * 3], BN);
		BN *= 2;
	}
	intsort_impl::merge(va, &va[BN], BN, vw, BN);
}

} // mie

