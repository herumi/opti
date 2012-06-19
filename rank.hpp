#pragma once

#include <vector>
#include <assert.h>
#include "util.hpp"

namespace mie {

class BitVector {
	size_t bitSize_;
//	std::vector<uint64_t> v_;
	AlignedArray<uint64_t> v_;
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
	const uint64_t *blk_;
	std::vector<uint32_t> tbl_; // 256-bit
public:
	SuccinctBitVector()
		: blk_(0)
	{
	}
	SuccinctBitVector(const uint64_t *blk, size_t blkNum)
	{
		init(blk, blkNum);
	}
	inline uint32_t rank1(size_t idx) const
	{
		uint32_t ret = tbl_[idx / 256];
		uint64_t q = (idx / 64) & 3;
		uint64_t r = idx % 64;
		size_t round = (idx / 64) & ~size_t(3);
		uint64_t b0 = blk_[round + 0];
		uint64_t b1 = blk_[round + 1];
		uint64_t b2 = blk_[round + 2];
		uint64_t b3 = blk_[round + 3];
		uint64_t mask = (2ULL << r) - 1;
#if 0
		uint64_t m0 = q < 1 ? mask : (-1);
		uint64_t m1 = q < 1 ? 0 : q == 1 ? mask : (-1);
		uint64_t m2 = q < 2 ? 0 : q == 2 ? mask : (-1);
		uint64_t m3 = q < 3 ? 0 : mask;
		ret += popCount64(b0 & m0);
		ret += popCount64(b1 & m1);
		ret += popCount64(b2 & m2);
		ret += popCount64(b3 & m3);
#else
		if (q < 1) {
			ret += popCount64(b0 & mask);
			return ret;
		}
		ret += popCount64(b0);
		if (q < 2) {
			ret += popCount64(b1 & mask);
			return ret;
		}
		ret += popCount64(b1);
		if (q < 3) {
			ret += popCount64(b2 & mask);
			return ret;
		}
		ret += popCount64(b2);
		ret += popCount64(b3 & mask);
#endif
		return ret;
	}
	inline uint32_t popCount64(uint64_t x) const
	{
		return (uint32_t)_mm_popcnt_u64(x);
	}
	void init(const uint64_t *blk, size_t blkNum)
	{
		assert((blkNum % 4) == 0);
		blk_ = blk;
		// set rank table and gTbl_
		tbl_.resize((blkNum + 3) / 4);

		uint32_t r = 0;
		size_t pos = 0;
		for (size_t i = 0; i < tbl_.size(); i++) {
			tbl_[i] = r;
			for (int j = 0; j < 4; j++) {
				if (pos < blkNum) {
					r += popCount64(blk_[pos++]);
				}
			}
		}
		if (pos != ((blkNum + 3) & ~3)) {
			fprintf(stderr, "bad pos=%d\n", (int)pos);
			exit(1);
		}
	}
};

} // mie
