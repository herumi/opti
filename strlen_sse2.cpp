/*
	fast memchr and strlen using SSE2 for gcc 4.x / Visual Studio 2008(32bit/64bit)

	Copyright (C) 2008 MITSUNARI Shigeo at Cybozu Labs, Inc.
	license:new BSD license
	2008/7/16 fix overrun in strlenSSE2
	2011/6/23 add strlenSSE42

	How to compile this file (require Xbyak)
	g++ search.cpp -O3 -fomit-frame-pointer -msse4 -fno-operator-names


Core2Duo 1.8GHz Xp SP3 + Visual Studio 2008
ave             2      5      7     10     12     16     20     32     64    128    256    512   1024
strlenANSI  683.8  406.3  308.8  234.5  203.3  168.0  148.3  113.3   86.0   70.5   62.5   58.5   54.5
strlenBLOG  835.8  449.3  347.5  269.5  234.3  195.3  175.8  140.5  109.3   89.8   82.0   78.3   82.3
strlenSSE2  765.8  355.5  269.5  199.3  172.0  133.0  109.5   78.3   47.0   27.3   19.8   15.5    7.8
memchrANSI 1046.8  648.5  515.8  390.5  347.8  273.3  226.5  164.0  105.5   74.3   62.5   50.8   50.8
memchrSSE2  773.5  375.0  285.0  214.8  179.5  144.8  121.3   82.0   54.5   31.3   19.5   15.8   11.8

Core2Duo 1.8GHz Linux 2.4 on VMware + gcc 4.3.0
ave             2      5      7     10     12     16     20     32     64    128    256    512   1024
strlenANSI 2019.6  933.9  728.7  576.8  512.3  430.5  386.6  316.5  261.2  235.6  219.4  214.0  212.9
strlenBLOG  692.6  397.4  331.0  242.5  216.7  194.3  152.2  124.7  110.3   81.5   76.3   82.2   70.0
strlenSSE2  560.0  275.6  214.3  159.2  135.7  104.4   87.5   65.1   41.5   25.3   16.6   14.0    9.3
memchrANSI 1152.4  609.5  487.4  375.0  325.6  260.5  229.9  152.4   95.5   72.8   56.8   49.3   48.0
memchrSSE2  574.6  282.1  224.3  161.0  139.1  108.5   90.0   63.3   45.1   23.9   15.8   10.0   11.3

Core2Duo 1.8GHz Linux 2.6.18-53 on VMware + gcc 4.1.2(64bit)
ave             2      5      7     10     12     16     20     32     64    128    256    512   1024
strlenANSI 1039.4  568.0  460.9  353.7  306.9  254.9  212.2  145.2   82.0   50.3   35.3   26.4   24.5
strlenBLOG  795.4  474.9  366.4  291.0  263.9  227.6  201.7  178.3  144.5  125.9  117.2  112.0  110.5
strlenSSE2  703.0  340.0  267.6  196.3  167.2  131.6  108.9   78.7   47.4   30.2   20.4   15.0   14.5
memchrANSI 1280.7  736.4  594.2  472.2  423.8  338.0  294.3  211.0  127.2   90.4   67.3   55.4   55.1
memchrSSE2 1001.4  456.3  336.4  241.3  206.4  159.5  138.5   93.0   57.5   35.6   23.3   17.0   15.7

Core2Duo 1.8GHz Linux 2.6.18-53 on VMware + gcc 4.1.2(32bit)
ave             2      5      7     10     12     16     20     32     64    128    256    512   1024
strlenANSI 1053.7  593.8  476.0  353.7  304.8  254.3  208.3  150.0   87.9   62.0   51.7   44.4   41.5
strlenBLOG  704.1  418.5  336.1  259.0  227.6  197.6  175.7  146.0  115.7   98.2   93.7   85.4   82.2
strlenSSE2  702.3  347.2  275.6  198.4  176.6  133.5  112.3   80.0   48.3   30.3   20.2   13.6   12.0
memchrANSI 1170.5  627.4  503.1  382.1  335.9  270.2  227.0  163.6  105.0   74.4   63.5   52.9   49.8
memchrSSE2 1012.4  513.0  398.1  291.7  247.8  195.2  161.8  112.4   70.9   44.9   28.9   19.6   17.3

CoreDuo 1.8GHz Linux 2.4 gcc 4.3.0
ave             2      5      7     10     12     16     20     32     64    128    256    512   1024
strlenANSI 2547.5 1150.0  905.0  695.0  612.5  517.5  460.0  380.0  307.5  277.5  257.5  247.5  245.0
strlenBLOG  827.5  500.0  402.5  312.5  277.5  240.0  215.0  182.5  152.5  135.0  130.0  125.0  125.0
strlenSSE2  687.5  337.5  275.0  205.0  177.5  140.0  120.0   87.5   57.5   35.0   25.0   20.0   17.5
memchrANSI 1390.0  750.0  605.0  455.0  400.0  322.5  275.0  202.5  137.5  107.5   92.5   82.5   80.0
memchrSSE2  805.0  380.0  295.0  220.0  192.5  152.5  130.0   92.5   62.5   42.5   25.0   22.5   20.0

Core2Duo 2.6GHz + WinXp 64bit on Vmware + VC2008(32bit)
ave             2      5      7     10     12     16     20     32     64    128    256    512   1024
strlenANSI  503.8  289.0  222.8  168.0  144.8  121.0  109.3   86.0   62.5   50.8   47.0   43.0   43.0
strlenBLOG  636.8  339.8  254.0  203.2  175.7  148.5  133.0  105.5   82.0   66.5   62.5   58.5   54.7
strlenSSE2  566.2  269.5  203.2  144.5  125.0  101.5   97.5   58.5   35.2   19.5   11.7   11.8   11.8
memchrANSI  808.7  492.3  390.5  297.0  257.7  203.3  172.0  125.0   78.0   54.5   47.0   39.2   39.0
memchrSSE2  574.3  289.0  218.8  156.3  136.8  105.5   89.7   62.5   35.2   23.5   11.7   11.5    7.7

Core2Duo 2.6GHz + WinXp 64bit on Vmware + VC2008(64bit)
ave             2      5      7     10     12     16     20     32     64    128    256    512   1024
strlenANSI 1195.3  613.5  504.0  422.0  394.8  351.5  328.2  289.0  257.7  242.0  238.3  230.2  230.5
strlenBLOG  574.3  316.3  246.2  195.3  171.7  144.5  125.0  105.5   82.0   62.5   58.5   58.7   54.8
strlenSSE2  503.7  242.3  183.7  136.7  117.3   89.8   78.0   54.8   31.3   19.5   11.8   11.8    7.7
memchrANSI  629.0  339.7  277.3  203.0  179.8  152.5  140.7  117.3   97.7   86.0   82.0   78.0   78.0
memchrSSE2  515.5  254.0  195.2  144.5  125.0   97.5   82.0   58.5   35.2   23.5   11.8    7.7    7.8

Opteraon 240 EE 1.4GHz + Linux 2.6.9 + gcc 3.4.6(32bit)
ave             2      5      7     10     12     16     20     32     64    128    256    512   1024
strlenANSI 1194.7  738.9  525.7  392.0  338.9  361.7  230.3  173.8  118.7   93.2   80.2   72.5   70.5
strlenBLOG  911.7  499.0  404.2  325.2  297.6  261.9  242.0  228.6  186.1  173.3  167.9  164.1  162.6
strlenSSE2 1465.2  679.2  516.5  372.6  319.6  249.4  206.0  152.8   87.9   54.9   38.2   29.1   26.5
memchrANSI 1308.9  724.6  565.5  429.1  373.8  299.6  254.6  198.2  134.2  107.9   92.0   85.2   83.9
memchrSSE2 2634.2  870.9  665.3  488.4  413.1  318.4  267.1  194.1  115.0   72.9   50.6   39.0   36.0

Opteraon 240 EE 1.4GHz + Linux 2.6.9 + gcc 3.4.6(64bit)
ave             2      5      7     10     12     16     20     32     64    128    256    512   1024
strlenANSI 1261.2  700.0  569.9  449.0  398.5  323.3  272.2  191.5  116.8   79.7   59.5   48.1   45.1
strlenBLOG  924.5  508.8  412.4  331.0  303.0  266.0  244.8  214.3  187.3  173.8  167.0  163.3  162.2
strlenSSE2 1292.4  602.2  465.7  341.3  292.8  233.5  194.1  140.5   87.7   57.6   41.2   31.9   29.3
memchrANSI 1383.0  801.0  667.0  539.4  483.2  399.2  339.5  244.0  152.7  105.9   80.4   65.9   61.8
memchrSSE2 1671.9  750.6  570.6  409.6  348.4  270.7  224.4  158.0   97.6   61.7   42.7   32.9   30.1

Xeon X5650 2.67GHz + Linux 2.6.32 + gcc 4.6(64bit)
ave             2      5      7     10     12     16     20     32     64    128    256    512   1024
strlenANSI   503.6  202.1  160.4  124.1  110.9   90.9   78.7   55.8   33.1   21.0   14.1   10.4    9.4
strlenSSE2   532.1  254.0  200.7  149.1  129.4  103.2   86.3   60.9   36.6   22.3   13.7    9.1    7.9
strlenSSE42  424.0  176.5  139.9  112.6  102.2   88.2   78.7   57.0   33.0   21.3   13.6    9.6    8.3
memchrANSI   411.6  203.4  163.0  127.3  113.9   94.6   81.1   57.0   33.1   20.1   13.0    9.2    8.2
memchrSSE2   493.8  245.5  192.2  141.4  120.0   94.0   78.9   55.4   33.2   20.0   13.1    9.1    8.0

Core i7-2600 3.40GHz + Linux 2.6.35 + gcc 4.4.5(64bit)
ave             2      5      7     10     12     16     20     32     64    128    256    512   1024
strlenANSI   382.5  180.2  143.3  106.1   91.3   71.1   59.8   37.9   18.3   12.1    8.5    6.7    6.2
strlenSSE2   383.3  177.8  139.4  102.4   88.3   66.6   53.9   33.4   16.9    9.7    6.7    5.1    4.8
strlenSSE42  396.7  163.5  127.0  100.2   89.3   76.7   66.9   42.2   21.4   12.7    9.0    7.1    6.4
memchrANSI   324.2  153.8  127.3   93.4   80.6   63.1   52.5   34.3   18.9   10.6    7.8    6.2    5.5
memchrSSE2   341.7  159.5  123.7   91.0   75.7   58.0   47.3   33.4   18.6    9.8    6.9    5.4    4.8

Core i7-2600 3.40GHz + Windows 7 + VC10(64bit)
ave             2      5      7     10     12     16     20     32     64    128    256    512   1024
strlenANSI   768.3  343.2  261.2  199.0  175.5  148.3  136.5  101.2   78.0   66.3   58.5   58.5   58.5
strlenSSE2   347.0  159.8  121.0   89.7   78.0   62.3   50.7   31.3   15.5    7.7    8.0    4.0    3.8
strlenSSE42  390.0  163.7  128.8  101.3   93.5   78.0   74.0   43.0   19.5   11.8    7.7    7.7    4.0
memchrANSI   421.3  206.8  167.5  140.5  132.5  121.0  109.3   93.5   74.3   66.3   78.0   58.5   58.5
memchrSSE2   343.3  167.7  128.7   89.8   82.0   58.5   50.7   31.3   19.5    7.7    3.8    4.0    4.0

Core i7-2600 3.40GHz + Windows 7 + VC10(32bit)
ave             2      5      7     10     12     16     20     32     64    128    256    512   1024
strlenANSI   394.0  191.3  152.3  120.8  109.0   93.5   85.8   70.2   54.8   47.0   46.8   42.7   39.0
strlenSSE2   413.3  198.7  152.0  113.2   97.5   74.3   62.2   39.0   19.5    7.7    4.0    4.0    3.7
strlenSSE42  433.0  179.5  136.5  109.0   93.8   81.7   70.2   46.8   23.2   11.8   11.8    7.7    8.0
memchrANSI   542.0  304.3  245.7  179.5  156.0  121.0  101.5   66.3   39.0   31.3   23.2   23.5   23.2
memchrSSE2   370.5  175.5  136.5  101.5   85.8   62.5   54.5   35.0   19.5    7.7    8.0    7.7    4.0
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <vector>
#include <xbyak/xbyak.h>

#ifdef _WIN32
	#include <intrin.h>
	#define ALIGN(x) __declspec(align(x))
	#define bsf(x) (_BitScanForward(&x, x), x)
	#define bsr(x) (_BitScanReverse(&x, x), x)
#else
	#include <xmmintrin.h>
	#define ALIGN(x) __attribute__((aligned(x)))
	#define bsf(x) __builtin_ctz(x)
#endif

void *memchrSSE2(const void *ptr, int c, size_t len)
{
	const char *p = reinterpret_cast<const char*>(ptr);
	if (len >= 16) {
		__m128i c16 = _mm_set1_epi8(static_cast<char>(c));
		/* 16 byte alignment */
		size_t ip = reinterpret_cast<size_t>(p);
		size_t n = ip & 15;
		if (n > 0) {
			ip &= ~15;
			__m128i x = *(const __m128i*)ip;
			__m128i a = _mm_cmpeq_epi8(x, c16);
			unsigned long mask = _mm_movemask_epi8(a);
			mask &= 0xffffffffUL << n;
			if (mask) {
				return (void*)(ip + bsf(mask));
			}
			n = 16 - n;
			len -= n;
			p += n;
		}
		while (len >= 32) {
			__m128i x = *(const __m128i*)&p[0];
			__m128i y = *(const __m128i*)&p[16];
			__m128i a = _mm_cmpeq_epi8(x, c16);
			__m128i b = _mm_cmpeq_epi8(y, c16);
			unsigned long mask = (_mm_movemask_epi8(b) << 16) | _mm_movemask_epi8(a);
			if (mask) {
				return (void*)(p + bsf(mask));
			}
			len -= 32;
			p += 32;
		}
	}
	while (len > 0) {
		if (*p == c) return (void*)p;
		p++;
		len--;
	}
	return 0;
}

