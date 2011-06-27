/*
	require http://homepage1.nifty.com/herumi/soft/xbyak_e.html
	g++ -O3 -fomit-frame-pointer -march=core2 -msse4 -fno-operator-names strlen_sse42.cpp && ./a.out

	Xeon X5650 2.67GHz + Linux 2.6.32 + gcc 4.6.0
	ave             2      5      7     10     12     16     20     32     64    128    256    512   1024
	strlenANSI   12.22   4.96   4.03   3.18   2.87   2.38   2.08   1.55   1.00   0.70   0.51   0.38   0.34
	strlenC      13.69   8.34   6.86   5.57   5.04   4.34   3.89   3.27   2.74   2.42   2.28   2.19   2.21
	strlenSSE42   9.29   3.98   3.22   2.66   2.46   2.19   1.99   1.55   0.97   0.68   0.49   0.35   0.30

	Core i7-2600 CPU 3.40GHz + Linux 2.6.35 + gcc 4.4.5
	ave             2      5      7     10     12     16     20     32     64    128    256    512   1024
	strlenANSI   11.57   5.69   4.64   3.54   3.10   2.52   2.11   1.39   0.70   0.46   0.35   0.30   0.29
	strlenC      14.29   6.96   5.55   4.37   3.97   3.42   3.07   2.53   1.97   1.68   1.52   1.44   1.41
	strlenSSE42  12.51   5.22   4.08   3.26   2.95   2.57   2.28   1.53   0.82   0.51   0.39   0.32   0.30
*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <vector>
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

struct StrlenSSE42 : Xbyak::CodeGenerator {
	StrlenSSE42()
	{
		using namespace Xbyak;
#if defined(XBYAK64_WIN)
		const Reg64& p1 = rdx;
		const Reg64& c = rcx;
		const Reg64& a = rax;
		mov(rdx, rcx);
#elif defined(XBYAK64_GCC)
		const Reg64& p1 = rdi;
		const Reg64& c = rcx;
		const Reg64& a = rax;
#else
		const Reg32& p1 = edx;
		const Reg32& c = ecx;
		const Reg32& a = eax;
		mov(edx, ptr [esp + 4]);
#endif
		mov(eax, 0xff01);
		movd(xm2, eax);

		lea(a, ptr [p1 - 16]);
		xor(c, c);
	L("@@");
		add(a, 16);
		pcmpistri(xm2, ptr [a], 0x14);
		jnz("@b");
		add(a, c);
		sub(a, p1);
		ret();
	}
} strlenSSE42_code;

struct Result {
	int hit;
	double time;
	Result() {}
	Result(int hit, double time) : hit(hit), time(time) {}
	void put() const
	{
		printf("hit=%d time= %.2f clk\n", hit, time);
	}
};

void createTable(char *p, size_t num, int ave)
{
	char c = 1;
	for (size_t i = 0; i < num; i++) {
		p[i] = c++;
		if (c == 0) c = 1;
		if ((rand() % ave) == 0) p[i] = 0;
	}
	p[num - 1] = 0;
}

Result test(const char *top, size_t n, size_t count, size_t func(const char*))
{
	Xbyak::util::Clock clk;
	clk.begin();
	int hit = 0;
	for (size_t i = 0; i < count; i++) {
		const char *p = top;
		int remain = n;
		while (remain > 0) {
			size_t len = func(p);
			hit++;
			remain -= len + 1;
			p += len + 1;
		}
	}
	clk.end();
	return Result(hit, clk.getClock() / (double)count / n);
}

size_t strlenC(const char *p)
{
	const char *top = p;
	while (*p) {
		p++;
	}
	return p - top;
}

size_t (*strlenSSE42)(const char*) = (size_t (*)(const char*))strlenSSE42_code.getCode();

#define NUM_OF_ARRAY(x) (sizeof(x)/sizeof(x[0]))

int main(int argc, char *argv[])
{
	const size_t count = 4000;
	const size_t N = 100000;
	const int funcNum = 5;
	std::vector<char> v(N);

	char *begin = &v[0];

	static const int aveTbl[] = { 2, 5, 7, 10, 12, 16, 20, 32, 64, 128, 256, 512, 1024 };
	static const struct {
		const char *name;
		size_t (*func)(const char*);
	} funcTbl[] = {
		{ "strlenANSI ", strlen },
		{ "strlenC    ", strlenC },
		{ "strlenSSE42", strlenSSE42 },
	};
	Result rv[NUM_OF_ARRAY(funcTbl)][NUM_OF_ARRAY(aveTbl)];

	for (size_t i = 0; i < NUM_OF_ARRAY(aveTbl); i++) {
		int ave = aveTbl[i];
		createTable(begin, N, ave);

		printf("test %d, %d\n", (int)i, ave);
		for (size_t j = 0; j < NUM_OF_ARRAY(funcTbl); j++) {
			puts(funcTbl[j].name);
			rv[j][i] = test(begin, N, count, funcTbl[j].func);
			rv[j][i].put();
			if (rv[j][i].hit != rv[0][i].hit) {
				printf("ERROR!!! ok=%d, ng=%d\n", (int)rv[0][i].hit, (int)rv[j][i].hit);
			}
		}
	}

	puts("end");

	printf("ave        ");
	for (size_t i = 0; i < NUM_OF_ARRAY(aveTbl); i++) {
		printf("%6d ", aveTbl[i]);
	}
	printf("\n");
	for (int i = 0; i < NUM_OF_ARRAY(funcTbl); i++) {
		printf("%s ", funcTbl[i].name);
		for (size_t j = 0; j < NUM_OF_ARRAY(aveTbl); j++) {
			printf("%6.2f ", rv[i][j].time);
		}
		printf("\n");
	}
}

