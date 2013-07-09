#include <stdio.h>
#include <vector>
#include <numeric>
#include <functional>
#include <algorithm>
#include <random>
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>

typedef std::vector<int> IntVec;
typedef std::vector<int*> PtrVec;

void init(IntVec& iv, PtrVec& pv, size_t n)
{
	iv.resize(n);
	pv.resize(n);
	std::mt19937 rg(0);
	for (size_t i = 0; i < n; i++) {
		iv[i] = rg();
		pv[i] = &iv[i];
	}
}

void put(PtrVec& pv)
{
	for (const auto& x : pv) {
		printf("%d ", *x);
	}
	printf("\n");
}

template<class F>
void test(size_t n, bool doPut, F pred)
{
	IntVec iv;
	PtrVec pv;
	init(iv, pv, n);
	if (doPut) put(pv);
	Xbyak::util::Clock clk;
	clk.begin();
	std::sort(pv.begin(), pv.end(), pred);
	clk.end();
	int sum = std::accumulate(iv.begin(), iv.end(), 0);
	printf("clk=%.2fclk, sum=%d\n", clk.getClock() / double(n), sum);
	if (doPut) put(pv);
}

int main()
{
	test(10, true, [](const int *px, const int *py) { return *px < *py; });
	auto cmp1 = [](const int *px, const int *py) { return *px < *py; };
	test(1000000, false, cmp1);
	std::function<bool(const int*, const int*)> cmp2 = [](const int *px, const int *py) { return *px < *py; };
	test(1000000, false, cmp2);
}
