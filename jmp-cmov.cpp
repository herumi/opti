/*
	require http://homepage1.nifty.com/herumi/soft/xbyak_e.html

	benchmark of jmp/cmov/setg
	g++ -O3 -fomit-frame-pointer -fno-operator-names jmp-cmov.cpp

Xeon X5650 2.67GHz + Linux 2.6.32 + gcc 4.6.0
--- test1 ---
name    rand  first    inc   inc2
STL    5.016  2.615  2.615  2.615
jmp    3.414  1.745  1.746  3.481
cmov   5.008  2.613  2.612  2.653
maxps  30.741  26.581  26.587  26.547
pmaxsd 0.437  0.437  0.437  0.437
maxps for int data:26.572Kclk
maxps for valid float data:0.648Kclk
--- test2 ---
        0.00   0.25   0.50   0.75   1.00
STL    2.617  2.617  2.617  2.616  2.616
jmp    2.622  8.426 12.539  9.709  1.859
setg   2.652  2.618  2.616  2.624  2.617
adc    2.614  2.614  2.620  2.649  2.620
setg2  2.616  2.616  2.617  2.616  2.615

Core i7-2600 CPU 3.40GHz + Linux 2.6.35 + gcc 4.4.5
--- test1 ---
name    rand  first    inc   inc2
STL    3.760  2.751  2.765  2.758
jmp    1.880  1.829  1.850  3.617
cmov   2.732  2.714  2.762  2.719
maxps  0.686  0.689  0.677  0.687
pmaxsd 0.276  0.279  0.287  0.273
maxps for int data:0.696Kclk
maxps for valid float data:0.713Kclk
--- test2 ---
        0.00   0.25   0.50   0.75   1.00
STL    2.749  2.754  2.755  2.725  2.718
jmp    1.878  6.302 11.256  7.688  1.860
setg   2.019  2.019  2.142  2.028  2.086
adc    1.826  1.827  1.844  1.851  1.814
setg2  1.818  1.818  1.863  1.844  1.842

Core i7-2600 Cpu 3.40GHz + Windows 7(64bit) + VC2010
--- test1 ---
name  rand  first    inc   inc2
STL  8.341  8.211  8.168  8.165
jmp  1.904  1.808  1.862  3.622
cmov 2.729  2.729  2.748  2.715
--- test2 ---
        0.00   0.25   0.50   0.75   1.00
STL    1.857  6.172 11.025  7.935  1.818
jmp    1.834  5.953 10.605  7.239  1.828
setg   2.027  2.035  2.039  2.069  2.012
adc    1.857  1.866  1.860  1.804  1.820
setg2  1.845  1.808  1.826  1.847  1.833

PentiumD 2.8GHz + Windows Xp(32bit) + VC2008
--- test1 ---
name    rand  first    inc   inc2
STL    14.077  13.654  14.065  14.066
jmp    3.422  3.649  3.447  4.199
cmov   10.053  10.047  10.073  10.045
maxps  1.259  1.258  1.261  1.260
pmaxsd
maxps for int data:1.265Kclk
maxps for valid float data:1.256Kclk
--- test2 ---
        0.00   0.25   0.50   0.75   1.00
STL    4.394 17.740 24.018 18.779  4.098
jmp    3.856 20.154 26.415 19.884  4.289
setg   5.908  5.902  5.893  5.979  5.912
adc    5.812  5.662  5.656  5.624  5.705
setg2  5.122  5.141  5.131  5.124  5.121

Core Duo T2300 1.6GHz + Linux(32bit) + gcc 4.6.0
--- test1 ---
name    rand  first    inc   inc2
STL    4.354  4.358  4.264  4.301
jmp    2.544  2.513  2.535  4.232
cmov   3.604  3.593  3.687  3.635
maxps  31.171  31.094  31.129  31.076
pmaxsd
maxps for int data:31.111Kclk
maxps for valid float data:1.092Kclk
--- test2 ---
        0.00   0.25   0.50   0.75   1.00
STL    4.495  4.440  4.434  4.442  4.552
jmp    2.956  9.288 12.029  9.458  2.863
setg   3.937  3.972  3.878  3.915  4.005
adc    3.446  3.500  3.394  3.430  3.404
setg2  3.473  3.491  3.420  3.470  3.536
*/
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "xbyak/xbyak.h"
#include "xbyak/xbyak_util.h"
#include "util.hpp"

