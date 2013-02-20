#include <stdio.h>
#include "rank.hpp"
#include <cybozu/wavelet_matrix.hpp>
#include <cybozu/time.hpp>
#include "util.hpp"
#include <xbyak/xbyak_util.h>
//#define USE_C11

#ifdef USE_C11
#include <random>
struct C11RandomGenerator {
	std::mt19937 rg_;
	std::uniform_int_distribution<uint64_t> dist(0, ~uint64_t(0));
	C11RandomGenerator()
	{
		rg_.seed(0);
	}
	uint64_t get64()
	{
		return dist(rg_);
	}
};
typedef C11RandomGenerator RandomGenerator;
#else
#include <cybozu/xorshift.hpp>
typedef cybozu::XorShift RandomGenerator;
#endif

struct Vec8 : std::vector<uint8_t> {
	size_t get(size_t pos) const
	{
		return (*this)[pos];
	}
	size_t rank(uint32_t val, size_t pos) const
	{
		return std::count(begin(), begin() + pos, val);
	}
	size_t rankLt(uint32_t val, size_t pos) const
	{
		size_t ret = 0;
		for (size_t i = 0, n = pos; i < n; i++) {
			if ((*this)[i] < val) ret++;
		}
		return ret;
	}
	size_t select(uint32_t val, size_t rank) const
	{
		const size_t N = size();
		rank++;
		for (size_t i = 0; i < N; i++) {
			if ((*this)[i] == val) rank--;
			if (rank == 0) return i;
		}
		return cybozu::NotFound;
	}
};

#ifdef COMPARE_WAT
#include "wat_array.hpp"
struct Wat {
	wat_array::WatArray wm;
	void init(const Vec8& v8, int)
	{
		std::vector<uint64_t> v64;
		v64.resize(v8.size());
		for (size_t i = 0; i < v8.size(); i++) {
			v64[i] = v8[i];
		}
		wm.Init(v64);
	}
	uint64_t get(uint64_t pos) const
	{
		return wm.Lookup(pos);
	}
	uint64_t rank(uint32_t val, uint64_t pos) const
	{
		return wm.Rank(val, pos);
	}
	uint64_t rankLt(uint32_t val, uint64_t pos) const
	{
		return wm.RankLessThan(val, pos);
	}
	uint64_t select(uint32_t val, uint64_t rank) const
	{
		return wm.Select(val, rank + 1) - 1;
	}
	size_t size() const { return wm.length(); }
	size_t size(uint32_t val) const
	{
		return rank(val, wm.length());
	}
};
#endif
#ifdef COMPARE_WAVELET
#include "wavelet_matrix.hpp"
struct Wavelet {
	wavelet_matrix::WaveletMatrix wm;
	void init(const Vec8& v8, int)
	{
		std::vector<uint64_t> v64;
		v64.resize(v8.size());
		for (size_t i = 0; i < v8.size(); i++) {
			v64[i] = v8[i];
		}
		wm.Init(v64);
	}
	uint64_t get(uint64_t pos) const
	{
		return wm.Lookup(pos);
	}
	uint64_t rank(uint32_t val, uint64_t pos) const
	{
		return wm.Rank(val, pos);
	}
	uint64_t rankLt(uint32_t val, uint64_t pos) const
	{
		return wm.RankLessThan(val, pos);
	}
	uint64_t select(uint32_t val, uint64_t rank) const
	{
		return wm.Select(val, rank + 1) - 1;
	}
	size_t size() const { return wm.length(); }
	size_t size(uint32_t val) const
	{
		return rank(val, wm.length());
	}
};
#endif
#ifdef COMPARE_SHELLINFORD
#include "shellinford_wavelet_matrix.h"
struct Shellinford {
	shellinford::wavelet_matrix<uint8_t> wm;
	void init(const Vec8& v8, int)
	{
		Vec8 v = v8;
		wm.build(v); // why not const?
	}
	uint64_t get(uint64_t pos) const
	{
		return wm.get(pos);
	}
	uint64_t rank(uint32_t val, uint64_t pos) const
	{
		return wm.rank(pos, val);
	}
	uint64_t rankLt(uint32_t val, uint64_t pos) const
	{
		return wm.rank_less_than(pos, val);
	}
	uint64_t select(uint32_t val, uint64_t rank) const
	{
		return wm.select(rank, val);
	}
	size_t size() const { return wm.size(); }
	size_t size(uint32_t val) const
	{
		return wm.size(val);
	}
};
#endif

template<class T, class RG>
void bench_get(const T& wm, const Vec8& v8, RG& rg, size_t C, size_t N)
{
	cybozu::disable_warning_unused_variable(v8);
	size_t ret = 0;
	double begin = cybozu::GetCurrentTimeSec();
	for (size_t i = 0; i < C; i++) {
		size_t pos = rg() & (N - 1);
		ret += wm.get(pos);
	}
	double t = cybozu::GetCurrentTimeSec() - begin;
	printf("get     %08x %9.2fusec\n", (int)ret, t / C * 1e6);
}

