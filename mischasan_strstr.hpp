#pragma once
/*
	see http://mischasan.wordpress.com/2011/07/16/convergence-sse2-and-strstr/
	fixed version at 2012/Apr/5
*/
#include "util.hpp"
#include <string.h>
#ifdef _WIN32
	inline int ffs(int x)
	{
		DWORD ret;
		if (_BitScanForward(&ret, x)) {
			return ret + 1;
		} else {
			return 0;
		}
	}
#else
	#define ffs(x) __builtin_ffs(x) // bsf ; 0 if x == 0
#endif

typedef __m128i XMM;
static inline unsigned under(unsigned x){ return (x - 1) & ~x; }
static inline XMM xmfill(char b) { return _mm_set1_epi8(b); }
static inline XMM xmload(void const *p) { return _mm_load_si128((XMM const *) p); }
static inline XMM xmloud(void const *p) { return (XMM) _mm_loadu_pd((double const *) p); }
static inline unsigned xmatch(XMM a, XMM b) { return _mm_movemask_epi8(_mm_cmpeq_epi8(a, b)); }
static inline unsigned xmdiff(XMM a, XMM b) { return xmatch(a, b) ^ 0xFFFF; }

char const *scanstrN(char const *tgt, char const *pat, size_t patlen)
{
	if (!pat[0]) return tgt;
	if (!pat[1]) return strchr(tgt, *pat);

	XMM zero = {}, xt;
	XMM xp0 = xmfill(pat[0]);
	XMM xp1 = xmfill(pat[1]);
	unsigned m = 15 & (intptr_t) tgt;
	unsigned mz = (-1 << m) & xmatch(zero, xt = xmload(tgt -= m));
	unsigned m0 = (-1 << m) & xmatch(xp0, xt);
	char const *p;

	while (!mz) {
		if (m0) {
			unsigned m1 = xmatch(xp1, xt);
			m0 &= (m1 >> 1) | (tgt[16] == pat[1] ? 0x8000 : 0);
			for (m = m0; m; m &= m - 1) {
				int pos = ffs(m) - 1;
				if (!memcmp(pat + 2, tgt + pos + 2, patlen - 2))
					return tgt + pos;
			}
		}
		mz = xmatch(zero, xt = xmload(tgt += 16));
		m0 = xmatch(xp0, xt);
	}

	if ((m0 &= under(mz))) {
		m0 &= (xmatch(xp1, xt) >> 1);
		for (m = m0; m; m &= m - 1)
//			if (!intcmp(pat + 2, 2 + (p = tgt + ffs(m) - 1), patlen - 2))
			if (!memcmp(pat + 2, 2 + (p = tgt + ffs(m) - 1), patlen - 2))
				return p;
	}
	return NULL;
}
struct Fmischasan_strstr {
	const char *str_;
	const char *key_;
	size_t keySize_;
	typedef const char* type;
	void set(const std::string& str, const std::string& key)
	{
		str_ = &str[0];
		key_ = &key[0];
		keySize_ = key.size();
	}
	Fmischasan_strstr() : str_(0), key_(0), keySize_(0) { }
	const char *begin() const { return str_; }
	const char *end() const { return 0; }
	const char *find(const char *p) const { return scanstrN(p, key_, keySize_); }
};