size_t strlenSSE2(const char *p)
{
	const char *const top = p;
	__m128i c16 = _mm_set1_epi8(0);
	/* 16 byte alignment */
	size_t ip = reinterpret_cast<size_t>(p);
	size_t n = ip & 15;
	if (n > 0) {
		ip &= ~15;
		__m128i x = *(const __m128i*)ip;
		__m128i a = _mm_cmpeq_epi8(x, c16);
		unsigned long mask = _mm_movemask_epi8(a);
		mask &= 0xffffffffUL << n;
		if (mask) {
			return bsf(mask) - n;
		}
		p += 16 - n;
	}
	/*
		thanks to egtra-san
	*/
	assert((reinterpret_cast<size_t>(p) & 15) == 0);
	if (reinterpret_cast<size_t>(p) & 31) {
		__m128i x = *(const __m128i*)&p[0];
		__m128i a = _mm_cmpeq_epi8(x, c16);
		unsigned long mask = _mm_movemask_epi8(a);
		if (mask) {
			return p + bsf(mask) - top;
		}
		p += 16;
	}
	assert((reinterpret_cast<size_t>(p) & 31) == 0);
	for (;;) {
		__m128i x = *(const __m128i*)&p[0];
		__m128i y = *(const __m128i*)&p[16];
		__m128i a = _mm_cmpeq_epi8(x, c16);
		__m128i b = _mm_cmpeq_epi8(y, c16);
		unsigned long mask = (_mm_movemask_epi8(b) << 16) | _mm_movemask_epi8(a);
		if (mask) {
			return p + bsf(mask) - top;
		}
		p += 32;
	}
}

