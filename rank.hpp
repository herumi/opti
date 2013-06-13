#pragma once

#include <vector>
#include <assert.h>
#include <stddef.h>
#include "v128.h"
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>

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

struct Block {
	uint32_t rank;
	union {
		uint8_t s8[4];
		uint32_t s;
	} ci;
	uint64_t data[8];
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
		const Reg32& idx = edx;
		mov(r8, rcx);
#else
		const Reg64& blk = rdi;
		const Reg32& idx = esi;
#endif
		const Reg64& mask = r9;
		const Reg64& m0 = r11;
		const Xmm& zero = xm0;
		const Xmm& v = xm1;

		mov(ecx, idx);
		and_(ecx, 63);
		xor_(eax, eax);
		inc(eax);
		shl(rax, cl);
		sub(rax, 1);
		mov(mask, rax);
		mov(eax, idx);
		shr(eax, 9);
		imul(eax, eax, sizeof(succ_impl::Block));
		add(blk, rax);
		mov(rcx, idx);
		shr(ecx, 7);
		and_(ecx, 3); // q
		shl(ecx, 3); // q * 8
		or_(m0, uint32_t(-1));
		and_(idx, 64);
		cmovz(m0, mask); // m0 = !(idx & 64) ? mask : -1
		cmovz(mask, idx); // mask = (idx & 64) ? 0(=idx) : mask
		// idx is free, so use edx
		and_(m0,   ptr [blk + offsetof(succ_impl::Block, data) + rcx * 2 + 0]);
		and_(mask, ptr [blk + offsetof(succ_impl::Block, data) + rcx * 2 + 8]);
		popcnt(m0, m0);
		popcnt(rax, mask);
		add(rax, m0);

		mov(edx, 1);
		add(eax, ptr [blk + offsetof(succ_impl::Block, rank)]);
		shl(edx, cl);
		sub(edx, 1);
		and_(edx, ptr [blk + offsetof(succ_impl::Block, ci.s)]);
		movd(v, edx);

		pxor(zero, zero);
		psadbw(v, zero);
		movd(edx, v);
		add(eax, edx);
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

/*
	extra memory
	(32 + 8 * 4) / 256 = 1/4
*/
struct SBV1 {
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
	SBV1()
	{
	}
	SBV1(const uint64_t *blk, size_t blkNum)
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
				int c = (int)popCount64(v);
				av += c;
				b.b[j] = (uint8_t)bv;
				bv += c;
			}
		}
	}
	uint32_t rank1(size_t i) const
	{
		if (i == 0) return 0;
		size_t q = i / 256;
		size_t r = (i / 64) & 3;
		const B& b = blk_[q];
		return uint32_t(b.a + b.b[r] + popCount64(b.org[r] & ((1ULL << (i & 63)) - 1)));
	}
};

/*
	extra memory
	(32 + 8 * 4) / 512 = 1/8
*/
class SBV2 {
	AlignedArray<succ_impl::Block> blk_;
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
#if 1
		return ((uint64_t (*)(const succ_impl::Block*, size_t))((char*)succ_impl::InstanceIsHere<>::buf))(&blk_[0], idx);
#else
		const uint64_t mask = (uint64_t(1) << (idx & 63)) - 1;
		const succ_impl::Block& blk = blk_[idx / 512];
		uint64_t ret = blk.rank;
		uint64_t q = (idx / 128) % 4;
		uint64_t b0 = blk.data[q * 2 + 0];
		uint64_t b1 = blk.data[q * 2 + 1];
		uint64_t m0 = (idx & 64) ? -1 : mask;
		uint64_t m1 = (idx & 64) ? mask : 0;
		ret += popCount64(b0 & m0);
		ret += popCount64(b1 & m1);
		uint32_t x = blk.ci.s & ((1U << (q * 8)) - 1);
		V128 v(x);
		v = psadbw(v, Zero());
		ret += movd(v);
		return ret;
#endif
	}
	uint64_t select1(uint64_t) const { return 0; }
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
			succ_impl::Block& b = blk_[i];
			b.rank = r;
			uint8_t s8 = 0;
			for (size_t j = 0; j < 4; j++) {
				uint64_t vL = pos < blkNum ? blk[pos++] : 0;
				uint64_t vH = pos < blkNum ? blk[pos++] : 0;
				b.data[j * 2 + 0] = vL;
				b.data[j * 2 + 1] = vH;
				s8 = uint8_t(popCount64(vL) + popCount64(vH));
				b.ci.s8[j] = s8;
				r += s8;
			}
		}
	}
};

} // mie

