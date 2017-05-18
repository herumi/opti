#include <stdio.h>
#include <cybozu/bit_operation.hpp>
#include <memory.h>
#include "remove_ctrl.hpp"
#include "../../../mie_string/mie_string.h"
#ifdef _MSC_VER
	#include <intrin.h>
#else
	#include <x86intrin.h>
#endif

__m128i mie_safe_load(const void *p, size_t size)
{
	static const unsigned char shiftPtn[32] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
	};
	size_t addr = (size_t)p;
	size_t addr2 = addr & 0xfff;
	if (addr2 > 0xff0 && addr2 + size <= 0x1000) {
		addr2 = addr & ~(size_t)15;
		size_t shift = addr & 15;
		__m128i ptn = _mm_loadu_si128((const __m128i*)(shiftPtn + shift));
		return _mm_shuffle_epi8(_mm_load_si128((const __m128i*)addr2), ptn);
	} else {
		return _mm_loadu_si128((const __m128i*)p);
	}
}

// return p + size if not found
inline const char *mie_findCharRangeCopy(char *dst, const char *p, size_t size, const char *key, size_t keySize)
{
	const __m128i r = _mm_loadu_si128((const __m128i*)key);
	__m128i v;
	int c;
	if (size >= 16) {
		size_t left = size & ~15;
		do {
			v = _mm_loadu_si128((const __m128i*)p);
			c = _mm_cmpestri(r, (int)keySize, v, (int)left, 4);
			_mm_storeu_si128((__m128i*)dst, v);
			if (c != 16) {
				p += c;
				return p;
			}
			p += 16;
			left -= 16;
			dst += 16;
		} while (left != 0);
	}
	size &= 15;
	if (size == 0) return p;
	v = mie_safe_load(p, size);
	_mm_storeu_si128((__m128i*)dst, v);
	c = _mm_cmpestri(r, (int)keySize, v, (int)size, 4);
	return p + c;
}

// return p + size if not found
inline const char *findCharRangeCopyAVX(char *dst, const char *p, size_t size)
{
	const int c[] = { 0x20202020, 0x20202020, 0x20202020, 0x20202020, 0x20202020, 0x20202020, 0x20202020, 0x20202020 };
#if 0
	const __m256i max = _mm256_loadu_si256((const __m256i*)c);
	while (size >= 32) {
		__m256i v = _mm256_loadu_si256((const __m256i*)p);
		_mm256_storeu_si256((__m256i*)dst, v);
		uint32_t mask = _mm256_movemask_epi8(_mm256_cmpgt_epi8(max, v));
		uint32_t idx = _tzcnt_u32(mask);
		if (idx < 32) {
			return p + idx;
		}
		dst += 16;
		p += 16;
		size -= 16;
	}
#else
	const __m256i max = _mm256_loadu_si256((const __m256i*)c);
	while (size >= 64) {
		__m256i v0 = _mm256_loadu_si256((const __m256i*)p);
		__m256i v1 = _mm256_loadu_si256((const __m256i*)(p + 32));
		_mm256_storeu_si256((__m256i*)dst, v0);
		_mm256_storeu_si256((__m256i*)(dst + 32), v1);
		uint64_t mask0 = _mm256_movemask_epi8(_mm256_cmpgt_epi8(max, v0));
		uint64_t mask1 = _mm256_movemask_epi8(_mm256_cmpgt_epi8(max, v1));
		uint64_t idx = _tzcnt_u64(mask0 | (mask1 << 32));
		if (idx < 64) {
			return p + idx;
		}
		dst += 32;
		p += 32;
		size -= 32;
	}
#endif
	while (size > 0) {
		char c = *p;
		if (c >= 0x20) {
			*dst++ = c;
		} else {
			return p;
		}
		p++;
		size--;
	}
	return p;
}

size_t removeCtrlOrg(char *dst, const char *src, size_t size)
{
	const char *const begin = dst;
	for (size_t i = 0; i < size; i++) {
		if (src[i] >= 0x20) {
			*dst++ = src[i];
		}
	}
	return dst - begin;
}

size_t removeCtrlSearch(char *dst, const char *src, size_t size)
{
	const char *const begin = dst;
	while (size > 0) {
		size_t i = 0;
		while (src[i] >= 0x20 && i < size) {
			i++;
		}
		memcpy(dst, src, i);
		dst += i;
		src += i;
		size -= i;
		i = 0;
		while (*src < 0x20 && size > 0) {
			src++;
			size--;
		}
	}
	return dst - begin;
}

size_t removeCtrlCmpestri(char *dst, const char *src, size_t size)
{
	const char *const begin = dst;
	while (size > 0) {
		size_t i = 0;
		const char *next = mie_findCharRange(src, size, "\x00\x1f\x80\xff", 4);
		if (next == 0) {
			i = size;
		} else {
			i = next - src;
		}
		memcpy(dst, src, i);
		dst += i;
		src += i;
		size -= i;
		i = 0;
		while (*src < 0x20 && size > 0) {
			src++;
			size--;
		}
	}
	return dst - begin;
}

// reserve size + 16 byte buffer for dst
size_t removeCtrlCmpestriCopy(char *dst, const char *src, size_t size)
{
	const char *const begin = dst;
	while (size > 0) {
		size_t i = 0;
		const char *next = mie_findCharRangeCopy(dst, src, size, "\x00\x1f\x80\xff", 4);
		i = next - src;
		dst += i;
		size -= i;
		src = next;
		while (*src < 0x20 && size > 0) {
			src++;
			size--;
		}
	}
	return dst - begin;
}

size_t removeCtrlAVX(char *dst, const char *src, size_t size)
{
	const char *const begin = dst;
	while (size > 0) {
		size_t i = 0;
		const char *next = findCharRangeCopyAVX(dst, src, size);
		i = next - src;
		dst += i;
		size -= i;
		src = next;
		while (*src < 0x20 && size > 0) {
			src++;
			size--;
		}
	}
	return dst - begin;
}