#ifdef __GNUC__
#include <sys/time.h>
#include <stdio.h>

static inline double gettimeofday_sec()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + (double) tv.tv_usec * 1e-6;
}
#else
#include <time.h>
static inline double gettimeofday_sec()
{
	return clock() / double(CLOCKS_PER_SEC);
}
#endif

struct Result {
	int hit;
	int ret;
	double time;
	Result() {}
	Result(int hit, int ret, double time) : hit(hit), ret(ret), time(time) {}
	void put() const
	{
		printf("ret=%d(%.1f) time= %f usec\n", ret, ret / double(hit), time);
	}
};

void createTable(char *p, size_t num, int ave)
{
	int v = 0;
	for (size_t i = 0; i < num; i++) {
		v = 1;
		p[i] = static_cast<char>(v);
		if ((rand() % ave) == 0) p[i] = 0;
	}
	p[num - 1] = 0;
}

template<typename Func>
Result test(const char *top, size_t n, size_t count)
{
	double begin = gettimeofday_sec();
	size_t ret = 0;
	int hit = 0;
	for (size_t i = 0; i < count; i++) {
		const char *p = top;
		int remain = n;
		while (remain > 0) {
			const char *q = Func::find(p, remain);
#if 0
			const char *ok = FstrlenANSI::find(p, remain);
			if (q != ok) {
				printf("ok=%p, ng=%p\n",ok, q);
				exit(1);
			}
#endif
			if (q == 0) break;
			ret += q - p;
			hit++;
			remain -= q - p + 1;
			p = q + 1;
		}
	}
	return Result(hit, ret, (gettimeofday_sec() - begin) * 1e6 / count);
}

