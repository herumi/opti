#pragma once

#include <vector>
#include <assert.h>
#include <stddef.h>
#include "util.hpp"
#include "v128.h"
#include <xbyak/xbyak.h>

//#define MIE_RANK_USE_TABLE_1024

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

namespace succ_impl {

#ifdef MIE_RANK_USE_TABLE_1024
const size_t TABLE_SHIFT = 10;
#else
const size_t TABLE_SHIFT = 9;
#endif
const size_t TABLE_SIZE = size_t(1) << TABLE_SHIFT;
const size_t DATA_NUM = TABLE_SIZE / 64;

struct Block {
	uint64_t data[DATA_NUM];
	uint8_t s8[8];
	uint64_t rank;
};

struct Code : Xbyak::CodeGenerator {
	Code(char *buf, size_t size)
		try
		: Xbyak::CodeGenerator(size, buf)
	{
		Xbyak::CodeArray::protect(buf, size, true);
		gen_rank1();
	} catch (Xbyak::Error err) {
		printf("ERR:%s(%d)\n", Xbyak::ConvertErrorToString(err), err);
		::exit(1);
	}
private:
	/*
		rank1(const Block *blk, size_t idx);
	*/
	void gen_rank1()
	{
		using namespace Xbyak;
#ifdef XBYAK32
		#error "not implemented for 32-bit version"
#endif
#ifdef XBYAK64_WIN
		const Reg64& blk = r8;
		const Reg64& idx = rdx;
		mov(r8, rcx);
#else
		const Reg64& blk = rdi;
		const Reg64& idx = rsi;
#endif
		const Reg64& mask = r9;
		const Xmm& vmask = xm0;
		const Xmm& shift = xm1;
		const Xmm& v = xm2;

		mov(rcx, idx);
		and(ecx, 63);
		mov(r9d, 2);
		shl(mask, cl);
		sub(mask, 1);
		mov(rax, idx);
		shr(rax, succ_impl::TABLE_SHIFT);
		imul(rax, rax, sizeof(succ_impl::Block));
//		shl(rax, 4);
//		lea(rax, ptr [rax + rax * 4]);
		add(blk, rax);
#ifdef MIE_RANK_USE_TABLE_1024
		const Reg64& q = r10;
		mov(q, idx);
		shr(q, succ_impl::TABLE_SHIFT - 3);
		and(q, 7);
		const Reg64& m0 = r11;
		lea(rax, ptr [q + q]);
		or(m0, -1);
		and(idx, 64);
		cmovz(m0, mask); // m0 = (!(idx & 64)) ? mask : -1
		cmovz(mask, idx); // mask = (idx & 64) ? mask : 0
		and(m0, ptr [blk + rax * 8 + 0]);
		and(mask, ptr [blk + rax * 8 + 8]);
		popcnt(m0, m0);
		popcnt(rax, mask);
		add(rax, m0);
#else
		const Reg64& q = idx;
		shr(q, succ_impl::TABLE_SHIFT - 3);
		and(q, 7);
		and(mask, ptr [blk + q * 8]);
		popcnt(rax, mask);
#endif
		neg(q);
		add(q, 8);
		shl(q, 3);
		movq(shift, q);
		pcmpeqd(vmask, vmask);
		psrlq(vmask, shift);
		movq(v, ptr [blk + offsetof(succ_impl::Block, s8)]);
		pand(v, vmask);
		pxor(vmask, vmask);
		psadbw(v, vmask);
		movq(q, v);
		add(rax, q);
		add(rax, ptr [blk + offsetof(succ_impl::Block, rank)]);
		ret();
	}
};

template<int dummy = 0>
struct InstanceIsHere {
	static MIE_ALIGN(4096) char buf[4096];
	static Code code;
};

template<int dummy>
Code InstanceIsHere<dummy>::code(buf, sizeof(buf));

template<int dummy>
char InstanceIsHere<dummy>::buf[4096];

struct DummyCall {
	DummyCall() { InstanceIsHere<>::code.getCode(); }
};

} // mie::succ_impl

class SuccinctBitVector {
	AlignedArray<succ_impl::Block> blk_;
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
	inline uint64_t rank1(size_t idx) const
	{
#ifdef MIE_RANK_USE_TABLE_1024
		return ((uint64_t (*)(const succ_impl::Block*, size_t))((char*)succ_impl::InstanceIsHere<>::buf))(&blk_[0], idx);
#else
		const uint64_t mask = (uint64_t(2) << (idx & 63)) - 1;
		const succ_impl::Block& blk = blk_[idx >> succ_impl::TABLE_SHIFT];
		uint64_t q = (idx >> (succ_impl::TABLE_SHIFT - 3)) % 8;
#ifdef MIE_RANK_USE_TABLE_1024
		uint64_t b0 = blk.data[q * 2 + 0];
		uint64_t b1 = blk.data[q * 2 + 1];
		uint64_t m0 = -1;
		uint64_t m1 = 0;
		if (!(idx & 64)) m0 = mask;
		if (idx & 64) m1 = mask;
		uint64_t ret = popCount64(b0 & m0);
		ret += popCount64(b1 & m1);
#else
		uint64_t b0 = blk.data[q];
		uint64_t m0 = b0 & mask;
		uint64_t ret = popCount64(b0 & m0);
#endif
		V128 vmask;
		vmask = pcmpeqd(vmask, vmask); // all [-1]
		V128 shift((8 - q) * 8);
		vmask = psrlq(vmask, shift);
		V128 v = V128((uint32_t*)blk.s8);
		v = pand(v, vmask);
		v = psadbw(v, Zero());
		ret += movd(v);
		ret += blk.rank;
		return ret;
#endif
	}
	inline uint64_t popCount64(uint64_t x) const
	{
		return _mm_popcnt_u64(x);
	}
	void init(const uint64_t *blk, size_t blkNum)
	{
		size_t tblNum = (blkNum + succ_impl::DATA_NUM - 1) / succ_impl::DATA_NUM;
		blk_.resize(tblNum);

		uint64_t r = 0;
		size_t pos = 0;
		for (size_t i = 0; i < tblNum; i++) {
			succ_impl::Block& b = blk_[i];
			b.rank = r;
			for (size_t j = 0; j < 8; j++) {
				uint64_t s8 = 0;
				for (size_t k = 0; k < succ_impl::DATA_NUM / 8; k++) {
					uint64_t v = pos < blkNum ? blk[pos++] : 0;
					b.data[j * (succ_impl::DATA_NUM / 8) + k] = v;
					s8 += popCount64(v);
				}
				r += s8;
				b.s8[j] = static_cast<uint8_t>(s8);
			}
		}
	}
};

} // mie

