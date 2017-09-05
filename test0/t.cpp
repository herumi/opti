#include <stdio.h>
#include <cybozu/benchmark.hpp>

extern "C" int func1(int);
extern "C" int func2(int);
extern "C" int func3(int);
extern "C" int func4(int);

int (*func5)(int);

int main()
{
	const int N = 10000000;
	func5 = func1;
	printf("%d\n", func1(5));
	printf("%d\n", func2(5));
	printf("%d\n", func3(5));
	printf("%d\n", func4(5));
	printf("%d\n", func5(5));
	CYBOZU_BENCH_C("func1", N, func1, 10);
	CYBOZU_BENCH_C("func2", N, func2, 10);
	CYBOZU_BENCH_C("func3", N, func3, 10);
	CYBOZU_BENCH_C("func4", N, func4, 10);
	CYBOZU_BENCH_C("func5", N, func5, 10);
}
