#include <stdio.h>
#include "rank.hpp"
#include <xbyak/xbyak_util.h>

#define TEST_EQUAL(a, b) { if ((a) != (b)) { fprintf(stderr, "%s:%d err lhs=%lld, rhs=%lld\n", __FILE__, __LINE__, (long long)(a), (long long)(b)); exit(1); } }

void testBitVector()
{
	puts("testBitVector");
	mie::BitVector v;
	const int bitSize = 100;
	v.resize(bitSize);

	// init : all zero
	for (int i = 0; i < bitSize; i++) {
		TEST_EQUAL(v.get(i), false);
	}
	// set all true
	for (int i = 0; i < bitSize; i++) {
		v.set(i, true);
	}
	for (int i = 0; i < bitSize; i++) {
		TEST_EQUAL(v.get(i), true);
	}
	// set all false
	for (int i = 0; i < bitSize; i++) {
		v.set(i, false);
	}
	for (int i = 0; i < bitSize; i++) {
		TEST_EQUAL(v.get(i), false);
	}
	// set even true
	for (int i = 0; i < bitSize; i += 2) {
		v.set(i, true);
	}
	for (int i = 0; i < bitSize; i += 2) {
		TEST_EQUAL(v.get(i), true);
		TEST_EQUAL(v.get(i + 1), false);
	}
	puts("ok");
}

double getDummyLoopClock(size_t n, size_t mask)
{
	int ret = 0;
	Xbyak::util::Clock clk;
	XorShift128 r;
	clk.begin();
	for (size_t i = 0; i < n; i++) {
		ret += r.get() & mask;
	}
	clk.end();
	printf("(%08x)", ret);
	return clk.getClock() / double(n);
}
template<class T>
void bench(const uint64_t *block, size_t blockNum, size_t n, size_t mask, double baseClk)
{
	const T sbv(block, blockNum);
	int ret = 0;
	Xbyak::util::Clock clk;
	XorShift128 r;
	clk.begin();
	for (size_t i = 0; i < n; i++) {
		ret += sbv.rank1(r.get() & mask);
	}
	clk.end();
	printf("%8d ret=%08x %fclk(%f)\n", (int)mask + 1, ret, (double)clk.getClock() / double(n) - baseClk, baseClk);
}

void test(const mie::BitVector& bv)
{
//	printf("----------------------\n");
//bv.put();
//	printf("bv.blockSize=%d, bitSize=%d\n", (int)bv.getBlockSize(), (int)bv.size());
	mie::SuccinctBitVector s(bv.getBlock(), bv.getBlockSize());
	uint32_t num = 0;
	for (size_t i = 0; i < bv.size(); i++) {
		if (bv.get(i)) num++;
		uint32_t rank = s.rank1(i);
		TEST_EQUAL(rank, num);
	}
}

void testSuccinctBitVector1()
{
	puts("testSuccinctBitVector1");
	{
		mie::BitVector bv;
		bv.resize(2048);
		test(bv);
	}
	for (int i = 0; i < 2048; i++) {
		mie::BitVector bv;
		bv.resize(2048);
		bv.set(i, true);
		test(bv);
	}
	puts("ok");
}

void testSuccinctBitVector2()
{
	puts("testSuccinctBitVector2");
	mie::BitVector bv;
	bv.resize(2048);
	for (int i = 0; i < 2048; i++) {
		bv.set(i, true);
		test(bv);
	}
	puts("ok");
}
void testSuccinctBitVector3()
{
	puts("testSuccinctBitVector3");
	mie::BitVector bv;
	const int blockNum = 16;
	const int bitSize = blockNum * 64;
	bv.resize(bitSize);
	for (int i = 0; i < 10000; i++) {
		XorShift128 r;
		for (int j = 0; j < blockNum; j++) {
			uint64_t x = r.get();
			x = (x << 32) | r.get();
			bv.getBlock()[j] = x;
		}
		test(bv);
	}
	puts("ok");
}

typedef std::vector<uint64_t> Vec;

void initRand(Vec& vec, size_t n)
{
	XorShift128 r;
	vec.resize(n);
	for (size_t i = 0; i < n; i++) {
		vec[i] = r.get();
	}
}

int main()
{
	mie::BitVector bv;
	testBitVector();
	testSuccinctBitVector1();
	testSuccinctBitVector2();
	testSuccinctBitVector3();

	const size_t lp = 5000000;
	for (size_t bitSize = 10; bitSize < 23; bitSize++) {
		const size_t n = 1U << bitSize;
		Vec vec;
		initRand(vec, n / sizeof(uint64_t));
		double baseClk = getDummyLoopClock(lp, n - 1);
		bench<mie::SuccinctBitVector>(&vec[0], vec.size(), lp, n - 1, baseClk);
	}
}