struct FstrlenANSI {
	static inline const char *find(const char *p, size_t)
	{
		return strlen(p) + p;
	}
};

size_t strlenC(const char *s)
{
	size_t len = 0;
	while (s[len]) len++;
	return len;
}

struct FstrlenC {
	static inline const char *find(const char *p, size_t)
	{
		return strlenC(p) + p;
	}
};

struct FmemchrANSI {
	static inline const char *find(const char *p, size_t n)
	{
		return reinterpret_cast<const char*>(memchr(p, 0, n));
	}
};

struct FmemchrSSE2 {
	static inline const char *find(const char *p, size_t n)
	{
		return reinterpret_cast<const char*>(memchrSSE2(p, 0, n));
	}
};

struct FstrlenSSE2 {
	static inline const char *find(const char *p, size_t)
	{
		return strlenSSE2(p) + p;
	}
};

#define NUM_OF_ARRAY(x) (sizeof(x)/sizeof(x[0]))

#ifdef _WIN32
#include <windows.h>
void test_overrun()
{
	DWORD old;
	const int size = 4096;
	char* p = (char*)VirtualAlloc(0, 8192, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	VirtualProtect(p + size, size, PAGE_NOACCESS, &old);
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			memset(p, 1, size);
			p[size - 1 - j] = 0;
			size_t a = strlenSSE2(p + i);
			size_t b = reinterpret_cast<const char*>(memchrSSE2(p + i, 0, size - i)) - (p + i);
			size_t c = size - 1 - i - j;
			if (a != c || b != c) {
				printf("i=%d, j=%d, a=%d, b=%d, c=%d\n", i, j, a, b, c);
			}
		}
	}
	puts("test overrun ok");
}
#endif


