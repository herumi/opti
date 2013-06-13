#include <stdio.h>
#ifdef _MSC_VER
	#include <intrin.h>
#else
	#include <x86intrin.h>
#endif
#include <math.h>

double near_rsqrt(double x)
{
	float f = (float)x;
	_mm_store_ss(&f, _mm_rsqrt_ss(_mm_load_ss(&f)));
	return f;
}

double near(double x, double x0)
{
	return -0.5 * (x * x0 * x0 - 3) * x0;
}

int main()
{
	for (int i = 1; i < 10; i++) {
		double x = double(i);
		double r = 1 / sqrt(x);
		double x0 = near_rsqrt(x); // 11-bit accuracy
		double x1 = near(x, x0); // 22-bit accuracy
		double x2 = near(x, x1); // 44-bit accuracy
		printf("1/sqrt(%d) = %e, %e, %e, %e\n", i, r, fabs(r - x0), fabs(r - x1), fabs(r - x2));
	}
}
