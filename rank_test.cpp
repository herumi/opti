#include <stdio.h>
#include "rank.hpp"

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

void test(const mie::BitVector& bv)
{
//	printf("----------------------\n");
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
		bv.resize(256);
		test(bv);
	}
	for (int i = 0; i < 256; i++) {
		mie::BitVector bv;
		bv.resize(256);
		bv.set(i, true);
		test(bv);
	}
	puts("ok");
}

void testSuccinctBitVector2()
{
	puts("testSuccinctBitVector2");
	mie::BitVector bv;
	bv.resize(256);
	for (int i = 0; i < 256; i++) {
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

int main()
{
	mie::BitVector bv;
	testBitVector();
	testSuccinctBitVector1();
	testSuccinctBitVector2();
	testSuccinctBitVector3();
}
