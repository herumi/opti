#include <stdio.h>
#include <memory.h>
#include <cybozu/benchmark.hpp>
#include <cybozu/xorshift.hpp>
#include <string>

#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

static inline __m128i load16sub(const void *p, size_t shift)
{
  static const unsigned char shiftPtn[32] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
  };
  __m128i v = _mm_loadu_si128((const __m128i*)p);
  return _mm_shuffle_epi8(v, *(const __m128i*)(shiftPtn + shift));
}

static inline __m128i load16(const void *p, size_t n)
{
#if 1 // load data near page boundary safely
	const size_t addr = (size_t)p;
	const size_t bound = addr & 0xfff;
	if (bound >= 0xff1 && bound <= 0xfff - n) {
		return load16sub((const char*)(addr & -16), addr & 0xf);
	}
#endif
	return _mm_loadu_si128((const __m128i*)p);
}

static inline uint64_t get1(const char *p)
{
	return (uint8_t)*p;
}
static inline uint64_t get2(const char *p)
{
	uint16_t r;
	memcpy(&r, p, 2);
	return r;
}
static inline uint64_t get3(const char *p)
{
	return get2(p) | (get1(p + 2) << 16);
}
static inline uint64_t get4(const char *p)
{
	uint32_t r;
	memcpy(&r, p, 4);
	return r;
}
static inline uint64_t get5(const char *p)
{
	return get4(p) | (get1(p + 4) << 32);
}
static inline uint64_t get6(const char *p)
{
	return get4(p) | (get2(p + 4) << 32);
}
static inline uint64_t get7(const char *p)
{
	return get4(p) | (get3(p + 4) << 32);
}
static inline uint64_t get8(const char *p)
{
	uint64_t r;
	memcpy(&r, p, 8);
	return r;
}
static inline uint64_t mask(int x) { return (uint64_t(1) << x) - 1; }
static inline uint64_t is_same1(const char *p, const char *q) { return get1(p) ^ get1(q); }
static inline uint64_t is_same2(const char *p, const char *q) { return get2(p) ^ get2(q); }
static inline uint64_t is_same3(const char *p, const char *q) { return get3(p) ^ get3(q); }
static inline uint64_t is_same4(const char *p, const char *q) { return get4(p) ^ get4(q); }
static inline uint64_t is_same5(const char *p, const char *q) { return get5(p) ^ get5(q); }
static inline uint64_t is_same6(const char *p, const char *q) { return get6(p) ^ get6(q); }
static inline uint64_t is_same7(const char *p, const char *q) { return get7(p) ^ get7(q); }
static inline uint64_t is_same8(const char *p, const char *q) { return get8(p) ^ get8(q); }
static inline uint64_t is_same9(const char *p, const char *q)  { return is_same8(p, q) | is_same1(p + 8, q + 8); }
static inline uint64_t is_same10(const char *p, const char *q) { return is_same8(p, q) | is_same2(p + 8, q + 8); }
static inline uint64_t is_same11(const char *p, const char *q) { return is_same8(p, q) | is_same3(p + 8, q + 8); }
static inline uint64_t is_same12(const char *p, const char *q) { return is_same8(p, q) | is_same4(p + 8, q + 8); }
static inline uint64_t is_same13(const char *p, const char *q) { return is_same8(p, q) | is_same5(p + 8, q + 8); }
static inline uint64_t is_same14(const char *p, const char *q) { return is_same8(p, q) | is_same6(p + 8, q + 8); }
static inline uint64_t is_same15(const char *p, const char *q) { return is_same8(p, q) | is_same7(p + 8, q + 8); }

static inline uint64_t is_same2u(const char *p, const char *q) { return (get2(p) ^ get4(q)) & mask(16); }
static inline uint64_t is_same3u(const char *p, const char *q) { return (get3(p) ^ get4(q)) & mask(24); }
static inline uint64_t is_same5u(const char *p, const char *q) { return (get5(p) ^ get8(q)) & mask(40); }
static inline uint64_t is_same6u(const char *p, const char *q) { return (get6(p) ^ get8(q)) & mask(48); }
static inline uint64_t is_same7u(const char *p, const char *q) { return (get7(p) ^ get8(q)) & mask(56); }

static inline uint64_t is_same10u(const char *p, const char *q) { return is_same8(p, q) | is_same2u(p + 8, q + 8); }
static inline uint64_t is_same11u(const char *p, const char *q) { return is_same8(p, q) | is_same3u(p + 8, q + 8); }
static inline uint64_t is_same12u(const char *p, const char *q) { return is_same8(p, q) | is_same4(p + 8, q + 8); }
static inline uint64_t is_same13u(const char *p, const char *q) { return is_same8(p, q) | is_same5u(p + 8, q + 8); }
static inline uint64_t is_same14u(const char *p, const char *q) { return is_same8(p, q) | is_same6u(p + 8, q + 8); }
static inline uint64_t is_same15u(const char *p, const char *q) { return is_same8(p, q) | is_same7u(p + 8, q + 8); }

