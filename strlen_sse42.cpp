/*
	require http://homepage1.nifty.com/herumi/soft/xbyak_e.html
	g++ -O3 -fomit-frame-pointer -march=core2 -msse4 -fno-operator-names strlen_sse42.cpp && ./a.out

	Xeon X5650 2.67GHz + Linux 2.6.32 + gcc 4.6.0
	ave           1.98   4.96   6.76   9.75  11.60  15.23  18.86  28.65  51.63  84.67 127.23 175.75 197.63
	strlenLIBC   12.27   5.01   4.07   3.22   2.91   2.43   2.13   1.57   1.02   0.71   0.52   0.38   0.34
	strlenC      14.07   8.44   6.91   5.58   5.05   4.34   3.90   3.24   2.74   2.43   2.32   2.23   2.20
	strlenSSE42   9.32   3.99   3.23   2.71   2.50   2.25   2.03   1.59   1.01   0.70   0.51   0.35   0.31

	Core i7-2600 CPU 3.40GHz + Linux 2.6.35 + gcc 4.4.5
	ave           1.98   4.96   6.76   9.75  11.60  15.23  18.86  28.65  51.63  84.67 127.23 175.75 197.63
	strlenLIBC   11.62   5.66   4.62   3.55   3.10   2.49   2.10   1.41   0.70   0.47   0.37   0.30   0.29
	strlenC      13.96   6.86   5.48   4.30   3.90   3.34   3.03   2.52   2.03   1.75   1.59   1.49   1.47
	strlenSSE42  12.62   5.27   4.13   3.28   2.96   2.59   2.31   1.53   0.81   0.52   0.39   0.32   0.29
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
	double len;
	double time;
	Result() {}
	Result(int hit, double len, double time) : hit(hit), len(len), time(time) {}
	void put() const
	{
		printf("hit=%d len=%.2f time= %.2f clk\n", hit, len, time);
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
	int hit = 0;
	for (size_t i = 0; i < count; i++) {
		clk.begin();
		const char *p = top;
		int remain = n;
		while (remain > 0) {
			size_t len = func(p);
			hit++;
			remain -= len + 1;
			p += len + 1;
		}
		clk.end();
	}
	return Result(hit, n / (double)hit * count, clk.getClock() / (double)count / n);
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
		{ "strlenLIBC ", strlen },
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

	printf("ave         ");
	for (size_t i = 0; i < NUM_OF_ARRAY(aveTbl); i++) {
//		printf("%6d ", aveTbl[i]);
		printf("%6.2f ", rv[0][i].len);
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