static const Xbyak::util::Cpu cpu;

typedef AlignedArray<int> IntVec;
typedef std::vector<double>DoubleVec;

const int MaxCount = 10000;

/*
	x[i] is always 1
	y[i] is 0 or 2
*/
void Init(IntVec& x, IntVec& y, size_t n, int rate)
{
	XorShift128 r;
	x.resize(n);
	y.resize(n);
	for (size_t i = 0; i < n; i++) {
		x[i] = 1;
		y[i] = int(r.get() % 1000) >= rate ? 0 : 2;
	}
}

template<class T>
void InitRandom(T& x, size_t n)
{
	XorShift128 r;
	x.resize(n);
	for (size_t i = 0; i < n; i++) {
		x[i] = typename T::value_type(r.get() % 65537);
	}
}

int getMaxBySTL(const int *x, size_t n)
{
	return *std::max_element(x, x + n);
}

size_t countMax_C(const int *x, const int *y, size_t n)
{
	size_t ret = 0;
	for (size_t i = 0; i < n; i++) {
		if (x[i] > y[i]) ret++;
	}
	return ret;
}

void Test1(DoubleVec& dv, const IntVec& x, int f(const int*, size_t n))
{
	Xbyak::util::Clock clk;
	const size_t n = x.size();
	const int *p = &x[0];
	int ret = 0;
	for (int i = 0; i < MaxCount; i++) {
		clk.begin();
		ret += f(p, n);
		clk.end();
	}
	double c = clk.getClock() / ((double)MaxCount * n);
	dv.push_back(c);
	printf("ret=%d, %fclk\n", ret / MaxCount, c);
}

void Test2(DoubleVec& dv, const IntVec& a, const IntVec& b, size_t f(const int*, const int *, size_t n))
{
	Xbyak::util::Clock clk;
	const size_t n = a.size();
	const int *p = &a[0];
	const int *q = &b[0];
	size_t ret = 0;
	for (int i = 0; i < MaxCount; i++) {
		clk.begin();
		ret += f(p, q, n);
		clk.end();
	}
	double c = clk.getClock() / ((double)MaxCount * n);
	dv.push_back(c);
	printf("ret=%d, %fclk\n", (int)ret / MaxCount, c);
}

static struct Func1Info {
	const char *name;
	int (*f)(const int*, size_t);
	bool useSSE4;
} func1Tbl[] = {
	{ "STL   ", getMaxBySTL, false },
	{ "jmp   ", 0, false },
	{ "cmov  ", 0, false },
	{ "maxps ", 0, false },
	{ "pmaxsd", 0, true },
};

static struct Func2Info {
	const char *name;
	size_t (*f)(const int *, const int*, size_t);
} func2Tbl[] = {
	{ "STL  ", countMax_C },
	{ "jmp  ", 0 },
	{ "setg ", 0 },
	{ "adc  ", 0 },
	{ "setg2", 0 },
};

