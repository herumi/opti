#include <stdint.h>
#include <cybozu/xorshift.hpp>
#include <memory.h>
#include <assert.h>
#ifdef _MSC_VER
#include <intrin.h>
#else
inline void __movsb(unsigned char *out, const unsigned char *src, size_t n)
{
	asm volatile("rep movsb":"=D"(out), "=S"(src), "=c"(n):"0"(out), "1"(src), "2"(n):"memory");
}
#endif
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>

uint64_t fcase(const uint8_t *m, size_t n)
{
	assert(n < 8);
	uint64_t b = 0;
    switch (n) {
    case 7: b |= uint64_t(m[6]) << 48;
    case 6: b |= uint64_t(m[5]) << 40;
    case 5: b |= uint64_t(m[4]) << 32;
    case 4: b |= uint64_t(m[3]) << 24;
    case 3: b |= uint64_t(m[2]) << 16;
    case 2: b |= uint64_t(m[1]) << 8;
    case 1: b |= uint64_t(m[0]);
    }
	return b;
}

uint64_t fmemcpy(const uint8_t *m, size_t n)
{
	assert(n < 8);
	uint64_t b = 0;
	memcpy(&b, m, n);
	return b;
}

uint64_t fasm(const uint8_t *m, size_t n)
{
	assert(n < 8);
	uint64_t b = 0;
	__movsb((uint8_t *)&b, m, n);
	return b;
}

void test(uint64_t f(const uint8_t *, size_t))
{
	cybozu::XorShift rg;
	const int N = 100000;
	uint64_t ret = 0;
	Xbyak::util::Clock clk;
	clk.begin();
	for (int i = 0; i < N; i++) {
		uint8_t buf[32];
		size_t begin = rg() % 8;
		size_t c = rg() % 8;
		for (size_t j = 0; j < c; j++) {
			buf[begin + j] = (uint8_t)rg();
		}
		uint64_t x = f(buf + begin, c);
		ret += x;
	}
	clk.end();
	printf("ret=%llx %.2f\n", (long long)ret, clk.getClock() / double(N));
}
int main()
{
	test(fcase);
	test(fmemcpy);
	test(fasm);
}

