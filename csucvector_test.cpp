#include <stdio.h>
#include <stdlib.h>
#include <cybozu/mmap.hpp>
#include <cybozu/bit_operation.hpp>
#include <cybozu/xorshift.hpp>
#include <cybozu/sucvector.hpp>
#include <cybozu/csucvector.hpp>
#include <cybozu/bitvector.hpp>
#include <cybozu/benchmark.hpp>
#include <fstream>
#include <algorithm>
#include <vector>

#ifdef _MSC_VER
	#pragma warning(disable : 4351)
#endif

#ifdef COMPARE_SDSL
#include <sdsl/rrr_vector.hpp>
struct SdslVec {
	sdsl::rrr_vector<> rrr;
	sdsl::rrr_rank_support<> rk;
	SdslVec(const uint64_t *block, size_t blockNum)
	{
		sdsl::bit_vector bv(blockNum * sizeof(uint64_t) * 8);
		for (size_t i = 0; i < blockNum; i++) {
			for (size_t j = 0; j < 64; j++) {
				bool b = ((block[i] >> j) & 1) !=0 ? 1 : 0;
				bv[i * 64 + j] = b;
			}
		}
		rrr = sdsl::rrr_vector<>(bv);
		rk.init(&rrr);
	}
	size_t rank1(size_t i) const
	{
		return rk.rank(i);
	}
	bool get(size_t i) const
	{
		return rrr[i];
	}
	void save(const std::string& name) const
	{
		std::ofstream ofs(name.c_str(), std::ios::binary);
		rrr.serialize(ofs);
		rk.serialize(ofs);
	}
};
#endif

void add(int& z, int x)
{
	z += x;
}

int main(int argc, char *argv[])
	try
{
	argc--, argv++;
	if (argc == 0) {
		printf("cmd <file>\n");
		return 1;
	}
	cybozu::Mmap m(*argv);
	const uint64_t *blk = (const uint64_t*)m.get();
	const size_t bitSize = m.size() * 8;
	cybozu::SucVector sv;
	sv.init(blk, bitSize);
	cybozu::BitVector bv(blk, bitSize);
	cybozu::CSucVector csv(blk, bitSize);
#ifdef COMPARE_SDSL
	SdslVec sdsl(blk, bitSize / 64);
	sdsl.save("sdsl.data");
#endif
	puts("test rank");
	for (size_t pos = 0; pos < std::min<size_t>(100000ull, bitSize); pos++) {
		size_t a = csv.rank1(pos);
		size_t b = sv.rank1(pos);
#ifdef COMPARE_SDSL
		size_t c = sdsl.rank1(pos);
		if (a != c) {
			printf("rank1(%d)=%d, %d\n", (int)pos, (int)a, (int)c);
			exit(1);
		}
#endif
		if (a != b) {
			printf("rank1(%d)=%d, %d (%d, %d, %d)\n", (int)pos, (int)a, (int)b, csv.get(pos), sv.get(pos), bv.get(pos));
			exit(1);
		}
	}

	puts("test get");
	cybozu::CpuClock clk;
	for (size_t pos = 0; pos < std::min<size_t>(1000000ull, bitSize); pos++) {
		bool a = csv.get(pos);
		bool b = sv.get(pos);
#ifdef COMPARE_SDSL
		bool c = sdsl.get(pos);
		if (a != c) {
			printf("rank1(%d)=%d, %d\n", (int)pos, a, c);
			exit(1);
		}
#endif
		if (a != b) {
			printf("err get(%d)=%d, %d\n", (int)pos, a, b);
			exit(1);
		}
	}
	int z = 0;
	cybozu::XorShift rg;
	CYBOZU_BENCH("sb   get", add, z, sv.get(rg() % bitSize));
	CYBOZU_BENCH("sb  rank", add, z, (int)sv.rank1(rg() % bitSize));

	CYBOZU_BENCH("csv  get", add, z, csv.get(rg() % bitSize));
	CYBOZU_BENCH("csv rank", add, z, (int)csv.rank1(rg() % bitSize));

#ifdef COMPARE_SDSL
	CYBOZU_BENCH("sds  get", add, z, sdsl.get(rg() % bitSize));
	CYBOZU_BENCH("sds rank", add, z, (int)sdsl.rank1(rg() % bitSize));
#endif
	return z * 0;

} catch (std::exception& e) {
	printf("err %s\n", e.what());
}
