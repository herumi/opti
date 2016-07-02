/*

sde -- t.exe
2.0 4.0 3.0 4.0
2.0 4.0 0.0 0.0
2.0 4.0 3.0 4.0 5.0 6.0 7.0 8.0
2.0 4.0 0.0 0.0 0.0 0.0 0.0 0.0
2.0 4.0 6.0 8.0 0.0 0.0 0.0 0.0

*/
#include <stdio.h>

extern "C" {

double dout[];

void f_addpd();
void f_vaddpd();
void f_addpd512();
void f_vaddpd512();
void f_vaddpd512y();

}

void put(const double *p, int n)
{
	for (int i = 0; i < n; i++) {
		printf("%.1f ", p[i]);
	}
	printf("\n");
}

int main()
{
	f_addpd();
	put(dout, 4);
	f_vaddpd();
	put(dout, 4);

	f_addpd512();
	put(dout, 8);
	f_vaddpd512();
	put(dout, 8);
	f_vaddpd512y();
	put(dout, 8);
}
