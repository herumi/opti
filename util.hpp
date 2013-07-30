#pragma once

#include <algorithm>
#include <string>
#include <fstream>

#if !defined(_MSC_VER) || (_MSC_VER >= 1600)
#include <stdint.h>
#else
typedef unsigned __int64 uint64_t;
typedef __int64 int64_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
#endif

#ifndef MIE_ALIGN
	#ifdef _MSC_VER
		#define MIE_ALIGN(x) __declspec(align(x))
	#else
		#define MIE_ALIGN(x) __attribute__((aligned(x)))
	#endif
#endif
#ifdef _WIN32
//	#include <winsock2.h>
	#include <intrin.h>
	#define my_bsf(x) (_BitScanForward(&x, x), x)
//	#define my_bsr(x) (_BitScanReverse(&x, x), x)
#else
	#ifdef __linux__
		#include <x86intrin.h>
	#else
		#include <emmintrin.h>
	#endif
	#define my_bsf(x) __builtin_ctz(x)
#endif

#ifdef _MSC_VER
#include <malloc.h>
#else
#include <stdlib.h>
static inline void *_aligned_malloc(size_t size, size_t alignment)
{
	void *p;
	int ret = posix_memalign(&p, alignment, size);
	return (ret == 0) ? p : 0;
}
static inline void _aligned_free(void *p)
{
	free(p);
}
#endif

#define NUM_OF_ARRAY(x) (sizeof(x)/sizeof(*x))

struct XorShift128 {
	uint32_t x;
	uint32_t y;
	uint32_t z;
	uint32_t w;
	XorShift128()
		: x(123456789)
		, y(362436069)
		, z(521288629)
		, w(88675123)
	{
	}
	uint32_t get()
	{
		uint32_t t;
		t = x ^ (x << 11);
		x = y;
		y = z;
		z = w;
		w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
		return w;
	}
	uint64_t get64()
	{
		uint64_t x = get();
		uint64_t y = get();
		return (y << 32) | x;
	}
};

template<class T, size_t alignedByte = 64>
struct AlignedArray {
	T *p_;
	size_t n_;
	AlignedArray(size_t n = 0)
		: p_(0)
		, n_(n)
	{
		resize(n);
	}
	~AlignedArray()
	{
		_aligned_free(p_);
	}
	const T& operator[](size_t n) const { return p_[n]; }
	T& operator[](size_t n) { return p_[n]; }
	void resize(size_t n)
	{
		_aligned_free(p_);
		p_ = (T*)_aligned_malloc(n * sizeof(T) + alignedByte, alignedByte);
		n_ = n;
	}
	size_t size() const { return n_; }
	const T* begin() const { return p_; }
	const T* end() const { return p_ + n_; }
	typedef T value_type;
	void swap(AlignedArray& rhs) throw()
	{
		std::swap(p_, rhs.p_);
		std::swap(n_, rhs.n_);
	}
private:
	AlignedArray(const AlignedArray&);
	void operator=(const AlignedArray&);
};

/*
	read text data from fileName
*/
inline bool LoadFile(AlignedArray<char>& textBuf, const std::string& fileName)
{
	std::ifstream ifs(fileName.c_str(), std::ios::binary);
	if (!ifs) return false;
	ifs.seekg(0, std::ifstream::end);
	const uint64_t orgSize = ifs.tellg();
	ifs.seekg(0);
	if (orgSize > 0x7ffffffe) {
		fprintf(stderr, "file is too large\n");
		return false;
	}
	const int size = (int)orgSize;
	fprintf(stderr, "size=%d\n", size);
	textBuf.resize(size + 1);
	ifs.read(&textBuf[0], size);
	textBuf[size] = '\0';
	return true;
}

inline std::string LoadFile(const std::string& fileName)
{
	std::ifstream ifs(fileName.c_str(), std::ios::binary);
	std::string str(std::istreambuf_iterator<char>(ifs.rdbuf()), std::istreambuf_iterator<char>());
	for (size_t i = 0; i < str.size(); i++) {
		char c = str[i];
		if (c == '\0') c = ' ';
	}
	printf("file=%s, size=%d\n", fileName.c_str(), (int)str.size());
	return str;
}
