#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "str_util.hpp"
#include "util.hpp"
#include "benchmark.hpp"

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
