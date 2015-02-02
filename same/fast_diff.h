#include <memory.h>
#include <stdint.h>
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

static inline uint32_t local_get1(const char *p)
{
  return (uint8_t)*p;
}
static inline uint32_t local_get2(const char *p)
{
  uint16_t r;
  memcpy(&r, p, 2);
  return r;
}
static inline uint32_t local_get4(const char *p)
{
  uint32_t r;
  memcpy(&r, p, 4);
  return r;
}
static inline uint64_t local_get8(const char *p)
{
  uint64_t r;
  memcpy(&r, p, 8);
  return r;
}

static inline uint32_t local_cat2(char c0, char c1) { return (uint32_t)c0 | ((uint32_t)c1 << 8); }
static inline uint32_t local_cat3(char c0, char c1, char c2) { return local_cat2(c0, c1) | ((uint32_t)c2 << 16); }
static inline uint32_t local_cat4(char c0, char c1, char c2, char c3) { return local_cat2(c0, c1) | (local_cat2(c2, c3) << 16); }
static inline uint64_t local_cat8(char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7) { return local_cat4(c0, c1, c2, c3) | ((uint64_t)local_cat4(c4, c5, c6, c7) << 32); }

static inline uint32_t local_is_diff1(const char *p, char c0) { return local_get1(p) ^ (uint8_t)c0; }
static inline uint32_t local_is_diff2(const char *p, char c0, char c1) { return local_get2(p) ^ local_cat2(c0, c1); }
static inline uint32_t local_is_diff3(const char *p, char c0, char c1, char c2) { return local_is_diff2(p, c0, c1) | local_is_diff1(p + 2, c2); }
static inline uint32_t local_is_diff4(const char *p, char c0, char c1, char c2, char c3) { return local_get4(p) ^ local_cat4(c0, c1, c2, c3); }
static inline uint32_t local_is_diff5(const char *p, char c0, char c1, char c2, char c3, char c4) { return local_is_diff4(p, c0, c1, c2, c3) | local_is_diff1(p + 4, c4); }
static inline uint32_t local_is_diff6(const char *p, char c0, char c1, char c2, char c3, char c4, char c5) { return local_is_diff4(p, c0, c1, c2, c3) | local_is_diff2(p + 4, c4, c5); }
static inline uint32_t local_is_diff7(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6) { return local_is_diff4(p, c0, c1, c2, c3) | local_is_diff4(p + 3, c3, c4, c5, c6); }
static inline uint64_t local_is_diff8(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7) { return local_get8(p) ^ local_cat8(c0, c1, c2, c3, c4, c5, c6, c7); }
static inline uint64_t local_is_diff9(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8) { return local_is_diff8(p, c0, c1, c2, c3, c4, c5, c6, c7) | local_is_diff1(p + 8, c8); }
static inline uint64_t local_is_diff10(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9) { return local_is_diff8(p, c0, c1, c2, c3, c4, c5, c6, c7) | local_is_diff2(p + 8, c8, c9); }
static inline uint64_t local_is_diff11(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10) { return local_is_diff8(p, c0, c1, c2, c3, c4, c5, c6, c7) | local_is_diff4(p + 7, c7, c8, c9, c10); }
static inline uint64_t local_is_diff12(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11) { return local_is_diff8(p, c0, c1, c2, c3, c4, c5, c6, c7) | local_is_diff4(p + 8, c8, c9, c10, c11); }
static inline uint64_t local_is_diff13(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12) { return local_is_diff8(p, c0, c1, c2, c3, c4, c5, c6, c7) | local_is_diff8(p + 5, c5, c6, c7, c8, c9, c10, c11, c12); }
static inline uint64_t local_is_diff14(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12, char c13) { return local_is_diff8(p, c0, c1, c2, c3, c4, c5, c6, c7) | local_is_diff8(p + 6, c6, c7, c8, c9, c10, c11, c12, c13); }
static inline uint64_t local_is_diff15(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12, char c13, char c14) { return local_is_diff8(p, c0, c1, c2, c3, c4, c5, c6, c7) | local_is_diff8(p + 7, c7, c8, c9, c10, c11, c12, c13, c14); }