struct Code : public Xbyak::CodeGenerator {
	// int getMax(const int *x, size_t n); // n > 0
	void genGetMax(int mode)
	{
		using namespace Xbyak;
		inLocalLabel();
		const Reg32& a = eax;
#if defined(XBYAK64_WIN)
		const Reg64& x = rcx;
		const Reg64& n = rdx;
		xor(rax, rax);
#elif defined(XBYAK64_GCC)
		const Reg64& x = rdi;
		const Reg64& n = rsi;
		xor(rax, rax);
#else
		const Reg32& x = ecx;
		const Reg32& n = edx;
		mov(x, ptr [esp + 4]);
		mov(n, ptr [esp + 8]);
#endif
		if (mode < 2) {
			bool useCmov = mode == 1;
			mov(a, ptr [x]);
			cmp(n, 1);
			je(".exit");
			lea(x, ptr [x + n * 4]);
			neg(n);
			add(n, 1);
		L("@@");
			if (useCmov) {
				cmp(a, ptr [x + n * 4]);
				cmovl(a, ptr [x + n * 4]);
			} else {
				cmp(a, ptr [x + n * 4]);
				jge(".skip");
				mov(a, ptr [x + n * 4]);
			L(".skip");
			}
			add(n, 1);
			jne("@b");
		L(".exit");
		} else {
			switch (mode) {
			case 2:
			case 3:
				movdqa(xm0, ptr [x]);
				lea(x, ptr [x + n * 4]);
				neg(n);
				add(n, 4);
			L("@@");
				if (mode == 2) {
					maxps(xm0, ptr [x + n * 4]);
				} else {
					pmaxsd(xm0, ptr [x + n * 4]);
				}
				add(n, 4);
				jnz("@b");
				movhlps(xm1, xm0);
				if (mode == 2) {
					maxps(xm0, xm1);
				} else {
					pmaxsd(xm0, xm1);
				}
				movdqa(xm1, xm0);
				psrldq(xm1, 4);
				if (mode == 2) {
					maxps(xm0, xm1);
				} else {
					pmaxsd(xm0, xm1);
				}
				movd(eax, xm0);
				break;
			default:
				fprintf(stderr, "bad mode=%d\n", mode);
				exit(1);
			}
		}
		ret();
		outLocalLabel();
	}
	/*
		size_t getCountMax(const int *x, const int *y, size_t n); // n > 0
		x[i] > y[i] となる個数を返す
		mode = 0 : use jmp
		       1 : use setg
		       2 : use adc
		       3 : use setg(wo. movzx)
	*/
	void genCountMax(int mode)
	{
		using namespace Xbyak;
		inLocalLabel();
#if defined(XBYAK64_WIN)
		const Reg64& x = rcx;
		const Reg64& y = r9;
		const Reg64& n = r8;
		const Reg32& t = edx;
		const Reg32& t2 = r10d;
		const Reg64& a = rax;
		mov(r9, rdx); // to use lower 8bit of t
		xor(rdx, rdx);
#elif defined(XBYAK64_GCC)
		const Reg64& x = rdi;
		const Reg64& y = rsi;
		const Reg64& n = rdx;
		const Reg32& t = ecx;
		const Reg32& t2 = r8d;
		const Reg64& a = rax;
		xor(rcx, rcx);
#else
		const Reg32& x = esi;
		const Reg32& y = edx;
		const Reg32& n = ecx;
		const Reg32& t = ebx;
		const Reg32& t2 = edi;
		const Reg32& a = eax;
		push(ebx);
		push(esi);
		int P = 4 * 2;
		if (mode == 3) {
			P = 4 * 3;
			push(edi);
			xor(ebx, ebx);
		}
		mov(x, ptr [esp + P + 4]);
		mov(y, ptr [esp + P + 8]);
		mov(n, ptr [esp + P + 12]);
#endif
		const Reg8& low8 = Reg8(t.getIdx());
		lea(x, ptr [x + n * 4]);
		lea(y, ptr [y + n * 4]);
		neg(n);
		xor(a, a);

	L(".lp");
		switch (mode) {
		case 0:
			mov(t, ptr [x + n * 4]);
			cmp(t, ptr [y + n * 4]);
			jle(".skip");
			add(a, 1);
		L(".skip");
			break;
		case 1:
			mov(t, ptr [x + n * 4]);
			cmp(t, ptr [y + n * 4]);
			setg(low8);
#ifdef XBYAK64
			movzx(Reg64(t.getIdx()), low8);
#else
			movzx(t, low8);
#endif
			add(a, t);
			break;
		case 2:
			mov(t, ptr [y + n * 4]);
			cmp(t, ptr [x + n * 4]);
			adc(a, 0);
			break;
		case 3:
			mov(t2, ptr [x + n * 4]);
			cmp(t2, ptr [y + n * 4]);
			setg(low8);
			add(a, t);
			break;
		}
		add(n, 1);
		jne(".lp");
	L(".exit");
#ifdef XBYAK32
		if (mode == 3) {
			pop(edi);
		}
		pop(esi);
		pop(ebx);
#endif
		ret();
		outLocalLabel();
	}
};

