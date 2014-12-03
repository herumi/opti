#include <cybozu/mmap.hpp>
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>
#include <stdint.h>
#include <assert.h>
#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

typedef const char * (*FindType)(const char *, size_t, const char *, size_t);

FindType findRange0;
FindType findRange1;
FindType findRange2;
FindType findRange3;

const char *findRangeC(const char *p, size_t size, const char *range, size_t rangeSize)
{
	assert(rangeSize <= 16 && (rangeSize % 2) == 0);
	const uint8_t *r = (const uint8_t*)range;
	for (size_t i = 0; i < size; i++) {
		const uint8_t c = p[i];
		for (size_t j = 0; j < rangeSize; j += 2) {
			if (r[j] <= c && c <= r[j + 1]) return &p[i];
		}
	}
	return NULL;
}

const char *findRangeIn0(const char *p, size_t size, const char *range, size_t rangeSize)
{
	const __m128i r = _mm_loadu_si128((const __m128i*)range);
	__m128i v;
	for (;;) {
		v = _mm_loadu_si128((const __m128i*)p);
		if (!_mm_cmpestra(r, rangeSize, v, size, 4)) break;
		p += 16;
		size -= 16;
	}
	if (_mm_cmpestrc(r, rangeSize, v, size, 4)) {
		return p += _mm_cmpestri(r, rangeSize, v, size, 4);
	}
	return 0;
}

struct Code : Xbyak::CodeGenerator {
/*
	Win : p = rcx, size = rdx, range = r8, rangeSize = r9
	Linux : p = rdi, size = rsi, range = rdx, rangeSize = rcx
*/
	void setup()
	{
#ifdef _MSC_VER
		movdqu(xm0, ptr [r8]);
		mov(rax, r9);
		mov(r8, rcx);
#else
		movdqu(xm0, ptr [rdx]);
		mov(rax, rcx);
		mov(rdx, rsi);
		mov(r8, rdi);
#endif
	}
	Code()
	{
		findRange0 = getCurr<FindType>();
		gen0();
		align(16);
		findRange1 = getCurr<FindType>();
		gen1();
	}
	void gen0() {
		inLocalLabel();
		setup();
		jmp(".in");
	L(".lp");
		sub(rdx, rcx);
		add(r8, rcx);
	L(".in");
		pcmpestri(xm0, ptr [r8], 4);
		ja(".lp");
		jae(".exit");
		lea(rax, ptr [r8 + rcx]);
		ret();
	L(".exit");
		xor_(eax, eax);
		ret();
		outLocalLabel();
	}

	void gen1() {
		inLocalLabel();
		setup();
		jmp(".in");
	L(".lp");
		sub(rdx, 16);
		add(r8, 16);
	L(".in");
		pcmpestri(xm0, ptr [r8], 4);
		ja(".lp");
		jae(".exit");
		lea(rax, ptr [r8 + rcx]);
		ret();
	L(".exit");
		xor_(eax, eax);
		ret();
		outLocalLabel();
	}
} s_code;

size_t test(const char *p, size_t size, const char *range, size_t rangeSize, FindType find)
{
	const int N = 100000;
	size_t ret = 0;
	Xbyak::util::Clock clk;
	int n = 0;
	const char *const start = p;
	const size_t orgSize = size;
	clk.begin();
	for (;;) {
		const char *q = find(p, size, range, rangeSize);
#if 0
		const char *s = findRangeC(p, size, range, rangeSize);
		if (q != s) {
			printf("ERR q=%d, ok=%d(p=%s, range=%s)\n", q == 0 ? 0 : int(q - p), s == 0 ? 0 : int(s - p), p, range);
			exit(1);
		}
#endif
		n++;
		if (n == N) break;
		if (q == 0) {
			p = start;
			size = orgSize;
		}
		ret += q - p;
		p++;
		size--;
	}
	clk.end();
	printf("clk %.2f\n", clk.getClock() / double(N));
	return ret;
}
void testAll(const char *p, size_t size, const char *range, size_t rangeSize)
{
	printf("range `%s`\n", range);
	const FindType tbl[] = {
		findRangeIn0,
		findRange0,
		findRange1,
	};
	size_t retC = test(p, size, range, rangeSize, findRangeC);
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
		size_t r = test(p, size, range, rangeSize, tbl[i]);
		if (r != retC) {
			printf("ERR i=%d\n", (int)i);
			exit(1);
		}
	}
	puts("ok");
}

int main(int argc, char *argv[])
	try
{
	if (argc == 1) {
		puts("range <filename>");
		return 1;
	}
	cybozu::Mmap m(argv[1]);
	const struct {
		const char *p;
		size_t len;
	} tbl[] = {
		{ "aa", 2 },
		{ "azAZ09", 6 },
		{ "(){}[]<>", 8 },
		{ "\t\t  \n\n\r\r", 8 },
		{ "abcdefgh01234567", 16 },
	};
	for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
		testAll(m.get(), m.size(), tbl[i].p, tbl[i].len);
	}
} catch (cybozu::Exception& e) {
	printf("ERR %s\n", e.what());
	return 1;
}

