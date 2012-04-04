#pragma once
/*
	see http://mischasan.wordpress.com/2011/07/16/convergence-sse2-and-strstr/
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

#define compxm(a,b) _mm_movemask_epi8(_mm_cmpeq_epi8((a), (b)))

#define xmload(p)   _mm_load_si128((__m128i const *)(p))
#define load16(p)   (*(uint16_t const*)(p))
#define load32(p)   (*(uint32_t const*)(p))

char const*scanstr2(char const *tgt, char const pat[2])
{
    __m128i const   zero = _mm_setzero_si128();
    __m128i const   p0   = _mm_set1_epi8(pat[0]);
    __m128i const   p1   = _mm_set1_epi8(pat[1]);
    unsigned        f    = 15 & (intptr_t)tgt;
    if (f) {
        __m128i  x = xmload(tgt - f);
        unsigned u = compxm(zero, x);
        unsigned v = ((compxm(p0, x) & (compxm(p1, x) >> 1)) & ~u & (u - 1)) >> f;
        if (v) return tgt + ffs(v) - 1;
        if (u >> f) return  NULL;
        tgt += 16 - f;
    }
    uint16_t    pair = load16(pat);
    while (1) {
        __m128i  x = xmload(tgt);
        unsigned u = compxm(zero, x);
        unsigned v = compxm(p0, x) & (compxm(p1, x) >> 1) & ~u & (u - 1);
        if (v) return tgt + ffs(v) - 1;
        if (u) return  NULL;
        tgt += 16;
        if (load16(tgt - 1) == pair)
            return tgt -1;
    }
}

char const *scanstr3(char const *tgt, char const pat[3])
{
    __m128i const   zero = _mm_setzero_si128();
    __m128i const   p0   = _mm_set1_epi8(pat[0]);
    __m128i const   p1   = _mm_set1_epi8(pat[1]);
    __m128i const   p2   = _mm_set1_epi8(pat[2]);
    unsigned        trio = load32(pat) & 0x00FFFFFF;
    unsigned        f    = 15 & (uintptr_t)tgt;

    if (f) {
        __m128i  x = xmload(tgt);
        unsigned u = compxm(zero, x);
        unsigned v = compxm(p0, x) & (compxm(p1, x) >> 1);
        v = (v & (compxm(p2, x) >> 2) & ~u & (u - 1)) >> f;
        if (v) return tgt + ffs(v) - 1;
        tgt += 16 - f;
        v = load32(tgt - 2);
        if (trio == (v & 0x00FFFFFF)) return tgt - 2;
        if (trio ==  v >> 8         ) return tgt - 1;
        if (u >> f) return  NULL;
    }

    while (1) {
        __m128i  x = xmload(tgt);
        unsigned u = compxm(zero, x);
        unsigned v = compxm(p0, x) & (compxm(p1, x) >> 1);
        v = (v & (compxm(p2, x) >> 2) & ~u & (u - 1)) >> f;
        if (v) return tgt + ffs(v) - 1;
        tgt += 16;
        v = load32(tgt - 2);
        if (trio == (v & 0x00FFFFFF)) return tgt - 2;
        if (trio ==  v >> 8         ) return tgt - 1;
        if (u) return  NULL;
    }
}

char const *scanstrN(char const *tgt, char const *pat, int len)
{
	if (len == 1) return strchr(tgt, *pat); // add by herumi
    for (; (tgt = scanstr2(tgt, pat)); tgt++)
        if (!memcmp(tgt+2, pat+2, len-2))
            return tgt;
        tgt++;
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
