#pragma once

#include <vector>
#include <assert.h>
#include "util.hpp"

namespace mie {

class BitVector {
	size_t bitSize_;
	std::vector<uint64_t> v_;
public:
	BitVector()
		: bitSize_(0)
	{
	}
	void resize(size_t bitSize)
	{
		bitSize_ = bitSize;
		v_.resize((bitSize + 63) / 64);
	}
	bool get(size_t idx) const
	{
		size_t q = idx / 64;
		size_t r = idx % 64;
		return (v_[q] & (1ULL << r)) != 0;
	}
	void set(size_t idx, bool b)
	{
		size_t q = idx / 64;
		size_t r = idx % 64;
		uint64_t v = v_[q];
		v &= ~(1ULL << r);
		if (b) v |= (1ULL << r);
		v_[q] = v;
	}
	size_t size() const { return bitSize_; }
	const uint64_t *getBlock() const { return &v_[0]; }
	uint64_t *getBlock() { return &v_[0]; }
	size_t getBlockSize() const { return v_.size(); }
	void put() const
	{
		printf(">%016llx:%016llx:%016llx:%016llx\n", (long long)v_[3], (long long)v_[2], (long long)v_[1], (long long)v_[0]);
	}
};

class SuccinctBitVector {
	struct Block {
		uint64_t b0;
		uint64_t b1;
		uint32_t rank;
		uint32_t padding;
	};
	AlignedArray<Block> blk_;
public:
	SuccinctBitVector()
	{
	}
	SuccinctBitVector(const uint64_t *blk, size_t blkNum)
	{
		init(blk, blkNum);
	}
	inline uint32_t rank1(size_t idx) const
	{
		const Block& blk = blk_[idx / 128];
		size_t ret = blk.rank;
		uint64_t r = idx % 64;
		uint64_t b0 = blk.b0;
		uint64_t b1 = blk.b1;
		uint64_t mask = (2ULL << r) - 1;
		uint64_t m = (idx & 64) ? (-1) : 0;
		uint64_t m0 = mask | m;
		uint64_t m1 = mask & m;
		ret += popCount64(b0 & m0);
		ret += popCount64(b1 & m1);
		return (uint32_t)ret;
	}
	inline uint64_t popCount64(uint64_t x) const
	{
		return _mm_popcnt_u64(x);
	}
	void init(const uint64_t *blk, size_t blkNum)
	{
		size_t tblNum = (blkNum + 1) / 2;
		blk_.resize(tblNum);

		uint32_t r = 0;
		size_t pos = 0;
		for (size_t i = 0; i < tblNum; i++, pos += 2) {
			uint64_t b0 = blk[pos];
			uint64_t b1 = (pos + 1 < blkNum) ? blk[pos + 1] : 0;
			blk_[i].b0 = b0;
			blk_[i].b1 = b1;
			blk_[i].rank = r;
			r += popCount64(b0);
			r += popCount64(b1);
		}
	}
};

} // mie
