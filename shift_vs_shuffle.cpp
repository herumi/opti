#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>
#include <cybozu/xorshift.hpp>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

static inline __m128i getByShuffle(const char *p, size_t shift)
{
	static const unsigned char MIE_ALIGN(16) shiftPtn[32] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
	};
	__m128i v = _mm_loadu_si128((const __m128i*)p);
	v = _mm_shuffle_epi8(v, *(const __m128i*)(shiftPtn + shift));
	return v;
}

static inline __m128i getByShift(const char *p, size_t shift)
{
	__m128i v = _mm_loadu_si128((const __m128i*)p);
	switch (shift) {
	case 0: return v;
	case 1: return _mm_srli_si128(v, 1);
	case 2: return _mm_srli_si128(v, 2);
	case 3: return _mm_srli_si128(v, 3);
	case 4: return _mm_srli_si128(v, 4);
	case 5: return _mm_srli_si128(v, 5);
	case 6: return _mm_srli_si128(v, 6);
	case 7: return _mm_srli_si128(v, 7);
	case 8: return _mm_srli_si128(v, 8);
	case 9: return _mm_srli_si128(v, 9);
	case 10: return _mm_srli_si128(v, 10);
	case 11: return _mm_srli_si128(v, 11);
	case 12: return _mm_srli_si128(v, 12);
	case 13: return _mm_srli_si128(v, 13);
	case 14: return _mm_srli_si128(v, 14);
	case 15: return _mm_srli_si128(v, 15);
	default:
		 __builtin_unreachable();
	}
}

void put(const char *msg, __m128i x)
{
	uint8_t buf[16];
	printf("%s ", msg);
	_mm_storeu_si128((__m128i*)buf, x);
	for (int i = 0; i < 16; i++) {
		printf("%02x ", buf[i]);
	}
	printf("\n");
}
void test(const char *p)
{
	for (size_t shift = 0; shift < 16; shift++) {
		__m128i a = getByShift(p, shift);
		__m128i b = getByShuffle(p, shift);
		if (!_mm_testc_si128(a, b)) {
			puts("ERR");
			put("a", a);
			put("b", b);
			exit(1);
		}
	}
}

template<class F>
void bench(const char *p, F f)
{
	const int N = 1000000;
	cybozu::XorShift rg;
	Xbyak::util::Clock clk;
	__m128i x = _mm_setzero_si128();
	clk.begin();
	for (int i = 0; i < N; i++) {
		uint32_t r = rg.get32();
		x = _mm_add_epi8(x, f(p + (r & 15), (r >> 8) & 15));
	}
	clk.end();
	put("ret", x);
	printf("%.2f\n", clk.getClock() / double(N));
}
int main()
{
	char buf[] = "abcdefghijklmnopqrstuvVerfasefaawxyzz01234234242424";
	test(buf);
	bench(buf, getByShuffle);
	bench(buf, getByShift);
}
