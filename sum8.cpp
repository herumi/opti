#include <stdio.h>
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>
#include "v128.h"
#include "util.hpp"

int sum1(const uint8_t data[8], int n)
{
	int sum = 0;
	for (int i = 0; i < n; i++) {
		sum += data[i];
	}
	return sum;
}

int sum2(const uint8_t data[8], int n)
{
	int sum = 0;
	switch (n) {
	case 7: sum += data[6];
	case 6: sum += data[5];
	case 5: sum += data[4];
	case 4: sum += data[3];
	case 3: sum += data[2];
	case 2: sum += data[1];
	case 1: sum += data[0];
	case 0:
		break;
	default:
#ifdef _WIN32
		__assume(0);
#else
		;
#endif
	}
	return sum;
}

int sum3(const uint8_t data[8], int n)
{
	V128 vmask;
	vmask = pcmpeqd(vmask, vmask); // all [-1]
	V128 shift((8 - n) * 8);
	vmask = psrlq(vmask, shift);
	V128 v = V128((uint32_t*)data);
	v = pand(v, vmask);
	v = psadbw(v, Zero());
	return movd(v);
}

int sum4_1(const uint8_t data[4], int n)
{
	int sum = 0;
	for (int i = 0; i < n; i++) {
		sum += data[i];
	}
	return sum;
}

int sum4_2(const uint8_t data[4], int n)
{
	int sum = 0;
	switch (n) {
	case 3: sum += data[2];
	case 2: sum += data[1];
	case 1: sum += data[0];
	}
	return sum;
}

int sum4_3(const uint8_t data[4], int n)
{
#if 1
	uint32_t x = *reinterpret_cast<const uint32_t*>(data);
	x &= (1U << (n * 8)) - 1;
	V128 v(x);
	v = psadbw(v, Zero());
	return movd(v);
#else
	V128 vmask;
	vmask = pcmpeqd(vmask, vmask); // all [-1]
	V128 shift((8 - n) * 8);
	vmask = psrlq(vmask, shift);
	V128 v = V128(*reinterpret_cast<const int*>(data));
	v = pand(v, vmask);
	v = psadbw(v, Zero());
	return movd(v);
#endif
}

struct Code : Xbyak::CodeGenerator {
	Code(char *buf, size_t size)
		try
		: Xbyak::CodeGenerator(size, buf)
	{
		Xbyak::CodeArray::protect(buf, size, true);
		using namespace Xbyak;
#ifdef _WIN32
		const Reg64& data = rcx;
		const Reg64& n = rdx;
#else
		const Reg64& data = rdi;
		const Reg64& n = rsi;
#endif
#if 1
		inLocalLabel();
		xor(rax, rax);
		test(n, n);
		jz(".exit");
	L(".lp");
		popcnt(r8, qword [data]);
		add(data, 8);
		add(rax, r8);
		sub(n, 1);
		jnz(".lp");
	L(".exit");
		outLocalLabel();
#else
		const Xmm& vmask = xm0;
		const Xmm& shift = xm1;
		const Xmm& v = xm2;
		neg(n);
		add(n, 8);
		shl(n, 3);
		movq(shift, n);
		pcmpeqd(vmask, vmask);
		psrlq(vmask, shift);
		movq(v, ptr [data]);
		pand(v, xm0);
		pxor(vmask, vmask);
		psadbw(v, vmask);
		movq(rax, v);
#endif
		ret();
	} catch (Xbyak::Error err) {
		printf("in ERR:%s(%d)\n", Xbyak::ConvertErrorToString(err), err);
		::exit(1);
	}
};

template<int dummy = 0>
struct InstanceIsHere {
	static MIE_ALIGN(4096) char buf[4096];
	static Code code;
};

template<int dummy>
Code InstanceIsHere<dummy>::code(buf, sizeof(buf));

template<int dummy>
char InstanceIsHere<dummy>::buf[4096];

struct DummyCall {
	DummyCall() { InstanceIsHere<>::code.getCode(); }
};

int (*sumA)(const uint8_t*, int) = (int (*)(const uint8_t*, int))(char*)InstanceIsHere<>::buf;

template<class F>
void bench(const uint8_t *data, F& f)
{
	XorShift128 rg;
	const int N = 100000;
	const int C = 1000;
	Xbyak::util::Clock clk;
	int ret = 0;
	for (int i = 0; i < N; i++) {
		clk.begin();
		for (int j = 0; j < C; j++) {
			ret += f(data, rg.get() % 8);
		}
		clk.end();
	}
	printf("ret=%x clk=%f\n", ret, clk.getClock() / double(N) / C);
}

template<class F>
void bench4(const uint8_t *data, F& f)
{
	XorShift128 rg;
	const int N = 100000;
	const int C = 1000;
	Xbyak::util::Clock clk;
	int ret = 0;
	for (int i = 0; i < N; i++) {
		clk.begin();
		for (int j = 0; j < C; j++) {
			ret += f(data, rg.get() % 4);
		}
		clk.end();
	}
	printf("ret=%x clk=%f\n", ret, clk.getClock() / double(N) / C);
}

int dummy(const uint8_t*, int)
{
	return 0;
}


int main()
	try
{
	MIE_ALIGN(16) uint8_t data[64] = {};
	for (int i = 1; i <= 8; i++) {
		data[i - i] = i * i;
	}
#if 0
	bench(data, dummy);
	puts("dummy");
	bench(data, dummy);
	puts("sum1");
	bench(data, sum1);
	puts("sum2");
	bench(data, sum2);
	puts("sum3");
	bench(data, sum3);
#endif
	bench4(data, dummy);
	puts("----------");
	puts("sum4_1");
	bench4(data, sum4_1);
	puts("sum4_2");
	bench4(data, sum4_2);
	puts("sum4_3");
	bench4(data, sum4_3);
//	puts("sumA");
//	bench(data, sumA);
} catch (Xbyak::Error err) {
	printf("ERR:%s(%d)\n", Xbyak::ConvertErrorToString(err), err);
	::exit(1);
}
