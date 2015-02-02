#include "fast_diff.h"
#include <stdio.h>
#include <memory.h>
#include <cybozu/benchmark.hpp>
#include <cybozu/xorshift.hpp>
#include <string>
#include <cybozu/test.hpp>

CYBOZU_TEST_AUTO(same)
{
	char s[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	CYBOZU_TEST_ASSERT(!local_is_diff1(s, '0'));
	CYBOZU_TEST_ASSERT(!local_is_diff2(s, '0', '1'));
	CYBOZU_TEST_ASSERT(!local_is_diff3(s, '0', '1', '2'));
	CYBOZU_TEST_ASSERT(!local_is_diff4(s, '0', '1', '2', '3'));
	CYBOZU_TEST_ASSERT(!local_is_diff5(s, '0', '1', '2', '3', '4'));
	CYBOZU_TEST_ASSERT(!local_is_diff6(s, '0', '1', '2', '3', '4', '5'));
	CYBOZU_TEST_ASSERT(!local_is_diff7(s, '0', '1', '2', '3', '4', '5', '6'));
	CYBOZU_TEST_ASSERT(!local_is_diff8(s, '0', '1', '2', '3', '4', '5', '6', '7'));
	CYBOZU_TEST_ASSERT(!local_is_diff9(s, '0', '1', '2', '3', '4', '5', '6', '7', '8'));
	CYBOZU_TEST_ASSERT(!local_is_diff10(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'));
	CYBOZU_TEST_ASSERT(!local_is_diff11(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a'));
	CYBOZU_TEST_ASSERT(!local_is_diff12(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b'));
	CYBOZU_TEST_ASSERT(!local_is_diff13(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c'));
	CYBOZU_TEST_ASSERT(!local_is_diff14(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd'));
	CYBOZU_TEST_ASSERT(!local_is_diff15(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e'));
	CYBOZU_TEST_ASSERT(!local_is_diff16(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'));
	CYBOZU_TEST_ASSERT(!local_is_diff17(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g'));
	CYBOZU_TEST_ASSERT(!local_is_diff18(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'));
	CYBOZU_TEST_ASSERT(!local_is_diff19(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'));
	CYBOZU_TEST_ASSERT(!local_is_diff20(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'));
	CYBOZU_TEST_ASSERT(!local_is_diff21(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k'));
	CYBOZU_TEST_ASSERT(!local_is_diff22(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l'));
	CYBOZU_TEST_ASSERT(!local_is_diff23(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm'));
	CYBOZU_TEST_ASSERT(!local_is_diff24(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'));
	CYBOZU_TEST_ASSERT(!local_is_diff25(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o'));
	CYBOZU_TEST_ASSERT(!local_is_diff26(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'));
}

CYBOZU_TEST_AUTO(diff)
{
	char s[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	CYBOZU_TEST_ASSERT(local_is_diff1(s, '1'));
	CYBOZU_TEST_ASSERT(local_is_diff2(s, '0', '2'));
	CYBOZU_TEST_ASSERT(local_is_diff3(s, '0', '1', '3'));
	CYBOZU_TEST_ASSERT(local_is_diff4(s, '0', '1', '2', '4'));
	CYBOZU_TEST_ASSERT(local_is_diff5(s, '0', '1', '2', '3', '5'));
	CYBOZU_TEST_ASSERT(local_is_diff6(s, '0', '1', '2', '3', '4', '6'));
	CYBOZU_TEST_ASSERT(local_is_diff7(s, '0', '1', '2', '3', '4', '5', '7'));
	CYBOZU_TEST_ASSERT(local_is_diff8(s, '0', '1', '2', '3', '4', '5', '6', '8'));
	CYBOZU_TEST_ASSERT(local_is_diff9(s, '0', '1', '2', '3', '4', '5', '6', '7', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff10(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff11(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff12(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff13(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff14(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff15(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff16(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff17(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff18(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff19(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff20(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff21(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff22(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff23(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff24(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff25(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'X'));
	CYBOZU_TEST_ASSERT(local_is_diff26(s, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'X'));
}

static inline uint32_t local_is_diff7s(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6) { return local_is_diff4(p, c0, c1, c2, c3) | local_is_diff3(p + 4, c4, c5, c6); }

static inline uint64_t local_is_diff16s(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12, char c13, char c14, char c15) { return local_is_diff8(p, c0, c1, c2, c3, c4, c5, c6, c7) | local_is_diff8(p + 8, c8, c9, c10, c11, c12, c13, c14, c15); }

static inline uint64_t local_is_diff19s(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12, char c13, char c14, char c15, char c16, char c17, char c18) { return local_is_diff16(p, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) | local_is_diff4(p + 15, c15, c16, c17, c18); }
static inline uint64_t local_is_diff25s(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12, char c13, char c14, char c15, char c16, char c17, char c18, char c19, char c20, char c21, char c22, char c23, char c24) {
#if 0 // slow
	__m128i x0 = _mm_loadu_si128((const __m128i*)p);
	__m128i x1 = _mm_loadu_si128((const __m128i*)&p[9]);
	__m128i y0 = _mm_setr_epi32(uint32_t(v0), uint32_t(v0 >> 32), local_cat4(c8, c9, c10, c11), local_cat4(c12, c13, c14, c15));
	__m128i y1 = _mm_setr_epi32(local_cat4(c9, c10, c11, c12), local_cat4(c13, c14, c15, c16), local_cat4(c17, c18, c19, c20), local_cat4(c21, c22, c23, c24));
	x0 = _mm_xor_si128(x0, y0);
	x1 = _mm_xor_si128(x1, y1);
	x0 = _mm_or_si128(x0, x1);
	return _mm_testz_si128(x0, x0);
#else
	return local_is_diff16(p, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) || local_is_diff16(p + 9, c9, c10, c11, c12, c13, c14, c15, c16, c17, c18, c19, c20, c21, c22, c23, c24);
#endif
}

CYBOZU_TEST_AUTO(bench)
{
	const char str[] = "abcdefhgijhklmnopqrstuvwxyz";
	volatile int x = 0;
	for (int i = 0; i < 2; i++) {
		const char *s = str + i;
		printf("%s\n", i == 0 ? "aligned" : "not aligned");
		CYBOZU_BENCH("diff7 ", x += local_is_diff7, s, 'a', 'b', 'c', 'd', 'e', 'f', 'g');
		CYBOZU_BENCH("diff7s", x += local_is_diff7s, s, 'a', 'b', 'c', 'd', 'e', 'f', 'g');
		CYBOZU_BENCH("diff16 ", x += local_is_diff16, s, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p');
		CYBOZU_BENCH("diff16s", x += local_is_diff16s, s, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p');
		CYBOZU_BENCH("diff19 ", x += local_is_diff19, s, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's');
		CYBOZU_BENCH("diff19s", x += local_is_diff19s, s, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's');
		CYBOZU_BENCH("diff25 ", x += local_is_diff25, s, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y');
		CYBOZU_BENCH("diff25s", x += local_is_diff25s, s, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y');
	}
	printf("x=%d\n", x);
}