static inline uint64_t local_is_diff16v(const char *p, uint64_t v0, uint64_t v1)
{
#ifdef __SSE4_1__
  __m128i x = _mm_loadu_si128((const __m128i*)p);
  __m128i y = _mm_setr_epi32(uint32_t(v0), uint32_t(v0 >> 32), uint32_t(v1), uint32_t(v1 >> 32));
  return _mm_testc_si128(x, y) == 0;
#else
  uint64_t x0 = local_get8(p);
  uint64_t x1 = local_get8(p + 8);
  return (x0 ^ v0) | (x1 ^ v1);
#endif
}
static inline uint64_t local_is_diff16(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12, char c13, char c14, char c15) { return local_is_diff16v(p, local_cat8(c0, c1, c2, c3, c4, c5, c6, c7), local_cat8(c8, c9, c10, c11, c12, c13, c14, c15)); }
static inline uint64_t local_is_diff17(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12, char c13, char c14, char c15, char c16) { return local_is_diff16(p, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) || local_is_diff1(p + 16, c16); }
static inline uint64_t local_is_diff18(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12, char c13, char c14, char c15, char c16, char c17) { return local_is_diff16(p, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) || local_is_diff2(p + 16, c16, c17); }
static inline uint64_t local_is_diff19(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12, char c13, char c14, char c15, char c16, char c17, char c18) { return local_is_diff16(p, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) || local_is_diff4(p + 15, c15, c16, c17, c18); }
static inline uint64_t local_is_diff20(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12, char c13, char c14, char c15, char c16, char c17, char c18, char c19) { return local_is_diff16(p, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) || local_is_diff8(p + 12, c12, c13, c14, c15, c16, c17, c18, c19); }
static inline uint64_t local_is_diff21(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12, char c13, char c14, char c15, char c16, char c17, char c18, char c19, char c20) { return local_is_diff16(p, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) || local_is_diff8(p + 13, c13, c14, c15, c16, c17, c18, c19, c20); }
static inline uint64_t local_is_diff22(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12, char c13, char c14, char c15, char c16, char c17, char c18, char c19, char c20, char c21) { return local_is_diff16(p, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) || local_is_diff8(p + 14, c14, c15, c16, c17, c18, c19, c20, c21); }
static inline uint64_t local_is_diff23(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12, char c13, char c14, char c15, char c16, char c17, char c18, char c19, char c20, char c21, char c22) { return local_is_diff16(p, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) || local_is_diff8(p + 15, c15, c16, c17, c18, c19, c20, c21, c22); }
static inline uint64_t local_is_diff24(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12, char c13, char c14, char c15, char c16, char c17, char c18, char c19, char c20, char c21, char c22, char c23) { return local_is_diff16(p, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) || local_is_diff8(p + 16, c16, c17, c18, c19, c20, c21, c22, c23); }
static inline uint64_t local_is_diff25(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12, char c13, char c14, char c15, char c16, char c17, char c18, char c19, char c20, char c21, char c22, char c23, char c24) { return local_is_diff16(p, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) || local_is_diff9(p + 16, c16, c17, c18, c19, c20, c21, c22, c23, c24); }
static inline uint64_t local_is_diff26(const char *p, char c0, char c1, char c2, char c3, char c4, char c5, char c6, char c7, char c8, char c9, char c10, char c11, char c12, char c13, char c14, char c15, char c16, char c17, char c18, char c19, char c20, char c21, char c22, char c23, char c24, char c25) { return local_is_diff16(p, c0, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15) || local_is_diff10(p + 16, c16, c17, c18, c19, c20, c21, c22, c23, c24, c25); }