const uint8_t maskTbl[32] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};
static inline uint64_t is_same_fast(const char *p, const char *q, int n)
{
	__m128i x = load16(p, n);
	__m128i mask = _mm_loadu_si128((const __m128i*)(maskTbl + n));
	__m128i y = _mm_loadu_si128((const __m128i*)q);
	y = _mm_and_si128(y, mask);
	return _mm_testc_si128(x, y) != 0;
}
static inline uint64_t is_same3s(const char *p, const char *q)
{
	return is_same_fast(p, q, 3);
}
static inline uint64_t is_same5s(const char *p, const char *q)
{
	return is_same_fast(p, q, 5);
}
static inline uint64_t is_same6s(const char *p, const char *q)
{
	return is_same_fast(p, q, 6);
}
static inline uint64_t is_same7s(const char *p, const char *q)
{
	return is_same_fast(p, q, 7);
}
static inline uint64_t is_same9s(const char *p, const char *q)
{
	return is_same_fast(p, q, 9);
}
static inline uint64_t is_same10s(const char *p, const char *q)
{
	return is_same_fast(p, q, 10);
}
static inline uint64_t is_same11s(const char *p, const char *q)
{
	return is_same_fast(p, q, 11);
}
static inline uint64_t is_same12s(const char *p, const char *q)
{
	return is_same_fast(p, q, 12);
}
static inline uint64_t is_same13s(const char *p, const char *q)
{
	return is_same_fast(p, q, 13);
}
static inline uint64_t is_same14s(const char *p, const char *q)
{
	return is_same_fast(p, q, 14);
}
static inline uint64_t is_same15s(const char *p, const char *q)
{
	return is_same_fast(p, q, 15);
}
static inline uint64_t is_same16(const char *p, const char *q)
{
	__m128i x = _mm_loadu_si128((const __m128i*)p);
	__m128i y = _mm_loadu_si128((const __m128i*)q);
	return _mm_testc_si128(x, y) != 0;
}

static inline uint64_t is_same4_memcmp(const char *p, const char *q) { return memcmp(p, q, 4); }
static inline uint64_t is_same7_memcmp(const char *p, const char *q) { return memcmp(p, q, 7); }
static inline uint64_t is_same15_memcmp(const char *p, const char *q) { return memcmp(p, q, 15); }

#if 0

static inline bool is_same17(const char *p, const char *q) { return is_same16(p, q) ? is_same1(p + 16, q + 16) : false; }
static inline bool is_same18(const char *p, const char *q) { return is_same16(p, q) ? is_same2(p + 16, q + 16) : false; }
static inline bool is_same19(const char *p, const char *q) { return is_same16(p, q) ? is_same3(p + 16, q + 16) : false; }
static inline bool is_same20(const char *p, const char *q) { return is_same16(p, q) ? is_same4(p + 16, q + 16) : false; }
static inline bool is_same21(const char *p, const char *q) { return is_same16(p, q) ? is_same5(p + 16, q + 16) : false; }
static inline bool is_same22(const char *p, const char *q) { return is_same16(p, q) ? is_same6(p + 16, q + 16) : false; }
static inline bool is_same23(const char *p, const char *q) { return is_same16(p, q) ? is_same7(p + 16, q + 16) : false; }
static inline bool is_same24(const char *p, const char *q) { return is_same16(p, q) ? is_same8(p + 16, q + 16) : false; }
static inline bool is_same25(const char *p, const char *q) { return is_same16(p, q) ? is_same9(p + 16, q + 16) : false; }
static inline bool is_same26(const char *p, const char *q) { return is_same16(p, q) ? is_same10(p + 16, q + 16) : false; }
static inline bool is_same27(const char *p, const char *q) { return is_same16(p, q) ? is_same11(p + 16, q + 16) : false; }
static inline bool is_same28(const char *p, const char *q) { return is_same16(p, q) ? is_same12(p + 16, q + 16) : false; }
static inline bool is_same29(const char *p, const char *q) { return is_same16(p, q) ? is_same13(p + 16, q + 16) : false; }
static inline bool is_same30(const char *p, const char *q) { return is_same16(p, q) ? is_same14(p + 16, q + 16) : false; }
static inline bool is_same31(const char *p, const char *q) { return is_same16(p, q) ? is_same15(p + 16, q + 16) : false; }
static inline bool is_same32(const char *p, const char *q)
{
	__m256i x = _mm256_loadu_si256((const __m256i*)p);
	__m256i y = _mm256_loadu_si256((const __m256i*)q);
	return _mm256_testc_si256(x, y) != 0;
}
#endif

