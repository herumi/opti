#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "str_util.hpp"

#define NUM_OF_ARRAY(x) (sizeof(x)/sizeof(*x))
#define TEST_EQUAL(a, b) { if ((a) != (b)) { fprintf(stderr, "%s:%d err a=%lld, b=%lld\n", __FILE__, __LINE__, (long long)(a), (long long)(b)); exit(1); } }

typedef std::vector<std::string> StrVec;

struct Ret {
	int val;
	double clk;
	Ret() : val(0), clk(0) {}
	bool operator!=(const Ret& rhs) const { return val != rhs.val; }
	operator long long() const { return val; }
};

std::string LoadFile(const std::string& fileName)
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

// std::string.find()
struct Fstr_find {
	const std::string *str_;
	const std::string *key_;
	typedef size_t type;
	void set(const std::string& str, const std::string& key)
	{
		str_ = &str;
		key_ = &key;
	}
	Fstr_find() : str_(0), key_(0) { }
	size_t begin() const { return 0; }
	size_t end() const { return std::string::npos; }
	size_t find(size_t p) const { return str_->find(*key_, p); }
};

// std::string.find_first_of()
struct Fstr_find_first_of {
	const std::string *str_;
	const std::string *key_;
	typedef size_t type;
	void set(const std::string& str, const std::string& key)
	{
		str_ = &str;
		key_ = &key;
	}
	Fstr_find_first_of() : str_(0), key_(0) { }
	size_t begin() const { return 0; }
	size_t end() const { return std::string::npos; }
	size_t find(size_t p) const { return str_->find_first_of(*key_, p); }
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

/////////////////////////////////////////////////

const char *strchr_range_C(const char *str, const char *key)
{
	while (*str) {
		unsigned char c = (unsigned char)*str;
		for (const unsigned char *p = (const unsigned char *)key; *p; p += 2) {
			if (p[0] <= c && c <= p[1]) {
				return str;
			}
		}
		str++;
	}
	return 0;
}

const char *strchr_any_C(const char *str, const char *key)
{
	while (*str) {
		unsigned char c = (unsigned char)*str;
		for (const unsigned char *p = (const unsigned char *)key; *p; p++) {
			if (c == *p) {
				return str;
			}
		}
		str++;
	}
	return 0;
}

const char *findChar_C(const char *begin, const char *end, char c)
{
	return std::find(begin, end, c);
}

const char *findChar_any_C(const char *begin, const char *end, const char *key, size_t keySize)
{
	while (begin != end) {
		unsigned char c = (unsigned char)*begin;
		for (size_t i = 0; i < keySize; i++) {
			if (c == key[i]) {
				return begin;
			}
		}
		begin++;
	}
	return end;
}

const char *findChar_range_C(const char *begin, const char *end, const char *key, size_t keySize)
{
	while (begin != end) {
		unsigned char c = (unsigned char)*begin;
		for (size_t i = 0; i < keySize; i += 2) {
			if (key[i] <= c && c <= key[i + 1]) {
				return begin;
			}
		}
		begin++;
	}
	return end;
}

const char *findStr_C(const char *begin, const char *end, const char *key, size_t keySize)
{
	while (begin + keySize <= end) {
		if (memcmp(begin, key, keySize) == 0) {
			return begin;
		}
		begin++;
	}
	return end;
}

/////////////////////////////////////////////////
void strlen_test()
{
	puts("strlen_test");
	std::string str;
	for (int i = 0; i < 16; i++) {
		str += 'a';
		size_t a = strlen(str.c_str());
		size_t b = mie::strlen(str.c_str());
		TEST_EQUAL(a, b);
	}
	str = "0123456789abcdefghijklmn\0";
	for (int i = 0; i < 16; i++) {
		size_t a = strlen(&str[i]);
		size_t b = strlen(&str[i]);
		TEST_EQUAL(a, b);
	}
	puts("ok");
}

void strchr_test(const std::string& text)
{
	const int MaxChar = 254;
	puts("strchr_test");
	std::string str;
	str.resize(MaxChar + 1);
	for (int i = 1; i < MaxChar; i++) {
		str[i - 1] = (char)i;
	}
	str[MaxChar] = '\0';
	const std::string *pstr = text.empty() ? &str : &text;
	for (int c = '0'; c <= '9'; c++) {
		benchmark("strchr_C", Fstrchr<strchr>(), "strchr", Fstrchr<mie::strchr>(), *pstr, std::string(1, (char)c));
	}
}

void strchr_any_test(const std::string& text)
{
	puts("strchr_any_test");
	std::string str = "123a456abcdefghijklmnob123aa3vnrabcdefghijklmnopaw3nabcdevra";
	for (int i = 1; i < 256; i++) {
		str += (char)i;
	}
	str += '\0';
	const char tbl[][17] = {
		"a",
		"ab",
		"abc",
		"abcd",
		"abcde",
		"abcdef",
		"abcdefg",
		"abcdefgh",
		"abcdefghi",
		"abcdefghij",
		"abcdefghijk",
		"abcdefghijkl",
		"abcdefghijklm",
		"abcdefghijklmn",
		"abcdefghijklmno",
		"abcdefghijklmnop",
	};
	const std::string *pstr = text.empty() ? &str : &text;
	for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
		const std::string key = tbl[i];
		benchmark("strchr_any_C", Fstrstr<strchr_any_C>(), "strchr_any", Fstrstr<mie::strchr_any>(), *pstr, key);
//		benchmark("find_first_of", Fstr_find_first_of(), "strchr_any", Fstrstr<mie::strchr_any>(), *pstr, key);
	}
	puts("ok");
}