template<class T, class RG>
void bench_rank(const T& wm, const Vec8& v8, RG& rg, size_t C, size_t N)
{
	cybozu::disable_warning_unused_variable(v8);
	size_t ret = 0;
	double begin = cybozu::GetCurrentTimeSec();
	for (size_t i = 0; i < C; i++) {
		size_t pos = rg() & (N - 1);
		uint8_t c = uint8_t(rg());
		uint64_t a = wm.rank(c, pos);
#if 0
		uint64_t b = v8.rank(c, pos);
		if (a != b) {
			printf("ERR i=%d a=%d b=%d c=%d pos=%d\n", (int)i, (int)a, (int)b, (int)c, (int)pos);
			exit(1);
		}
#endif
		ret += a;
	}
	double t = cybozu::GetCurrentTimeSec() - begin;
	printf("rank    %08x %9.2fusec\n", (int)ret, t / C * 1e6);
}

template<class T, class RG>
void bench_rankLt(const T& wm, const Vec8& v8, RG& rg, size_t C, size_t N)
{
	cybozu::disable_warning_unused_variable(v8);
	size_t ret = 0;
	double begin = cybozu::GetCurrentTimeSec();
	for (size_t i = 0; i < C; i++) {
		size_t pos = rg() & (N - 1);
		uint8_t c = uint8_t(rg());
		ret += wm.rankLt(c, pos);
	}
	double t = cybozu::GetCurrentTimeSec() - begin;
	printf("rankLt  %08x %9.2fusec\n", (int)ret, t / C * 1e6);
}

template<class T, class RG>
void bench_select(const T& wm, const Vec8& v8, RG& rg, size_t C)
{
	cybozu::disable_warning_unused_variable(v8);
	size_t ret = 0;
	std::vector<int> maxTbl;
	maxTbl.resize(256);
	for (int i = 0; i < 256; i++) {
		int v = (int)wm.size(i);
		if (v == 0) v = 1;
		maxTbl[i] = v;
	}
	double begin = cybozu::GetCurrentTimeSec();
	for (size_t i = 0; i < C; i++) {
		uint8_t c = uint8_t(rg());
		size_t pos = rg() % maxTbl[c];
		uint64_t a = wm.select(c, pos);
#if 0
		uint64_t b = v8.select(c, pos);
		if (a != b) {
			printf("ERR i=%d a=%d b=%d c=%d pos=%d\n", (int)i, (int)a, (int)b, (int)c, (int)pos);
			exit(1);
		}
#endif
		ret += a;
	}
	double t = cybozu::GetCurrentTimeSec() - begin;
	printf("select  %08x %9.2fusec\n", (int)ret, t / C * 1e6);
}

template<class T>
void bench(const char *msg, const Vec8& v, size_t N)
{
	puts(msg);
	T wm;
	wm.init(v, 8);
	RandomGenerator rg;
	bench_get(wm, v, rg, 1000000, N);
	bench_rank(wm, v, rg, 1000000, N);
	bench_rankLt(wm, v, rg, 1000000, N);
	bench_select(wm, v, rg, 100000);
}

void run(size_t bitLen)
{
	if (bitLen < 8) {
		printf("too small bitLen=%d\n", (int)bitLen);
		exit(1);
	}
	const size_t N = size_t(1) << bitLen;
	RandomGenerator rg;
	printf("%09llx\n", (long long)N);
	puts("init");
	Vec8 v;
	v.resize(N);
	for (size_t i = 0; i < 256; i++) {
		v[i] = uint8_t(i);
	}
	for (size_t i = 256; i < N; i++) {
		v[i] = uint8_t(rg());
	}
	puts("start");
	bench<cybozu::WaveletMatrix>("wm", v, N);
#ifdef COMPARE_WAT
	bench<Wat>("wat", v, N);
#endif
#ifdef COMPARE_WAVELET
	bench<Wavelet>("wavelet", v, N);
#endif
#ifdef COMPARE_SHELLINFORD
	bench<Shellinford>("shellinford", v, N);
#endif
}

int main(int argc, char *argv[])
	try
{
	size_t bitLen = 26;
	argc--, argv++;
	while (argc > 0) {
		if (argc > 1 && strcmp(*argv, "-b") == 0) {
			argc--, argv++;
			bitLen = atoi(*argv);
		} else
		{
			printf("usage wm_bench_smpl.exe [-b bitLen]\n");
			return 1;
		}
		argc--, argv++;
	}
	run(bitLen);
} catch (std::exception& e) {
	printf("err %s\n", e.what());
}

