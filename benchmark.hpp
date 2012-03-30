#pragma once

#include <string>
#include <vector>

#define TEST_EQUAL(a, b) { if ((a) != (b)) { fprintf(stderr, "%s:%d err a=%lld, b=%lld\n", __FILE__, __LINE__, (long long)(a), (long long)(b)); exit(1); } }

typedef std::vector<std::string> StrVec;

struct Ret {
	int val;
	double clk;
	Ret() : val(0), clk(0) {}
	bool operator!=(const Ret& rhs) const { return val != rhs.val; }
	operator long long() const { return val; }
};

template<const char* (*f)(const char*str, const char *key)>
struct Fstrstr {
	const char *str_;
	const char *key_;
	typedef const char* type;
	void set(const std::string& str, const std::string& key)
	{
		str_ = &str[0];
		key_ = &key[0];
	}
	Fstrstr() : str_(0), key_(0) { }
	const char *begin() const { return str_; }
	const char *end() const { return 0; }
	const char *find(const char *p) const { return f(p, key_); }
};

template<const char* (*f)(const char*str, int c)>
struct Fstrchr {
	const char *str_;
	int c_;
	typedef const char* type;
	void set(const std::string& str, const std::string& key)
	{
		str_ = &str[0];
		c_ = key[0];
	}
	Fstrchr() : str_(0), c_(0) { }
	const char *begin() const { return str_; }
	const char *end() const { return 0; }
	const char *find(const char *p) const { return f(p, c_); }
};

template<const char* (*f)(const char*begin, const char *end, const char *key, size_t size)>
struct Frange {
	const char *str_;
	const char *end_;
	const char *key_;
	size_t keySize_;
	typedef const char* type;
	void set(const std::string& str, const std::string& key)
	{
		str_ = &str[0];
		end_ = str_ + str.size();
		key_ = &key[0];
		keySize_ = key.size();
	}
	Frange() : str_(0), end_(0), key_(0), keySize_(0) { }
	const char *begin() const { return str_; }
	const char *end() const { return end_; }
	const char *find(const char *p) const { return f(p, end_, key_, keySize_); }
};

template<const char* (*f)(const char*begin, const char *end, char c)>
struct Frange_char {
	const char *str_;
	const char *end_;
	char c_;
	size_t keySize_;
	typedef const char* type;
	void set(const std::string& str, const std::string& key)
	{
		str_ = &str[0];
		end_ = str_ + str.size();
		c_ = key[0];
	}
	Frange_char() : str_(0), end_(0), c_(0) { }
	const char *begin() const { return str_; }
	const char *end() const { return end_; }
	const char *find(const char *p) const { return f(p, end_, c_); }
};

template<class F>
Ret benchmark1(F f, const std::string& str, const std::string& key)
{
	const int N = 10;
	int val = 0;
	f.set(str, key);
	Xbyak::util::Clock clk;
	for (int i = 0; i < N; i++) {
		typename F::type p = f.begin();
		typename F::type end = f.end();
		for (;;) {
			clk.begin();
			typename F::type q = f.find(p);
			clk.end();
			if (q == end) break;
			val += (int)(q - p);
			p = q + 1;
		}
	}
	if (val == 0) val = (int)(str.size()) * N;
	Ret ret;
	ret.val = val;
	ret.clk = clk.getClock() / (double)val;
	return ret;
}

template<class F1, class F2>
void benchmark(const char *msg1, F1 f1, const char *msg2, F2 f2, const std::string& str, const std::string& key)
{
	Ret r1 = benchmark1(f1, str, key);
	Ret r2 = benchmark1(f2, str, key);
	printf("%25s %16s % 6.2f %16s % 6.2f %5.2f\n", key.substr(0, 25).c_str(), msg1, r1.clk, msg2, r2.clk, r1.clk / r2.clk);
	TEST_EQUAL(r1, r2);
}

template<class F1, class F2>
void benchmarkTbl(const char *msg1, F1 f1, const char *msg2, F2 f2, const std::string& str, const StrVec& keyTbl)
{
	for (size_t i = 0; i < keyTbl.size(); i++) {
		benchmark(msg1, f1, msg2, f2, str, keyTbl[i]);
	}
}

// dirty hack for gcc 4.2 on mac
#if defined(__APPLE__) || defined(__clang__)
	#define STRSTR mystrstr
	#define STRCHR mystrchr
inline const char *mystrstr(const char*text, const char *key)
{
	return strstr(text, key);
}
inline const char *mystrchr(const char*text, int c)
{
	return strchr(text, c);
}

#else
	#define STRSTR strstr
	#define STRCHR strchr
#endif
