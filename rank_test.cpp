#include <stdio.h>
#include "rank.hpp"
#include <xbyak/xbyak_util.h>

#define TEST_EQUAL(a, b) { if ((a) != (b)) { fprintf(stderr, "%s:%d err lhs=%lld, rhs=%lld\n", __FILE__, __LINE__, (long long)(a), (long long)(b)); exit(1); } }

#include "rank_comp.hpp"

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
	const int lp = 5;
	for (int i = 0; i < lp; i++) {
		clk.begin();
		for (size_t i = 0; i < n; i++) {
			ret += r.get() & mask;
		}
		clk.end();
	}
	printf("(%08x)", ret);
	return clk.getClock() / double(n) / lp;
}
template<class T>
uint64_t bench(const uint64_t *block, size_t blockNum, size_t n, size_t mask, double baseClk)
{
	const T sbv(block, blockNum);
	uint64_t ret = 0;
	Xbyak::util::Clock clk;
	XorShift128 r;
	const int lp = 5;
	for (int j = 0; j < lp; j++) {
		clk.begin();
		for (size_t i = 0; i < n; i++) {
			ret += sbv.rank1(r.get() & mask);
		}
		clk.end();
	}
	printf("%11lld ret %08x %6.2f clk(%6.2f)\n", (long long)mask + 1, (int)ret, (double)clk.getClock() / double(n) / lp - baseClk, baseClk);
	return ret;
}

template<class T>
void test(const mie::BitVector& bv)
{
//	printf("----------------------\n");
//bv.put();
//	printf("bv.blockSize=%d, bitSize=%d\n", (int)bv.getBlockSize(), (int)bv.size());
	T s(bv.getBlock(), bv.getBlockSize());
	uint32_t num = 0;
	for (size_t i = 0; i < bv.size(); i++) {
		if (bv.get(i)) num++;
		uint32_t rank = s.rank1m(i);
		TEST_EQUAL(rank, num);
	}
}

template<class T>
void testSuccinctBitVector1()
{
	puts("testSuccinctBitVector1");
	{
		mie::BitVector bv;
		bv.resize(2048);
		test<T>(bv);
	}
	for (int i = 0; i < 2048; i++) {
		mie::BitVector bv;
		bv.resize(2048);
		bv.set(i, true);
		test<T>(bv);
	}
	puts("ok");
}

template<class T>
void testSuccinctBitVector2()
{
	puts("testSuccinctBitVector2");
	mie::BitVector bv;
	bv.resize(2048);
	for (int i = 0; i < 2048; i++) {
		bv.set(i, true);
		test<T>(bv);
	}
	puts("ok");
}
template<class T>
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
			uint64_t v = r.get();
			v = (v << 32) | r.get();
			bv.getBlock()[j] = v;
		}
		test<T>(bv);
	}
	puts("ok");
}

typedef std::vector<uint64_t> Vec64;

void initRand(Vec64& vec, size_t n)
{
	XorShift128 r;
	vec.resize(n);
	for (size_t i = 0; i < n; i++) {
		uint64_t v = r.get();
		v = (v << 32) | r.get();
		vec[i] = v;
	}
}

#ifdef COMPARE_MARISA
#include <marisa/grimoire/vector.h>
/*
	use marisa-0.2.0.tar.gz
	http://code.google.com/p/marisa-trie/
*/
struct MarisaVec {
	marisa::grimoire::BitVector bv;
	MarisaVec(const uint64_t *block, size_t blockNum)
	{
		for (size_t i = 0; i < blockNum; i++) {
			for (size_t j = 0; j < 64; j++) {
				bv.push_back(((block[i] >> j) & 1) != 0);
			}
		}
		bv.build(true, true);
	}
	/*
		now my version rank1 is a little different from
		starndard rank1, so use wrapper function.
		the penalty may be less than a few clk cycles.
	*/
	size_t rank1(size_t i) const
	{
		return bv.rank1(i + 1);
	}
};
#endif

#ifdef COMPARE_SUX
#include <rank9.h>
struct SucVec {
	rank9 bv;
	SucVec(const uint64_t *block, size_t blockNum)
		: bv(block, blockNum * sizeof(uint64_t) * 8)
	{
	}
	size_t rank1(size_t i) const
	{
		return const_cast<rank9&>(bv).rank(i + 1);
	}
};
#endif
#ifdef COMPARE_SDSL
#define and &&
#define or ||
#define not !
#include <sdsl/vectors.hpp>
#undef and
#undef or
#undef not
struct SdslVec {
	sdsl::bit_vector bv;
	sdsl::rank_support_v<> rs;
	SdslVec(const uint64_t *block, size_t blockNum)
		: bv(blockNum * sizeof(uint64_t) * 8)
	{
		for (size_t i = 0;i < blockNum; i++) {
			for (size_t j = 0; j < 64; j++) {
				bv[i * 64 + j] = ((block[i] >> j) & 1) !=0 ? 1 : 0;
			}
		}
		rs.init(&bv);
	}
	size_t rank1(size_t i) const
	{
		return rs.rank(i + 1);
	}
};
#endif

template<class T>
void benchAll()
{
	const size_t lp = 1000000;
	int ret = 0;
	for (size_t bitLen = 16; bitLen < 32; bitLen++) {
//	for (size_t bitLen = 16; bitLen < 29; bitLen++) {
		const size_t bitSize = size_t(1) << bitLen;
		Vec64 vec;
		initRand(vec, bitSize / (sizeof(uint64_t) * 8));
		double baseClk = getDummyLoopClock(lp, bitSize - 1);
		ret += bench<T>(&vec[0], vec.size(), lp, bitSize - 1, baseClk);
	}
	printf("ret=%x\n", ret);
}

template<class T>
void testAll()
{
	testSuccinctBitVector1<T>();
	testSuccinctBitVector2<T>();
	testSuccinctBitVector3<T>();
	benchAll<T>();
}

int main()
{
	testBitVector();
#if 0
	puts("NaiveSV2");
	testAll<mie::NaiveSV2>();
#else
	puts("SBV6");
	testAll<mie::SBV6>();
#endif
#ifdef COMPARE_MARISA
	puts("marisa");
	benchAll<MarisaVec>();
#endif
#ifdef COMPARE_SUX
	puts("sux");
	benchAll<SucVec>();
#endif
#ifdef COMPARE_SDSL
	puts("sdsl");
	benchAll<SdslVec>();
#endif
}

