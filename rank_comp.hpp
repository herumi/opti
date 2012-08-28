#pragma once

#include "rank.hpp"

namespace mie {

/*
	mem = (32 + 8 * 4) / 256
*/
struct NaiveSV {
	const uint64_t *org_;
	std::vector<uint32_t> a_;
	std::vector<uint8_t> b_;
	uint64_t popCount64(uint64_t x) const
	{
		return _mm_popcnt_u64(x);
	}
public:
	NaiveSV()
	{
	}
	NaiveSV(const uint64_t *blk, size_t blkNum)
	{
		init(blk, blkNum);
	}

	void init(const uint64_t *blk, size_t blkNum)
	{
		org_ = blk;
		size_t tblNum = (blkNum + 3) / 4;
		a_.resize(tblNum);
		b_.resize(tblNum * 4);

		uint32_t av = 0;
		size_t pos = 0;
		size_t aPos = 0;
		size_t bPos = 0;
		for (size_t i = 0; i < tblNum; i++) {
			a_[aPos++] = av;
			uint32_t bv = 0;
			for (size_t j = 0; j < 4; j++) {
				uint64_t v = pos < blkNum ? blk[pos++] : 0;
				int c = popCount64(v);
				av += c;
				b_[bPos++] = bv;
				bv += c;
			}
		}
	}
	uint32_t rank1(size_t idx) const
	{
		return rank1m(idx);
	}
	uint32_t rank1m(uint32_t i) const
	{
		return a_[i / 256] + b_[i / 64] + popCount64(org_[i / 64] & ((2ULL << (i & 63)) - 1));
	}
};

struct NaiveSV1 {
	const uint64_t *org_;
	struct B {
		uint32_t a;
		uint8_t b[4];
	};
	std::vector<B> blk_;
	uint64_t popCount64(uint64_t x) const
	{
		return _mm_popcnt_u64(x);
	}
public:
	NaiveSV1()
	{
	}
	NaiveSV1(const uint64_t *blk, size_t blkNum)
	{
		init(blk, blkNum);
	}

	void init(const uint64_t *blk, size_t blkNum)
	{
		org_ = blk;
		size_t tblNum = (blkNum + 3) / 4;
		blk_.resize(tblNum);

		uint32_t av = 0;
		size_t pos = 0;
		for (size_t i = 0; i < tblNum; i++) {
			B& b = blk_[i];
			b.a = av;
			uint32_t bv = 0;
			for (size_t j = 0; j < 4; j++) {
				uint64_t v = pos < blkNum ? blk[pos++] : 0;
				int c = popCount64(v);
				av += c;
				b.b[j] = bv;
				bv += c;
			}
		}
	}
	uint32_t rank1(size_t idx) const
	{
		return rank1m(idx);
	}
	uint32_t rank1m(uint32_t i) const
	{
		size_t q = i / 256;
		size_t r = (i / 64) & 3;
		return blk_[q].a + blk_[q].b[r] + popCount64(org_[i / 64] & ((2ULL << (i & 63)) - 1));
	}
};

struct NaiveSV2 {
	struct B {
		uint64_t org[4];
		uint32_t a;
		uint8_t b[4];
	};
	std::vector<B> blk_;
	uint64_t popCount64(uint64_t x) const
	{
		return _mm_popcnt_u64(x);
	}
public:
	NaiveSV2()
	{
	}
	NaiveSV2(const uint64_t *blk, size_t blkNum)
	{
		init(blk, blkNum);
	}