void Test1All(std::vector<DoubleVec>& ret1, const IntVec& a)
{
	for (size_t i = 0; i < NUM_OF_ARRAY(func1Tbl); i++) {
		if (func1Tbl[i].useSSE4 && !cpu.has(Xbyak::util::Cpu::tSSE41)) break;
		Test1(ret1[i], a, func1Tbl[i].f);
	}
}

int main()
{
	try {
		IntVec a;
		Code code;
		std::vector<DoubleVec> ret1(NUM_OF_ARRAY(func1Tbl));
		for (size_t i = 1; i < NUM_OF_ARRAY(func1Tbl); i++) {
			code.align(16);
			func1Tbl[i].f = (int (*)(const int*, size_t))code.getCurr();
			code.genGetMax((int)i - 1);
		}

		/* 乱数 */
		const size_t N = 8192;
		InitRandom(a, N);
		printf("rand max pos=%d\n", int(std::max_element(a.begin(), a.end()) - a.begin()));
		Test1All(ret1, a);
		/* 最初が一番大きい */
		puts("fst is max");
		a[0] = 100000;
		Test1All(ret1, a);
		puts("inc");
		/* 単調増加 */
		for (int i = 0; i < (int)a.size(); i++) {
			a[i] = i;
		}
		Test1All(ret1, a);

		/* やや単調増加 */
		puts("inc2");
		XorShift128 r;
		for (int i = 0; i < (int)a.size(); i += 4) {
			a[i] = (r.get() % 1) ? i : i - 2;
			a[i + 1] = i - 1;
			a[i + 2] = i - 1;
			a[i + 3] = i - 1;
		}
		Test1All(ret1, a);

		///
		for (int i = 1; i <= 4; i++) {
			code.align(16);
			func2Tbl[i].f = (size_t (*)(const int*, const int*, size_t))code.getCurr();
			code.genCountMax(i - 1);
		}

		std::vector<DoubleVec> ret2(5);
		for (int i = 0; i < 5; i++) {
			IntVec b;
			int rate = i * 250;
			Init(a, b, 8192, rate);
			a[0] = b[0] = 5; // check equal

			printf("rate=%d\n", rate);
			for (size_t j = 0; j < NUM_OF_ARRAY(func2Tbl); j++) {
				Test2(ret2[j], a, b, func2Tbl[j].f);
			}
		}

		puts("--- test1 ---");
		// print ret1
		// STL/jmp/cmov
		printf("name    rand  first    inc   inc2\n");
		for (size_t i = 0; i < NUM_OF_ARRAY(func1Tbl); i++) {
			printf("%s ", func1Tbl[i].name);
			for (size_t j = 0; j < ret1[i].size(); j++) {
				printf("%.3f  ", ret1[i][j]);
			}
			printf("\n");
		}
		{
			AlignedArray<int> x;
			x.resize(N);
			InitRandom(x, N);
			Xbyak::util::Clock clk;
			int (*f)(const int*, size_t) = func1Tbl[3].f;
			for (int i = 0; i < MaxCount; i++) {
				clk.begin();
				f(&x[0], N);
				clk.end();
			}
			printf("maxps for int data:%.3fclk\n", clk.getClock() / (double)(MaxCount * N));
		}
		{
			AlignedArray<float> x;
			x.resize(N);
			InitRandom(x, N);
			Xbyak::util::Clock clk;
			int (*f)(const int*, size_t) = func1Tbl[3].f;
			for (int i = 0; i < MaxCount; i++) {
				clk.begin();
				f((int*)&x[0], N);
				clk.end();
			}
			printf("maxps for valid float data:%.3fclk\n", clk.getClock() / (double)(MaxCount * N));
		}
		puts("--- test2 ---");
		printf("        0.00   0.25   0.50   0.75   1.00\n");
		// print ret2
		for (size_t i = 0; i < NUM_OF_ARRAY(func2Tbl); i++) {
			printf("%s ", func2Tbl[i].name);
			for (size_t j = 0; j < ret2[i].size(); j++) {
				printf("%6.3f ", ret2[i][j]);
			}
			printf("\n");
		}

	} catch (std::exception& e) {
		printf("ERR:%s\n", e.what());
	} catch (...) {
		printf("unknown error\n");
	}
}
