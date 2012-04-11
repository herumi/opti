/*
	25000446 24996089 24997085 25006380

	Pentium D 2.8GHz + VC2008
	0.390000
	c_old sum=-1494357181, 1.094000, 0.704000
	c_new sum=-1494357181, 0.937000, 0.547000
	a_old sum=-1494357181, 1.829000 1.439000
	a_new sum=-1494357181, 1.718000 1.328000

	Core Duo 1.8GHz + gcc 4.6.0
	0.650000
	c_old sum=-1494357181, 1.410000, 0.760000
	c_new sum=-1494357181, 1.270000, 0.620000
	a_old sum=-1494357181, 1.910000 1.260000
	a_new sum=-1494357181, 1.880000 1.230000

	Xeon X5650 2.67GHz + gcc 4.6.1
	0.240000
	c_old sum=-1494357181, 0.700000, 0.460000
	c_new sum=-1494357181, 0.620000, 0.380000
	a_old sum=-1494357181, 1.000000 0.760000
	a_new sum=-1494357181, 0.960000 0.720000
	Core i7-2600K 3.4GHz + VC2011
	0.202000
	c_old sum=-1494357181, 0.500000, 0.298000
	c_new sum=-1494357181, 0.390000, 0.188000
	a_old sum=-1494357181, 0.873000 0.671000
	a_new sum=-1494357181, 0.765000 0.563000
	K6-III + High-C
	7.07
	c_old sum=-1494357181, 11.880000, 4.81000
	c_new sum=-1494357181, 11.930000, 4.86000
	a_old sum=-1494357181, 12.590000 5.52000
	a_new sum=-1494357181, 11.760000 4.69000
*/
#include <stdio.h>
#include <time.h>

unsigned int x_, y_, z_, w_;
void init()
{
	x_ = 123456789;
	y_ = 362436069;
	z_ = 521288629;
	w_ = 88675123;
}
unsigned int get()
{
	unsigned int t = x_ ^ (x_ << 11);
	x_ = y_; y_ = z_; z_ = w_;
	return w_ = (w_ ^ (w_ >> 19)) ^ (t ^ (t >> 8));
}

int c_old(unsigned int k)
{
	if (k < 2) return 3;
	if (k == 2) return 6;
	return 100;
}

int c_new(unsigned int k)
{
	if (k == 2) return 6;
	if (k == 3) return 100;
	return 3;
}

extern int a_old(unsigned int k);
extern int a_new(unsigned int k);

int main()
{
	const int N = 100000000;
	int begin, end;
	int i, sum;
	int count[4] = { 0, 0, 0, 0 };
	double base = 0, t;

	init();
	begin = clock();
	for (i = 0; i < N; i++) {
		count[get() & 3]++;
	}
	end = clock();
	base = (end - begin) / (double)CLOCKS_PER_SEC;
	printf("%d %d %d %d\n%f\n", count[0], count[1], count[2], count[3], base);

	init();
	sum = 0;
	begin = clock();
	for (i = 0; i < N; i++) {
		unsigned int x = get() & 3;
		sum += c_old(x);
	}
	end = clock();
	t = (end - begin) / (double)CLOCKS_PER_SEC;
	printf("c_old sum=%d, %f, %f\n", sum, t, t - base);

	init();
	sum = 0;
	begin = clock();
	for (i = 0; i < N; i++) {
		unsigned int x = get() & 3;
		sum += c_new(x);
	}
	end = clock();
	t = (end - begin) / (double)CLOCKS_PER_SEC;
	printf("c_new sum=%d, %f, %f\n", sum, t, t - base);

	init();
	sum = 0;
	begin = clock();
	for (i = 0; i < N; i++) {
		unsigned int x = get() & 3;
		sum += a_old(x);
	}
	end = clock();
	t = (end - begin) / (double)CLOCKS_PER_SEC;
	printf("a_old sum=%d, %f %f\n", sum, t, t - base);

	init();
	sum = 0;
	begin = clock();
	for (i = 0; i < N; i++) {
		unsigned int x = get() & 3;
		sum += a_new(x);
	}
	end = clock();
	t = (end - begin) / (double)CLOCKS_PER_SEC;
	printf("a_new sum=%d, %f %f\n", sum, t, t - base);
}
