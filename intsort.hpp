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

inline void vector_cmpswap_skew(V128& a, V128& b)
{
#if 0
	V128 c = psrldq<4>(a); // [0:a3:a2:a1]
	V128 minbc = pminud(b, c); // [0:a3':a2':a1']
	b = pmaxud(b, c); // [b3:b2':b1':b0']
	a = pslldq<12>(a); // [a0:0:0:0]
	a = palignr<12>(minbc, a); // [a3':a2':a1':a0]
#else
	V128 c = pslldq<4>(a); // [a2:a1:a0:0]
	V128 minbc = pminud(b, c); // [a2':a1':a0':0]
	b = pmaxud(b, c); // [b3':b2':b1':b0]
	a = psrldq<12>(a); // [0:0:0:a3]
	a = palignr<4>(a, minbc); // [a3:a2':a1':a0']
#endif
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

inline void sort_step2(uint32_t *a, size_t N)
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
	for (int i = 0; i < 8; i++) {
		for (size_t i = 0; i < N / 4 - 1; i++) {
			vector_cmpswap(va[i], va[i + 1]);
		}
		vector_cmpswap_skew(va[N / 4 - 1], va[0]);
//		if (isSorted(a, N)) return true;
	}
//	puts("ERR!!! not sorted");
//	return false;
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
