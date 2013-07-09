/*

VC11 x64 Core i7-2600K
mode:  none num=     240  4813750  5186010        0 clk= 61.699Mclk
mode:mfence num=       0        0 10000000        0 clk=431.247Mclk
mode:  lock num=       0        0 10000000        0 clk=221.165Mclk

VC11 x32 Core i7-2600K
mode:  none num=    8624  4839049  5152327        0 clk= 62.147Mclk
mode:mfence num=       0  3882827  6115205     1968 clk=654.149Mclk
mode:  lock num=       0        0 10000000        0 clk=272.579Mclk

gcc-4.6.1 x64 Xeon X5650
mode:  none num=    8624  4839049  5152327        0 clk= 62.147Mclk
mode:mfence num=       0  3882827  6115205     1968 clk=654.149Mclk
mode:  lock num=       0        0 10000000        0 clk=272.579Mclk

gcc-4.6.1 x32 Xeon X5650
mode:  none num=  730909  5174775  4093547      769 clk=123.606Mclk
mode:mfence num=       0       38  9999956        6 clk=618.896Mclk
mode:  lock num=       0        0 10000000        0 clk=405.311Mclk

gcc-4.2.1 x64 Core2Duo
mode:  none num= 1707671  3697608  4592424     2297 clk=161.112Mclk
mode:mfence num=       0  9712120   287879        1 clk=312.287Mclk
mode:  lock num=       0  9871106   128894        0 clk=419.611Mclk

VC2010 x32 Core2Duo
mode:  none num=       0     9828  9990172        0 clk=304.327Mclk
mode:mfence num=       0   678716  9321284        0 clk=266.541Mclk
mode:  lock num=       0  1896942  8103056        2 clk=439.686Mclk

	g++ -O3 -fno-operator-names -fomit-frame-pointer fence.cpp -lpthread -std=c++0x
*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#if __cplusplus >= 201103L
	#define USE_STD_THREAD
#endif
#ifdef USE_STD_THREAD
#include <thread>
#else
#include <cybozu/thread.hpp>
#endif
#include <memory.h>
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

const int N = 10000000;
struct Data {
	int sa;
	int sb;
	int r1;
	int r2;
} data[N];

void (*write1[3])();
void (*write2[3])();

struct Code : Xbyak::CodeGenerator {
	enum {
		sa_off = offsetof(Data, sa),
		sb_off = offsetof(Data, sb),
		r1_off = offsetof(Data, r1),
		r2_off = offsetof(Data, r2)
	};
	/*
		remark : use ebx if mode == 2 and 32bit
	*/
	void mov_1_lock(const Xbyak::Address& addr, int mode)
	{
		switch (mode) {
		case 0:
			mov(addr, 1);
			break;
		case 1:
			mov(addr, 1);
			mfence();
			break;
		case 2:
#ifdef XBYAK64
			mov(addr, 1);
			db(0xf0); // lock
			or_(dword [rsp], 0);
#else
			mov(ebx, 1);
			xchg(addr, ebx);
#endif
			break;
		default:
			printf("ERR mode=%d\n", mode);
			exit(1);
		}
	}
	Code()
	{
		for (int i = 0; i < 3; i++) {
			align(16);
			write1[i] = (void (*)())getCurr();
			gen_write(i, 0);
		}
		for (int i = 0; i < 3; i++) {
			align(16);
			write2[i] = (void (*)())getCurr();
			gen_write(i, 1);
		}
	}
	/*
		for (int i = 0; i < N; i++) {
			if (dir == 0) {
				mov([A], 1);
				mov(r1, [B]);
			} else {
				mov([B], 1);
				mov(r2, [A]);
			}
		}
	*/
	void gen_write(int mode, int dir)
	{
		using namespace Xbyak;
		inLocalLabel();
#ifdef XBYAK64
		const Reg64& a = rax;
		const Reg64& c = rcx;
//		const Reg64& d = rdx;
#else
		const Reg32& a = eax;
		const Reg32& c = ecx;
//		const Reg32& d = edx;
		push(ebx);
		sub(esp, 4);
#endif
		mov(a, (size_t)data);
		mov(c, N);
	L(".lp");
		if (dir == 0) {
			// mov(dword [a + sa_off], 1);
			// mfence();
			mov_1_lock(dword [a + sa_off], mode);
			mov(edx, ptr [a + sb_off]);
			mov(ptr [a + r1_off], edx);
		} else {
			mov_1_lock(dword [a + sb_off], mode);
			mov(edx, ptr [a + sa_off]);
			mov(ptr [a + r2_off], edx);
		}
		add(a, sizeof(Data));
		sub(c, 1);
		jnz(".lp");
#ifdef XBYAK32
		add(esp, 4);
		pop(ebx);
#endif
		ret();
		outLocalLabel();
	}
};

const char *mode2str(int mode)
{
	switch (mode) {
	case 0:
		return "none";
	case 1:
		return "mfence";
	case 2:
		return "lock";
	default:
		printf("ERR mode=%d\n", mode);
		exit(1);
	}
}

#ifndef USE_STD_THREAD
namespace std {
struct thread : cybozu::ThreadBase {
	void (*f_)();
	thread(void (*f)())
		: f_(f)
	{
		beginThread();
	}
	void threadEntry()
	{
		f_();
	}
	void join()
	{
		joinThread();
	}
};
} // std
#endif

void test(int mode)
{
	printf("mode:%6s ", mode2str(mode));
	memset(data, 0, sizeof(data));
	double time = 0;
	{
		Xbyak::util::Clock clk;
		clk.begin();
		std::thread t1(write1[mode]);
		std::thread t2(write2[mode]);
		t1.join();
		t2.join();
		clk.end();
		time = (double)clk.getClock();
	}

	int num[4] = { };
	for (int i = 0; i < N; i++) {
		const Data& d = data[i];
		if (d.sa == 0 || d.sb == 0) {
			printf("ERR %d %d\n", d.sa, d.sb);
			exit(1);
		}
		if (d.r1 == 0 && d.r2 == 0) {
			num[0]++;
		} else
		if (d.r1 == 1 && d.r2 == 0) {
			num[1]++;
		} else
		if (d.r1 == 0 && d.r2 == 1) {
			num[2]++;
		} else
		{
			num[3]++;
		}
	}
	int sum = 0;
	printf("num=");
	for (int i = 0; i < 4; i++) {
		printf("%8d ", num[i]);
		sum += num[i];
	}
	if (sum != N) {
		fprintf(stderr, "ERR sum=%d\n", sum);
		exit(1);
	}
	printf("clk=%7.3fMclk\n", time * 1e-6);
}

int main(int argc, char *argv[])
{
	Code c;
	argc--, argv++;
	if (argc == 1) {
		int mode = atoi(argv[0]);
		test(mode);
		return 0;
	}

	for (int mode = 0; mode < 3; mode++) {
		test(mode);
	}
}

