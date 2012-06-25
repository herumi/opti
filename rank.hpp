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

class SuccinctBitVector {
	struct Block {
		uint8_t s8[8];
		uint32_t rank;
		uint8_t pad[4];
	};
	const uint64_t *org_;
	AlignedArray<Block> blk_;
public:
	SuccinctBitVector()
		: org_(0)
	{
	}
	SuccinctBitVector(const uint64_t *blk, size_t blkNum)
		: org_(0)
	{
		init(blk, blkNum);
	}
	V128 shr_byte(const V128& a, uint32_t idx) const
	{
		switch (idx) {
		default:
		case  0: return a;
		case  1: return psrldq<1>(a);
		case  2: return psrldq<2>(a);
		case  3: return psrldq<3>(a);
		case  4: return psrldq<4>(a);
		case  5: return psrldq<5>(a);
		case  6: return psrldq<6>(a);
		case  7: return psrldq<7>(a);
		case  8: return psrldq<8>(a);
		case  9: return psrldq<9>(a);
		case 10: return psrldq<10>(a);
		case 11: return psrldq<11>(a);
		case 12: return psrldq<12>(a);
		case 13: return psrldq<13>(a);
		case 14: return psrldq<14>(a);
		case 15: return psrldq<15>(a);
		case 16: return Zero();
		}
	}
	inline uint32_t rank1(size_t idx) const
	{
		const Block& blk = blk_[idx / 512];
		size_t ret = blk.rank;
		uint64_t q = (idx / 64) % 8;
		V128 vmask;
		vmask = pcmpeqd(vmask, vmask); // all [-1]
		V128 shift((8 - q) * 8);
		vmask = psrlq(vmask, shift);
		V128 v = V128((uint32_t*)blk.s8);
		v = pand(v, vmask);
		v = psadbw(v, Zero());
		ret += movd(v);
		uint64_t mask = (uint64_t(2) << (idx & 63)) - 1;
		ret += popCount64(org_[idx / 64] & mask);
		return ret;
	}
	inline uint64_t popCount64(uint64_t x) const
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
			Block& b = blk_[i];
			b.rank = r;
			for (size_t j = 0; j < 8; j++) {
				uint64_t v = pos < blkNum ? blk[pos++] : 0;
				uint64_t s8 = popCount64(v);
				r += s8;
				b.s8[j] = static_cast<uint8_t>(s8);
			}
		}
	}
};

} // mie