int main()
{
#ifdef _WIN32
	test_overrun();
#endif
	const size_t count = 4000;
	const size_t N = 100000;
	const int funcNum = 5;
	std::vector<char> v(N);

	typedef std::vector<Result> ResultVect;

	ResultVect rv[funcNum];

	char *begin = &v[0];

	const int aveTbl[] = { 2, 5, 7, 10, 12, 16, 20, 32, 64, 128, 256, 512, 1024 };
//	const int aveTbl[] = { 64, 128, 256, 512, 1024 };

	for (size_t i = 0; i < NUM_OF_ARRAY(aveTbl); i++) {
		int ave = aveTbl[i];
		createTable(begin, N, ave);

		printf("test %d, %d\n", (int)i, ave);
		Result ret;
		int hit;

		puts("strlenANSI");
		ret = test<FstrlenANSI>(begin, N, count);
		ret.put();
		rv[0].push_back(ret);
		hit = ret.hit;

		puts("strlenC   ");
		ret = test<FstrlenC>(begin, N, count);
		if (ret.hit != hit) { printf("ERROR!!! ok=%d, ng=%d\n", hit, ret.hit); }
		ret.put();
		rv[1].push_back(ret);

		puts("strlenSSE2");
		ret = test<FstrlenSSE2>(begin, N, count);
		if (ret.hit != hit) { printf("ERROR!!! ok=%d, ng=%d\n", hit, ret.hit); }
		ret.put();
		rv[2].push_back(ret);

		puts("memchrANSI");
		ret = test<FmemchrANSI>(begin, N, count);
		if (ret.hit != hit) { printf("ERROR!!! ok=%d, ng=%d\n", hit, ret.hit); }
		ret.put();
		rv[3].push_back(ret);

		puts("memchrSSE2");
		ret = test<FmemchrSSE2>(begin, N, count);
		if (ret.hit != hit) { printf("ERROR!!! ok=%d, ng=%d\n", hit, ret.hit); }
		ret.put();
		rv[4].push_back(ret);

	}

	puts("end");

	printf("ave        ");
	for (size_t i = 0; i < NUM_OF_ARRAY(aveTbl); i++) {
		printf("%6d ", aveTbl[i]);
	}
	printf("\n");
	static const char nameTbl[funcNum][16] = { "strlenANSI ", "strlenC    ", "strlenSSE2 ", "memchrANSI ", "memchrSSE2 " };
	for (int i = 0; i < funcNum; i++) {
		printf("%s ", nameTbl[i]);
		for (size_t j = 0; j < NUM_OF_ARRAY(aveTbl); j++) {
			printf("%6.1f ", rv[i][j].time);
		}
		printf("\n");
	}
	return 0;
}