void strchr_range_test(const std::string& text)
{
	puts("strchr_range_test");
	std::string str = "123a456abcdefghijklmnob123aa3vnraw3nabcdevra";
	for (int i = 1; i < 256; i++) {
		str += (char)i;
	}
	str += '\0';
	const char tbl[][17] = {
		"aa",
		"az",
		"09",
		"az09AZ",
		"acefgixz",
		"acefgixz29",
		"acefgixz29XY",
		"acefgixz29XYAB",
		"acefgixz29XYABRU",
	};
	const std::string *pstr = text.empty() ? &str : &text;
	for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
		const std::string key = tbl[i];
		benchmark("strchr_range_C", Fstrstr<strchr_range_C>(), "strchr_range", Fstrstr<mie::strchr_range>(), *pstr, key);
	}
	puts("ok");
}

void strstr_test()
{
	puts("strstr_test");
	const int SIZE = 1024 * 1024 * 10;
	struct {
		const char *str;
		const char *key;
	} tbl[] = {
		{ "abcdefghijklmn", "fghi" },
		{ "abcdefghijklmn", "i" },
		{ "abcdefghijklmn", "ij" },
		{ "abcdefghijklmn", "abcdefghijklm" },
		{ "0123456789abcdefghijkl", "0123456789abcdefghijklm" },
		{ "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTU", "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTU@" },
	};
	for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
		std::string str;
		str.reserve(SIZE);
		const size_t len = strlen(tbl[i].str);
		for (size_t j = 0; j < SIZE / len; j++) {
			str.append(tbl[i].str, len);
		}
		std::string key = tbl[i].key;
		benchmark("strstr_C", Fstrstr<strstr>(), "strstr", Fstrstr<mie::strstr>(), str, key);
	}
}

void findChar_test(const std::string& text)
{
	puts("findChar_test");
	std::string str = "123a456abcdefghijklmnob123aa3vnraw3nabcdevra";
	str += "abcdefghijklmaskjfalksjdflaksjflakesoirua93va3vnopasdfasdfaserxdf";
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 256; i++) {
			str += (char)i;
		}
	}
	str += "abcdefghijklmn";

	const std::string *pstr = text.empty() ? &str : &text;
	for (int c = '0'; c <= '9'; c++) {
		benchmark("findChar_C", Frange_char<findChar_C>(), "findChar", Frange_char<mie::findChar>(), *pstr, std::string(1, (char)c));
	}
}

void findChar_any_test(const std::string& text)
{
	puts("findChar_any_test");
	std::string str = "123a456abcdefghijklmnob123aa3vnraw3nXbcdevra";
	str += "abcdefghijklmaskjfalksjdflaksjflakesoiruXa93va3vnopasdfasdfaserxdf";
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 256; i++) {
			str += (char)i;
		}
	}
	str += "abcdefghYjklmn";
	for (int i = 0; i < 3; i++) {
		str += str;
	}

	const char tbl[][17] = {
		"z",
		"XYZ",
		"ax035ZU",
		"0123456789abcdef"
	};

	const std::string *pstr = text.empty() ? &str : &text;
	for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
		const std::string key = tbl[i];
		benchmark("findChar_any_C", Frange<findChar_any_C>(), "findChar_any", Frange<mie::findChar_any>(), *pstr, key);
	}
	puts("ok");
}

