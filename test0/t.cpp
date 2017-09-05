#include <stdio.h>
#include <stdint.h>
#include <cybozu/benchmark.hpp>

extern "C" int func1(void *, size_t c);
extern "C" int func2(void *, size_t c);
extern "C" int func3(void *, size_t c);
extern "C" int func4(void *, size_t c);

int main()
{
	const int N = 1000;//00;
	uint64_t x = 0;
	CYBOZU_BENCH_C("func1", N, func1, &x, N);
	CYBOZU_BENCH_C("func2", N, func2, &x, N);
	CYBOZU_BENCH_C("func3", N, func3, &x, N);
	CYBOZU_BENCH_C("func4", N, func4, &x, N);
}
