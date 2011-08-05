/*
	require http://homepage1.nifty.com/herumi/soft/xbyak_e.html

	Xeon X5650 2.67GHz + Linux 2.6.32 + gcc 4.6.0
	type=00h, model=0ch, family=06h, stepping=02h
	extModel=02, extFamily=00
	intel C version          :count=13428, clock=1714.922Kclk
	optimized intel C version:count=13428, clock=874.378Kclk
	SSE4.2 intrinsic version :count=13428, clock=46.510Kclk
	SSE4.2 Xbyak version0    :count=13428, clock=44.439Kclk
	SSE4.2 Xbyak version1    :count=13428, clock=53.375Kclk

	Core i7-2600 CPU 3.40GHz + Linux 2.6.35 + gcc 4.4.5
	type=00h, model=0ah, family=06h, stepping=07h
	extModel=02, extFamily=00
	intel C version          :count=13428, clock=854.592Kclk
	optimized intel C version:count=13428, clock=456.279Kclk
	SSE4.2 intrinsic version :count=13428, clock=42.778Kclk
	SSE4.2 Xbyak version0    :count=13428, clock=41.940Kclk
	SSE4.2 Xbyak version1    :count=13428, clock=32.975Kclk

	Core i7-2600 Cpu 3.40GHz + Windows 7(64bit) + VC2010
	type=00h, model=0ah, family=06h, stepping=07h
	extModel=02, extFamily=00
	intel C version          :count=13428, clock=979.836Kclk
	optimized intel C version:count=13428, clock=623.810Kclk
	SSE4.2 intrinsic version :count=13428, clock=42.633Kclk
	SSE4.2 Xbyak version0    :count=13428, clock=43.063Kclk
	SSE4.2 Xbyak version1    :count=13428, clock=32.466Kclk

*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <vector>
#include <fstream>
#include <string>
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>
#include "cpu.h"
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
	explicit CountWordSSE42(int mode)
	{
		inLocalLabel();
		using namespace Xbyak;
#if defined(XBYAK64_WIN)
		const Reg64& p = rcx;
		const Reg64& a = rax;
		const Reg64& d = rdx;
#elif defined(XBYAK64_GCC)
		const Reg64& p = rdi;
		const Reg64& a = rax;
		const Reg64& d = rdx;
#else
		const Reg32& p = ecx;
		const Reg32& a = eax;
		const Reg32& d = edx;
		mov(p, ptr [esp + 4]);
#endif
		mov(a, (size_t)alnumTbl);
		pxor(xm3, xm3);
		movdqa(xm2, ptr [a]);
		xor(a, a);
		jmp(".in");
		switch (mode) {
		case 0:
		// faster on Xeon(43.5clk vs 53.3clk)
	L("@@");
		add(p, 16);
	L(".in");
			movdqa(xm1, ptr [p]);
			pcmpistrm(xm2, xm1, 4);
			movdqa(xm4, xm0);
			psllw(xm4, 1);
			por(xm4, xm3);
			pxor(xm4, xm0);
	#ifdef XBYAK64
			movq(d, xm4);
	#else
			movd(d, xm4);
	#endif
			movdqa(xm3, xm0);
			popcnt(d, d);
			add(a, d);
			psrld(xm3, 15);
			pcmpistrm(xm2, xm1, 4);
			jnz("@b");
			shr(a, 1);
			break;
		case 1:
		// faster on i7(43clk vs 32clk)
		L("@@");
			movdqa(xm4, xm0);
			psllw(xm4, 1);
			por(xm4, xm3);
			movdqa(xm3, xm0);
			pxor(xm4, xm0);
			psrld(xm3, 15);
	#ifdef XBYAK64
			movq(d, xm4);
	#else
			movd(d, xm4);
	#endif
			popcnt(d, d);
			add(p, 16);
			add(a, d);
		L(".in");
			pcmpistrm(xm2, ptr [p], 4);
			jnz("@b");
			movdqa(xm4, xm0);
			psllw(xm4, 1);
			por(xm4, xm3);
			pxor(xm4, xm0);
			movd(Reg32(d.getIdx()), xm4);
			popcnt(d, d);
			add(a, d);
			shr(a, 1);
			break;
		default:
			fprintf(stderr, "err mode=%d\n", mode);
			break;
		}
		ret();
		outLocalLabel();
	}
};
CountWordSSE42 countWordSSE42_code0(0);
CountWordSSE42 countWordSSE42_code1(1);
size_t (*countWord_SSE42asm0)(const char*) = (size_t (*)(const char*))countWordSSE42_code0.getCode();
size_t (*countWord_SSE42asm1)(const char*) = (size_t (*)(const char*))countWordSSE42_code1.getCode();

void check(size_t (*countFunc)(const char*))
{
	for (int len = 1; len <= 20; len++) {
		MIE_ALIGN(16) char str[32];
		str[len] = '\0';
		for (int ptn = 0; ptn < (1 << len); ptn++) {
			for (int j = 0; j < len; j++) {
				str[j] = ptn & (1 << j) ? 'a' : '.';
			}
			size_t a = countWord_C2(str);
			size_t b = countFunc(str);
			if (a != b) {
				printf("err str='%s', a=%d, b=%d\n", str, (int)a, (int)b);
				exit(1);
			}
		}
	}
}

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
	check(countFunc);
}

int main(int argc, char *argv[])
{
	argc--, argv++;
	const char *file = "test.txt";
	if (argc) file = *argv;
	fprintf(stderr, "load %s\n", file);
	for (int i = 0; i < 256; i++) {
		alnumTbl2[i] = (i == '\'') || ('0' <= i && i <= '9') || ('a' <= i && i <= 'z') || ('A' <= i && i <= 'Z');
	}
	std::string textBuf;
	const char *text = LoadFile(textBuf, file);
	if (text == 0) return 1;
	VersionInfo vi;
	printf("type=%02xh, model=%02xh, family=%02xh, stepping=%02xh\n", vi.type, vi.model, vi.family, vi.stepping);
	printf("extModel=%02x, extFamily=%02x\n", vi.extModel, vi.extFamily);
	printf("intel C version          :");
	test(text, countWord_C);
	printf("optimized intel C version:");
	test(text, countWord_C2);
	Xbyak::util::Cpu cpu;
	if (!cpu.has(Xbyak::util::Cpu::tSSE42)) {
		fprintf(stderr, "SSE42 is not supported\n");
		return 1;
	}
	printf("SSE4.2 intrinsic version :");
	test(text, countWord_SSE42);
	printf("SSE4.2 Xbyak version0    :");
	test(text, countWord_SSE42asm0);
	printf("SSE4.2 Xbyak version1    :");
	test(text, countWord_SSE42asm1);
}

