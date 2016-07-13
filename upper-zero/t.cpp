/*

sde -- t.exe
f_movsd
8.0 7.0 6.0 5.0 4.0 3.0 2.0 1.0
f_movsd_mem
8.0 7.0 6.0 5.0 4.0 3.0 0.0 1.0
f_vmovsd
0.0 0.0 0.0 0.0 0.0 0.0 2.0 1.0
f_vmovsd_mem
0.0 0.0 0.0 0.0 0.0 0.0 0.0 1.0
f_addsd
8.0 7.0 6.0 5.0 4.0 3.0 2.0 2.0
f_addpd
8.0 7.0 6.0 5.0 4.0 3.0 4.0 2.0
f_vaddsd
0.0 0.0 0.0 0.0 0.0 0.0 2.0 2.0
f_vaddpd
0.0 0.0 0.0 0.0 0.0 0.0 4.0 2.0
f_vaddpd_y
0.0 0.0 0.0 0.0 8.0 6.0 4.0 2.0

*/
#include <stdio.h>

extern "C" {

double dout[];

void f_movsd();
void f_movsd_mem();
void f_addsd();
void f_addpd();
void f_movlpd();

void f_vmovsd();
void f_vmovsd_mem();
void f_vmovlpd();
void f_vaddsd();
void f_vaddpd();
void f_vaddpd_y();
void f_vaddpd_k();

}

void put(const double *p, int n)
{
	for (int i = 0; i < n; i++) {
		printf("%.1f ", p[n - 1 - i]);
	}
	printf("\n");
}

void call(const char *msg, void f())
{
	puts(msg);
	f();
	put(dout, 8);
}

#define CALL(f) call(#f, f);

int main()
{
	CALL(f_movsd);
	CALL(f_movsd_mem);
	CALL(f_vmovsd);
	CALL(f_vmovsd_mem);
	CALL(f_movlpd);
	CALL(f_vmovlpd);
	CALL(f_addsd);
	CALL(f_addpd);
	CALL(f_vaddsd);
	CALL(f_vaddpd);
	CALL(f_vaddpd_y);
}
