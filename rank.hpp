#pragma once

#include <vector>
#include <assert.h>
#include "util.hpp"
#include "v128.h"

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

//#define USE_TABLE_1024
class SuccinctBitVector {
	enum {
#ifdef USE_TABLE_1024
		TABLE_SIZE = 1024,
#else
		TABLE_SIZE = 512,
#endif
		DATA_NUM = TABLE_SIZE / 64
	};
	struct Block {
		uint64_t data[DATA_NUM];
		uint8_t s8[8];
		uint32_t rank;
		uint32_t pad;
	};
	AlignedArray<Block> blk_;
	SuccinctBitVector(const SuccinctBitVector&);
	void operator=(const SuccinctBitVector&);
public:
	void swap(SuccinctBitVector& rhs) throw()
	{
		blk_.swap(rhs.blk_);
	}
	SuccinctBitVector()
	{
	}
	SuccinctBitVector(const uint64_t *blk, size_t blkNum)
	{
		init(blk, blkNum);
	}
	inline uint32_t rank1(size_t idx) const
	{
		const Block& blk = blk_[idx / TABLE_SIZE];
		size_t ret = blk.rank;
		uint64_t q = (idx / (TABLE_SIZE / 8)) % 8;
		V128 vmask;
		vmask = pcmpeqd(vmask, vmask); // all [-1]
		V128 shift((8 - q) * 8);
		vmask = psrlq(vmask, shift);
		V128 v = V128((uint32_t*)blk.s8);
		v = pand(v, vmask);
		v = psadbw(v, Zero());
		ret += movd(v);
		uint64_t mask = (uint64_t(2) << (idx & 63)) - 1;
#ifdef USE_TABLE_1024
		uint64_t b0 = blk.data[q * 2 + 0];
		uint64_t b1 = blk.data[q * 2 + 1];
		uint64_t m0 = -1;
		uint64_t m1 = 0;
		if (!(idx & 64)) m0 = mask;
		if(idx & 64) m1 = mask;
		ret += popCount64(b0 & m0);
		ret += popCount64(b1 & m1);
#else
		ret += popCount64(blk.data[q] & mask);
#endif
		return (uint32_t)ret;
	}
	inline uint64_t popCount64(uint64_t x) const
	{
		return _mm_popcnt_u64(x);
	}
	void init(const uint64_t *blk, size_t blkNum)
	{
		size_t tblNum = (blkNum + DATA_NUM - 1) / DATA_NUM;
		blk_.resize(tblNum);

		uint32_t r = 0;
		size_t pos = 0;
		for (size_t i = 0; i < tblNum; i++) {
			Block& b = blk_[i];
			b.rank = r;
			for (size_t j = 0; j < 8; j++) {
				uint64_t s8 = 0;
				for (size_t k = 0; k < DATA_NUM / 8; k++) {
					uint64_t v = pos < blkNum ? blk[pos++] : 0;
					b.data[j * (DATA_NUM / 8) + k] = v;
					s8 += popCount64(v);
				}
				r += s8;
				b.s8[j] = static_cast<uint8_t>(s8);
			}
		}
	}
};

} // mie