const size_t N = 10000;

int main()
{
	cybozu::XorShift rg;
	std::string v;
	v.resize(N + 32);
	for (size_t i = 0; i < N; i++) {
		v[i] = (uint8_t)('a' + (rg.get32() % 5));
	}
	const char str[] = "abcdeabaabbabeabcdababeaec";
	size_t pos = 0;
	const char*p = v.data();
	volatile uint64_t ret = 0;
	CYBOZU_BENCH("1  ", ++pos %= N; ret += !!is_same1, &p[pos], str); pos = 0;
	CYBOZU_BENCH("2  ", ++pos %= N; ret += !!is_same2, &p[pos], str); pos = 0;
	CYBOZU_BENCH("2u ", ++pos %= N; ret += !!is_same2u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("3  ", ++pos %= N; ret += !!is_same3, &p[pos], str); pos = 0;
	CYBOZU_BENCH("3u ", ++pos %= N; ret += !!is_same3u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("3s ", ++pos %= N; ret += !!is_same3s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("4  ", ++pos %= N; ret += !!is_same4, &p[pos], str); pos = 0;
	CYBOZU_BENCH("5  ", ++pos %= N; ret += !!is_same5, &p[pos], str); pos = 0;
	CYBOZU_BENCH("5u ", ++pos %= N; ret += !!is_same5u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("5s ", ++pos %= N; ret += !!is_same5s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("6  ", ++pos %= N; ret += !!is_same6, &p[pos], str); pos = 0;
	CYBOZU_BENCH("6u ", ++pos %= N; ret += !!is_same6u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("6s ", ++pos %= N; ret += !!is_same6s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("7  ", ++pos %= N; ret += !!is_same7, &p[pos], str); pos = 0;
	CYBOZU_BENCH("7u ", ++pos %= N; ret += !!is_same7u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("7s ", ++pos %= N; ret += !!is_same7s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("8  ", ++pos %= N; ret += !!is_same8, &p[pos], str); pos = 0;
	CYBOZU_BENCH("9  ", ++pos %= N; ret += !!is_same9, &p[pos], str); pos = 0;
	CYBOZU_BENCH("9s ", ++pos %= N; ret += !!is_same9s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("10 ", ++pos %= N; ret += !!is_same10, &p[pos], str); pos = 0;
	CYBOZU_BENCH("10u", ++pos %= N; ret += !!is_same10u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("10s", ++pos %= N; ret += !!is_same10s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("11 ", ++pos %= N; ret += !!is_same11, &p[pos], str); pos = 0;
	CYBOZU_BENCH("11u", ++pos %= N; ret += !!is_same11u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("11s", ++pos %= N; ret += !!is_same11u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("12 ", ++pos %= N; ret += !!is_same12, &p[pos], str); pos = 0;
	CYBOZU_BENCH("12u", ++pos %= N; ret += !!is_same12u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("12s", ++pos %= N; ret += !!is_same12s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("13 ", ++pos %= N; ret += !!is_same13, &p[pos], str); pos = 0;
	CYBOZU_BENCH("13u", ++pos %= N; ret += !!is_same13u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("13s", ++pos %= N; ret += !!is_same13s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("14 ", ++pos %= N; ret += !!is_same14, &p[pos], str); pos = 0;
	CYBOZU_BENCH("14u", ++pos %= N; ret += !!is_same14u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("14s", ++pos %= N; ret += !!is_same14s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("15 ", ++pos %= N; ret += !!is_same15, &p[pos], str); pos = 0;
	CYBOZU_BENCH("15u", ++pos %= N; ret += !!is_same15u, &p[pos], str); pos = 0;
	CYBOZU_BENCH("15s", ++pos %= N; ret += !!is_same15s, &p[pos], str); pos = 0;
	CYBOZU_BENCH("16 ", ++pos %= N; ret += !!is_same16, &p[pos], str); pos = 0;
	CYBOZU_BENCH("memcmp4", ++pos %= N; ret += !!is_same4_memcmp, &p[pos], str); pos = 0;
	CYBOZU_BENCH("memcmp7", ++pos %= N; ret += !!is_same7_memcmp, &p[pos], str); pos = 0;
	CYBOZU_BENCH("memcmp15", ++pos %= N; ret += !!is_same15_memcmp, &p[pos], str); pos = 0;
	printf("ret=%d\n", (int)ret);
}