	void init(const uint64_t *blk, size_t blkNum)
	{
		size_t tblNum = (blkNum + 3) / 4;
		blk_.resize(tblNum);

		uint32_t av = 0;
		size_t pos = 0;
		for (size_t i = 0; i < tblNum; i++) {
			B& b = blk_[i];
			b.a = av;
			uint32_t bv = 0;
			for (size_t j = 0; j < 4; j++) {
				uint64_t v = pos < blkNum ? blk[pos++] : 0;
				b.org[j] = v;
				int c = popCount64(v);
				av += c;
				b.b[j] = bv;
				bv += c;
			}
		}
	}
	uint32_t rank1(size_t idx) const
	{
		return rank1m(idx);
	}
	uint32_t rank1m(uint32_t i) const
	{
		size_t q = i / 256;
		size_t r = (i / 64) & 3;
		const B& b = blk_[q];
		return b.a + b.b[r] + popCount64(b.org[r] & ((2ULL << (i & 63)) - 1));
	}
};

class SBV1 {
	struct B2 {
		uint64_t data[8];
		uint32_t rank;
		uint32_t carry;
		uint8_t s8[8];
	};
	const uint64_t *org_;
	AlignedArray<B2> blk_;
	SBV1(const SBV1&);
	void operator=(const SBV1&);
public:
	SBV1()
	{
	}
	SBV1(const uint64_t *blk, size_t blkNum)
	{
		init(blk, blkNum);
	}
	uint64_t rank1(size_t idx) const
	{
		return rank1m(idx);
	}
	uint64_t rank1m(size_t idx) const
	{
		const uint64_t mask = (uint64_t(2) << (idx & 63)) - 1;
		const B2& blk = blk_[idx / 512];
		uint64_t q = (idx / 64) % 8;
		uint64_t b0 = blk.data[q];
//		uint64_t b0 = org_[idx / 64];
		uint64_t m0 = b0 & mask;
		uint64_t ret = popCount64(b0 & m0);
		ret += blk.s8[q];
		if ((blk.carry >> q) & 1) ret += 256;
		ret += blk.rank;
		return ret;
	}
	uint64_t popCount64(uint64_t x) const
	{
		return _mm_popcnt_u64(x);
	}
	void init(const uint64_t *blk, size_t blkNum)
	{
		org_ = blk;
		size_t tblNum = (blkNum + 7) / 8;
		blk_.resize(tblNum);

		uint32_t r = 0;
		size_t pos = 0;
		for (size_t i = 0; i < tblNum; i++) {
			B2& b = blk_[i];
			b.rank = r;
			uint16_t sum = 0;
			b.carry = 0;
			for (size_t j = 0; j < 8; j++) {
				uint64_t v = pos < blkNum ? blk[pos++] : 0;
				b.data[j] = v;
				int s8 = popCount64(v);
				r += s8;
				b.s8[j] = (uint8_t)sum;
				if (sum >= 256) b.carry |= 1U << j;
				sum += s8;
			}
		}
	}
};

class SBV2 {
	struct B2 {
		uint32_t rank;
		uint32_t carry;
		uint8_t s8[8];
	};
	const uint64_t *org_;
	AlignedArray<B2> blk_;
	SBV2(const SBV2&);
	void operator=(const SBV2&);
public:
	SBV2()
	{
	}
	SBV2(const uint64_t *blk, size_t blkNum)
	{
		init(blk, blkNum);
	}
	uint64_t rank1(size_t idx) const
	{
		return rank1m(idx);
	}
	uint64_t rank1m(size_t idx) const
	{
		const uint64_t mask = (uint64_t(2) << (idx & 63)) - 1;
		const B2& blk = blk_[idx / 512];
		uint64_t q = (idx / 64) % 8;
		uint64_t b0 = org_[idx / 64];
		uint64_t m0 = b0 & mask;
		uint64_t ret = popCount64(b0 & m0);
		ret += blk.s8[q];
		if ((blk.carry >> q) & 1) ret += 256;
		ret += blk.rank;
		return ret;
	}
	uint64_t popCount64(uint64_t x) const
	{
		return _mm_popcnt_u64(x);
	}
	void init(const uint64_t *blk, size_t blkNum)
	{
		org_ = blk;
		size_t tblNum = (blkNum + 7) / 8;
		blk_.resize(tblNum);

		uint32_t r = 0;
		size_t pos = 0;
		for (size_t i = 0; i < tblNum; i++) {
			B2& b = blk_[i];
			b.rank = r;
			uint16_t sum = 0;
			b.carry = 0;
			for (size_t j = 0; j < 8; j++) {
				uint64_t v = pos < blkNum ? blk[pos++] : 0;
				int s8 = popCount64(v);
				r += s8;
				b.s8[j] = (uint8_t)sum;
				if (sum >= 256) b.carry |= 1U << j;
				sum += s8;
			}
		}
	}
};

class SBV3 {
	struct B2 {
		uint64_t data[16];
		uint8_t s8[8];
		uint32_t rank;
		uint32_t carry;
	};
	const uint64_t *org_;
	AlignedArray<B2> blk_;
	SBV3(const SBV3&);
	void operator=(const SBV3&);
public:
	SBV3()
	{
	}
	SBV3(const uint64_t *blk, size_t blkNum)
	{
		init(blk, blkNum);
	}
	uint64_t rank1(size_t idx) const
	{
		return rank1m(idx);
	}
	uint64_t rank1m(size_t idx) const
	{
		const uint64_t mask = (uint64_t(2) << (idx & 63)) - 1;
		const B2& blk = blk_[idx / 1024];
		uint64_t q = (idx / 128) % 8;
		uint64_t b0 = blk.data[q * 2 + 0];
		uint64_t b1 = blk.data[q * 2 + 1];
		uint64_t m0 = -1;
		uint64_t m1 = 0;
		if (!(idx & 64)) m0 = mask;
		if (idx & 64) m1 = mask;
		uint64_t ret = popCount64(b0 & m0);
		ret += popCount64(b1 & m1);
		ret += blk.s8[q];
		ret += ((blk.carry >> (q * 2 + 8)) & (3 << 8));
		ret += blk.rank;
		return ret;
	}
	uint64_t popCount64(uint64_t x) const
	{
		return _mm_popcnt_u64(x);
	}
	void init(const uint64_t *blk, size_t blkNum)
	{
		org_ = blk;
		size_t tblNum = (blkNum + 15) / 16;
		blk_.resize(tblNum);

		uint32_t r = 0;
		size_t pos = 0;
		for (size_t i = 0; i < tblNum; i++) {
			B2& b = blk_[i];
			b.rank = r;
			uint16_t sum = 0;
			b.carry = 0;
			for (size_t j = 0; j < 8; j++) {
				uint64_t vL = pos < blkNum ? blk[pos++] : 0;
				uint64_t vH = pos < blkNum ? blk[pos++] : 0;
				b.data[j * 2 + 0] = vL;
				b.data[j * 2 + 1] = vH;
				int s8 = popCount64(vL) + popCount64(vH);
				r += s8;
				b.s8[j] = (uint8_t)sum;
				b.carry |= (sum >> 8) << (j * 2 + 16);
				sum += s8;
			}
		}
	}
};

class SBV4 {
	struct B {
		uint64_t rank;
		uint64_t data[8];
	};
	AlignedArray<B> blk_;
	SBV4(const SBV4&);
	void operator=(const SBV4&);
public:
	SBV4()
	{
	}
	SBV4(const uint64_t *blk, size_t blkNum)
	{
		init(blk, blkNum);
	}
	uint64_t rank1(size_t idx) const
	{
		return rank1m(idx);
	}
	uint64_t rank1m(size_t idx) const
	{
		const uint64_t mask = (uint64_t(2) << (idx & 63)) - 1;
		const B& blk = blk_[idx / 512];
		uint64_t q = (idx / 64) % 8;
		uint64_t ret = blk.rank;
		switch (q) {
		case 7: ret += popCount64(blk.data[6]);
		case 6: ret += popCount64(blk.data[5]);
		case 5: ret += popCount64(blk.data[4]);
		case 4: ret += popCount64(blk.data[3]);
		case 3: ret += popCount64(blk.data[2]);
		case 2: ret += popCount64(blk.data[1]);
		case 1: ret += popCount64(blk.data[0]);
		}
		ret += popCount64(blk.data[q] & mask);
		return ret;
	}
	uint64_t popCount64(uint64_t x) const
	{
		return _mm_popcnt_u64(x);
	}
	void init(const uint64_t *blk, size_t blkNum)
	{
		size_t tblNum = (blkNum + 7) / 8;
		blk_.resize(tblNum);

		uint32_t r = 0;
		size_t pos = 0;
		for (size_t i = 0; i < tblNum; i++) {
			B& b = blk_[i];
			b.rank = r;
			for (size_t j = 0; j < 8; j++) {
				uint64_t v = pos < blkNum ? blk[pos++] : 0;
				b.data[j] = v;
				r += popCount64(v);
			}
		}
	}
};

class SBV5 {
	struct B {
		uint64_t rank;
		uint64_t data[4];
	};
	AlignedArray<B> blk_;
	SBV5(const SBV5&);
	void operator=(const SBV5&);
public:
	SBV5()
	{
	}
	SBV5(const uint64_t *blk, size_t blkNum)
	{
		init(blk, blkNum);
	}
	uint64_t rank1(size_t idx) const
	{
		return rank1m(idx);
	}
	uint64_t rank1m(size_t idx) const
	{
		const uint64_t mask = (uint64_t(2) << (idx & 63)) - 1;
		const B& blk = blk_[idx / 256];
		uint64_t q = (idx / 64) % 4;
		uint64_t ret = blk.rank;
		switch (q) {
		case 3: ret += popCount64(blk.data[2]);
		case 2: ret += popCount64(blk.data[1]);
		case 1: ret += popCount64(blk.data[0]);
		}
		ret += popCount64(blk.data[q] & mask);
		return ret;
	}
	uint64_t popCount64(uint64_t x) const
	{
		return _mm_popcnt_u64(x);
	}
	void init(const uint64_t *blk, size_t blkNum)
	{
		size_t tblNum = (blkNum + 3) / 4;
		blk_.resize(tblNum);

		uint32_t r = 0;
		size_t pos = 0;
		for (size_t i = 0; i < tblNum; i++) {
			B& b = blk_[i];
			b.rank = r;
			for (size_t j = 0; j < 4; j++) {
				uint64_t v = pos < blkNum ? blk[pos++] : 0;
				b.data[j] = v;
				r += popCount64(v);
			}
		}
	}
};
/*
	(32 + 8 * 4) / 512 = 1/8
*/
class SBV6 {
	struct B2 {
		uint32_t rank;
		union {
			uint8_t s8[4];
			uint32_t s;
		} ci;
		uint64_t data[8];
	};
	AlignedArray<B2> blk_;
	SBV6(const SBV6&);
	void operator=(const SBV6&);
public:
	SBV6()
	{
	}
	SBV6(const uint64_t *blk, size_t blkNum)
	{
		init(blk, blkNum);
	}
	uint64_t rank1(size_t idx) const
	{
		return rank1m(idx);
	}
	uint64_t rank1m(size_t idx) const
	{
		const uint64_t mask = (uint64_t(2) << (idx & 63)) - 1;
		const B2& blk = blk_[idx / 512];
		uint64_t ret = blk.rank;
		uint64_t q = (idx / 128) % 4;
		uint64_t b0 = blk.data[q * 2 + 0];
		uint64_t b1 = blk.data[q * 2 + 1];
		uint64_t m0 = -1;
		uint64_t m1 = 0;
		if (!(idx & 64)) m0 = mask;
		if (idx & 64) m1 = mask;
		ret += popCount64(b0 & m0);
		ret += popCount64(b1 & m1);
#if 1
#if 1
		uint32_t x = blk.ci.s & ((1U << (q * 8)) - 1);
		V128 v(x);
		v = psadbw(v, Zero());
		ret += movd(v);
#else
		switch (q) {
		case 3: ret += blk.ci.s8[2];
		case 2: ret += blk.ci.s8[1];
		case 1: ret += blk.ci.s8[0];
		}
#endif
#else
		for (uint64_t i = 0; i < q; i++) {
			ret += blk.ci.s8[i];
		}
#endif
		return ret;
	}
	uint64_t popCount64(uint64_t x) const
	{
		return _mm_popcnt_u64(x);
	}
	void init(const uint64_t *blk, size_t blkNum)
	{
		size_t tblNum = (blkNum + 7) / 8;
		blk_.resize(tblNum);

		uint32_t r = 0;
		size_t pos = 0;
		for (size_t i = 0; i < tblNum; i++) {
			B2& b = blk_[i];
			b.rank = r;
			uint8_t s8 = 0;
			for (size_t j = 0; j < 4; j++) {
				uint64_t vL = pos < blkNum ? blk[pos++] : 0;
				uint64_t vH = pos < blkNum ? blk[pos++] : 0;
				b.data[j * 2 + 0] = vL;
				b.data[j * 2 + 1] = vH;
				s8 = popCount64(vL) + popCount64(vH);
				b.ci.s8[j] = s8;
				r += s8;
			}
		}
	}
};
} // mie

