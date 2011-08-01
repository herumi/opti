#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <vector>
#include <fstream>
#include <string>
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>
#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
/*
	read text from fileName and put it in textBuf and return aligned pointer to the data
*/
const char *LoadFile(std::string& textBuf, const std::string& fileName)
{
	std::ifstream ifs(fileName.c_str(), std::ios::binary);
	if (!ifs) return 0;
	ifs.seekg(0, std::ifstream::end);
	const size_t size = ifs.tellg();
	ifs.seekg(0);
	printf("size=%d\n", (int)size);
	textBuf.resize(size + 1 + 16);
	char *p = (char*)Xbyak::CodeArray::getAlignedAddress((Xbyak::uint8*)&textBuf[0]);
	ifs.read(p, size);
	p[size] = '\0';
	return p;
}


size_t countWord_C(const char *p)
{
	static const char alp_map8[32] = { 0, 0, 0, 0, 0x80, 0, 0xff, 0x3, 0xfe, 0xff, 0xff, 0x7, 0xfe, 0xff, 0xff, 0x7 };
	size_t i = 1,  cnt = 0;
	unsigned char cc, cc2;
	bool flag[3];
	cc2 = cc = p[0];
	flag[1] = alp_map8[cc >> 3] & (1 << (cc & 7));
	while (cc2) {
		cc2 = p[i];
		flag[2] = alp_map8[cc2 >> 3] & (1 << (cc2 & 7));
		if (!flag[2] && flag[1]) {
			cnt++;
		}
		flag[1] = flag[2];
		i++;
	}
	return cnt;
}

static char alnumTbl2[256];
size_t countWord_C2(const char *p)
{
	size_t count = 0;
	unsigned char c = *p++;
	char prev = alnumTbl2[c];
	while (c) {
		c = *p++;
		char cur = alnumTbl2[c];
		if (!cur && prev) {
			count++;
		}
		prev = cur;
	}
	return count;
}

/*
	see http://msirocoder.blog35.fc2.com/blog-entry-65.html
*/
MIE_ALIGN(16) static const char alnumTbl[16] = { '\'', '\'', '0', '9', 'A', 'Z', 'a', 'z', '\0' };
size_t countWord_SSE42(const char *p)
{
	const __m128i im = *(const __m128i*)alnumTbl;
	__m128i ret, x, prev;
	size_t count = 0;
	prev = _mm_setzero_si128();
	goto SKIP;
	do {
		p += 16;
	SKIP:
		ret = _mm_cmpistrm(im, *(const __m128i*)p, 0x4);
		x = _mm_slli_epi16(ret, 1);
		x = _mm_or_si128(prev, x);
		prev = _mm_srli_epi32(ret, 15);
		x = _mm_xor_si128(x, ret);
		count += _mm_popcnt_u32(_mm_cvtsi128_si32(x));
	} while (!_mm_cmpistrz(im, *(const __m128i*)p, 0x4));
	return count / 2;
}

struct CountWordSSE42 : Xbyak::CodeGenerator {
	CountWordSSE42()
	{
		inLocalLabel();
		using namespace Xbyak;
#if defined(XBYAK64_WIN)
		const Reg64& p = rdx;
		const Reg64& a = rax;
		const Reg64& c = rcx;
#elif defined(XBYAK64_GCC)
		const Reg64& p = rdi;
		const Reg64& a = rax;
		const Reg64& c = rcx;
#else
		const Reg32& p = edx;
		const Reg32& a = eax;
		const Reg32& c = ecx;
		mov(edx, ptr [esp + 4]);
#endif
		mov(a, (size_t)alnumTbl);
		pxor(xm3, xm3);
		movdqa(xm2, ptr [a]);
		xor(a, a);
		jmp(".in");
	L("@@");
		movdqa(xm4, xm0);
		psllw(xm4, 1);
		por(xm4, xm3);
		movdqa(xm3, xm0);
		pxor(xm4, xm0);
		psrld(xm3, 15);
		movd(Reg32(c.getIdx()), xm4);
		popcnt(c, c);
		add(p, 16);
		add(a, c);
	L(".in");
		movdqa(xm1, ptr [p]);
		pcmpistrm(xm2, xm1, 4);
		jnz("@b");
		movdqa(xm4, xm0);
		psllw(xm4, 1);
		por(xm4, xm3);
		movdqa(xm3, xm0);
		pxor(xm4, xm0);
		psrld(xm3, 15);
		movd(Reg32(c.getIdx()), xm4);
		popcnt(c, c);
		add(a, c);
		shr(a, 1);
		ret();
		outLocalLabel();
	}
} countWordSSE42_code;
size_t (*countWord_SSE42asm)(const char*) = (size_t (*)(const char*))countWordSSE42_code.getCode();

void test(const char *text, size_t (*countFunc)(const char *))
{
	const int N = 100;
	Xbyak::util::Clock clk;
	size_t c = 0;
	for (int i = 0; i < N; i++) {
		clk.begin();
		c += countFunc(text);
		clk.end();
	}
	printf("count=%d, clock=%.3fKclk\n", (int)c / N, clk.getClock() / (double)N * 1e-3);
}

int main()
{
	for (int i = 0; i < 256; i++) {
		alnumTbl2[i] = (i == '\'') || ('0' <= i && i <= '9') || ('a' <= i && i <= 'z') || ('A' <= i && i <= 'Z');
	}
	std::string textBuf;
	const char *text = LoadFile(textBuf, "test.txt");
	if (text == 0) return 1;
	test(text, countWord_C);
	test(text, countWord_C2);
	test(text, countWord_SSE42);
	test(text, countWord_SSE42asm);
#if 0
	MIE_ALIGN(16) const char src[] = "ute address DS";
	int a = countWord_C(src);
	int b = countWord_SSE42(src);
	printf("a=%d, b=%d\n", a, b);
#else
	size_t len = strlen(text);
	for (size_t i = 0; i < len; i += 16) {
		MIE_ALIGN(16) char src[17];
		memcpy(src, text + i, 16);
		src[16] = 0;
		int a = countWord_C(src);
		int b = countWord_SSE42(src);
		if (a != b) {
			printf("pos=%d, a=%d, b=%d\n", i, a, b);
			printf("[%s]\n", src);
		}
	}
#endif
}
