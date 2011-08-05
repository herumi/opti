#pragma once
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
};

template<class T>
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
		p_ = (T*)_aligned_malloc(n * sizeof(T), 16);
		n_ = n;
	}
	size_t size() const { return n_; }
	const T* begin() const { return p_; }
	const T* end() const { return p_ + n_; }
	typedef T value_type;
private:
	AlignedArray(const AlignedArray&);
	void operator=(const AlignedArray&);
};

#define NUM_OF_ARRAY(x) (sizeof(x)/sizeof(*x))
