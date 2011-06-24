/*
	benchmark of sequential code and random jmp code
	require Xbyak : http://homepage1.nifty.com/herumi/soft/xbyak_e.html
	% g++ -O3 -fno-operator-names icache.cpp && ./a.out
[sequential]
 count  clk   code size(Kbyte)
     1 : 0.922,    0.020
     2 : 0.905,    0.023
     4 : 0.910,    0.029
     8 : 0.911,    0.041
    16 : 0.922,    0.065
    32 : 0.930,    0.113
    64 : 0.923,    0.209
   128 : 0.936,    0.401
   256 : 0.909,    0.785
   512 : 0.908,    1.553
  1024 : 0.906,    3.089
  2048 : 0.923,    6.161
  4096 : 0.898,   12.305
  8192 : 0.907,   24.593
 16384 : 0.910,   49.169
 32768 : 0.920,   98.321
 65536 : 0.917,  196.625
131072 : 0.935,  393.233
[random]
 count  clk   code size(Kbyte)
     1 : 1.819,    0.025
     2 : 1.346,    0.033
     4 : 2.514,    0.049
     8 : 2.275,    0.081
    16 : 2.153,    0.145
    32 : 2.104,    0.273
    64 : 2.101,    0.529
   128 : 2.099,    1.041
   256 : 2.060,    2.065
   512 : 2.059,    4.113
  1024 : 2.085,    8.209
  2048 : 2.029,   16.401
  4096 : 9.138,   32.785
  8192 :12.138,   65.553
 16384 :12.039,  131.089
 32768 :12.092,  262.161
 65536 :12.121,  524.305
131072 :12.146, 1048.593
*/
#include <stdio.h>
#include <stdlib.h>
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

const int N = 0x1000000;

struct Code : Xbyak::CodeGenerator {
	explicit Code(int count, int mode)
		: CodeGenerator(1024 * 1024 * 64)
	{
		using namespace Xbyak;
		mov(ecx, N / count);
		xor(eax, eax);
	L(".lp");
		for (int i = 0; i < count; i++) {
			if (mode == 0) {
				add(eax, 1);
			} else {
				L(Label::toStr(i).c_str());
				add(eax, 1);
				int to = 0;
				if (i < count / 2) {
					to = count - 1 - i;
				} else {
					to = count  - i;
				}
				if (i == count / 2) {
					jmp(".exit", T_NEAR);
				} else {
					jmp(Label::toStr(to).c_str(), T_NEAR);
				}
			}
		}
	L(".exit");
		sub(ecx, 1);
		jnz(".lp", T_NEAR);
		ret();
	}
};

void test(int count, int mode)
{
	printf("%6d :", count);
	Code code(count, mode);
	Xbyak::util::Clock clk;
	const int LP = 4;
	int sum = 0;
	for (int i = 0; i < LP; i++) {
		clk.begin();
		sum += ((int (*)())code.getCode())();
		clk.end();
	}
	if (sum != LP * N) printf("err sum=%d, %d\n", sum, LP * N);
	printf("%6.3f, %8.3f\n", clk.getClock() / (double)LP / N, code.getSize() / 1e3);
}


int main(int argc, char *argv[])
{
	argc--, argv++;
	const int sel = argc > 0 ? atoi(*argv) : -1;

	for (int mode = 0; mode < 2; mode++) {
		if (sel != -1 && sel != mode) continue;
		printf("[%s]\n", mode == 0 ? "sequential" : "random");
		printf(" count  clk   code size(Kbyte)\n");
		int count = 1;
		for (int i = 0; i < 18; i++) {
			test(count, mode);
			count *= 2;
		}
	}
}