void findChar_range_test(const std::string& text)
{
	puts("findChar_range_test");
	std::string str = "123a456abcdefghijklmnob123aa3vnraw3nXbcdevra";
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 256; i++) {
			str += (char)i;
		}
	}
	str += "............!!f..!!!!!!!$$$$$$$$$$""!!!0()........A......../....Z";
	str += "abcdefghYjklmn";
	for (int i = 0; i < 3; i++) {
		str += str;
	}

	const char tbl[][16] = {
		"zz",
		"09",
		"az09",
		"09afAF//..",
	};
	const std::string *pstr = text.empty() ? &str : &text;
	for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
		const std::string key = tbl[i];
		benchmark("findChar_range_C", Frange<findChar_range_C>(), "findChar_range", Frange<mie::findChar_range>(), *pstr, key);
	}
}

void findStr_test(const std::string& text)
{
	puts("findStr_test");
	struct {
		const char *str;
		const char *key;
	} tbl[] = {
		{ "abcdefghijklmn", "fghi" },
		{ "abcdefghijklmn", "x" },
		{ "abcdefghijklmn", "i" },
		{ "abcdefghijklmn", "ij" },
		{ "abcdefghijklmn", "lmn" },
		{ "abcdefghijklmn", "abcdefghijklm" },
		{ "0123456789abcdefghijkl", "0123456789abcdef" },
		{ "0123456789abcdefghijkl", "0123456789abcdefghijklm" },
		{ "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTU", "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTU@" },
	};
	for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
		std::string str = tbl[i].str;
		for (int j = 0; j < 4; j++) {
			str += str;
		}
		const std::string key = tbl[i].key;
		const std::string *pstr = text.empty() ? &str : &text;
		benchmark("findStr_C", Frange<findStr_C>(), "findStr", Frange<mie::findStr>(), *pstr, key);
	}
	puts("ok");
}

int main(int argc, char *argv[])
{
	if (!mie::isAvaiableSSE42()) {
		fprintf(stderr, "SSE4.2 is not supported\n");
		return 1;
	}
	argc--, argv++;
	const std::string text = (argc == 1) ? LoadFile(argv[0]) : "";
	std::vector<std::string> keyTbl;

	const char tbl[][32] = {
		"a", "b",
		"xy", "ex",
		"std", "jit",
		"atoi", "1234",
		"File", "?????",
		"patch", "56789",
		"\xE3\x81\xA7\xE3\x81\x99", /* de-su */
		"cybozu",
		"openssl",
		"namespace",
		"\xe3\x81\x93\xe3\x82\x8c\xe3\x81\xaf", /* ko-re-wa */
		"cybozu::ssl",
		"asdfasdfasdf",
		"const_iterator",
		"000000000000000",
		"WARIXDFSKVJWSVFDVWESVF",
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
	};

	for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
		keyTbl.push_back(tbl[i]);
	}

	try {
#if 0
		strlen_test();
		strchr_test(text);
		strchr_any_test(text);
		strchr_range_test(text);

		strstr_test();
		if (!text.empty()) {
			benchmarkTbl("strstr_C", Fstrstr<strstr>(), "strstr", Fstrstr<mie::strstr>(), text, keyTbl);
			benchmarkTbl("string::find", Fstr_find(), "findStr", Frange<mie::findStr>(), text, keyTbl);
		}

		findChar_test(text);
		findChar_any_test(text);
		findChar_range_test(text);
#endif
//		findStr_test(text);
			benchmarkTbl("strstr_C", Fstrstr<strstr>(), "strstr", Fstrstr<mie::strstr>(), text, keyTbl);
		return 0;
	} catch (Xbyak::Error err) {
		printf("ERR:%s(%d)\n", Xbyak::ConvertErrorToString(err), err);
	} catch (...) {
		printf("unknown error\n");
	}
	return 1;
}
