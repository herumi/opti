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
#ifdef USE_MISCHASAN
#include "mischasan_strstr.hpp"
#endif
#ifdef __GNUC__
#include <strings.h>
#endif

// stristr(key must not have capital character)
const char *stristr_C(const char *str, const char *key)
{
	const size_t keySize = strlen(key);
	while (*str) {
#ifdef _WIN32
		if (_memicmp(str, key, keySize) == 0) return str;
#else
		if (strncasecmp(str, key, keySize) == 0) return str;
#endif
		str++;
	}
	return 0;
}
const char *findiStr_C(const char *begin, const char *end, const char *key, size_t keySize)
{
	while (begin + keySize <= end) {
#ifdef _WIN32
		if (_memicmp(begin, key, keySize) == 0) return begin;
#else
		if (strncasecmp(begin, key, keySize) == 0) return begin;
#endif
		begin++;
	}
	return end;
}

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

// findChar_range(begin, end, "azAZ<>")
struct FfindChar_range_emu {
	const char *begin_;
	const char *end_;
	char tbl_[256];
	typedef const char* type;
	void set(const std::string& str, const std::string&)
	{
		begin_ = &str[0];
		end_ = begin_ + str.size();
		for (unsigned int i = 0; i < 256; i++) {
			tbl_[i] = ('a' <= i && i <= 'z') || ('A' <= i && i <= 'Z') || ('<' <= i && i <= '>');
		}
	}
	FfindChar_range_emu() : begin_(0), end_(0) { }
	const char *begin() const { return begin_; }
	const char *end() const { return end_; }
	const char *find(const char *p) const
	{
		while (p < end_) {
			if (tbl_[(unsigned char)*p]) return p;
			p++;
		}
		return end_;
	}
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

const char *mystrstr_C(const char *str, const char *key)
{
	size_t len = strlen(key);
//	if (len == 1) return strchr(str, key[0]);
	while (*str) {
		const char *p = strchr(str, key[0]);
		if (p == 0) return 0;
		if (memcmp(p + 1, key + 1, len - 1) == 0) return p;
		str = p + 1;
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

const char *findStr2_C(const char *begin, const char *end, const char *key, size_t keySize)
{
	while (begin + keySize <= end) {
		const char *p = (const char*)memchr(begin, key[0], end - begin);
		if (p == 0) break;
		if (memcmp(p + 1, key + 1, keySize - 1) == 0) {
			return p;
		}
		begin = p + 1;
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
		benchmark("strchr_C", Fstrchr<STRCHR>(), "strchr", Fstrchr<mie::strchr>(), *pstr, std::string(1, (char)c));
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
		benchmark("strstr_C", Fstrstr<STRSTR>(), "strstr", Fstrstr<mie::strstr>(), str, key);
		benchmark("strstr2_C", Fstrstr<mystrstr_C>(), "strstr", Fstrstr<mie::strstr>(), str, key);
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
		"0-9a-fA-F",
		"09afAF//..",
	};
	const std::string *pstr = text.empty() ? &str : &text;
	for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
		const std::string key = tbl[i];
		benchmark("findChar_range_C", Frange<findChar_range_C>(), "findChar_range", Frange<mie::findChar_range>(), *pstr, key);
	}
	std::string key = "azAZ<>"; // QQQ:same as in FfindChar_range_emu
	benchmark("findChar_range_emu", FfindChar_range_emu(), "findChar_range", Frange<mie::findChar_range>(), text, key);
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
		benchmark("findStr2_C", Frange<findStr2_C>(), "findStr", Frange<mie::findStr>(), *pstr, key);
	}
	puts("ok");
}

void stristr_test(const std::string& text)
{
	puts("stristr_test");
	std::string str;
	for (int i = 1; i < 256; i++) {
		str += (char)i;
	}
	for (int i = 0; i < 3; i++) {
		str += str;
	}
	for (int i = 1; i < 256; i++) {
		if ('A' <= i && i <= 'Z') continue;
		std::string key(1, (char)i);
		verify(Fstrstr<stristr_C>(), Fstrstr<mie::stristr>(), str, key);
	}
	str = "@AZ[`az{";
	for (int i = 0; i < 7; i++) {
		str += str;
	}
	const char tbl[][48] = {
		"@",
		"a",
		"z",
		"@az[",
		"`az{",
		"z[`",
		"@az[`az{",
		"@az[`az{@az[`az",
		"@az[`az{@az[`az{@az[`az{@az[`az{",
	};
	for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
		verify(Fstrstr<stristr_C>(), Fstrstr<mie::stristr>(), str, tbl[i]);
	}
	if (!text.empty()) {
		const char tbl[][32] = {
			"ssl",
			"cybozu",
			"xyz",
			"@",
			"}",
			"["
			"`",
			"abc",
			"xyz",
			"openssl",
			"0123456789",
			"00000000000",
			"abcdefghijklmnopqrstuvwxyz",
		};
		for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			benchmark("stristr_C", Fstrstr<stristr_C>(), "mie::stristr", Fstrstr<mie::stristr>(), text, tbl[i]);
		}
	}
	puts("ok");
}

void findiStr_test(const std::string& text)
{
	puts("findiStr_test");
	std::string str;
	for (int i = 1; i < 256; i++) {
		str += (char)i;
	}
	for (int i = 0; i < 3; i++) {
		str += str;
	}
	for (int i = 1; i < 256; i++) {
		if ('A' <= i && i <= 'Z') continue;
		std::string key(1, (char)i);
		verify(Frange<findiStr_C>(), Frange<mie::findiStr>(), str, key);
	}
	str = "@AZ[`az{";
	for (int i = 0; i < 7; i++) {
		str += str;
	}
	const char tbl[][48] = {
		"@",
		"a",
		"z",
		"@az[",
		"`az{",
		"z[`",
		"@az[`az{",
		"@az[`az{@az[`az",
		"@az[`az{@az[`az{@az[`az{@az[`az{",
	};
	for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
		verify(Frange<findiStr_C>(), Frange<mie::findiStr>(), str, tbl[i]);
	}
	if (!text.empty()) {
		const char tbl[][32] = {
			"ssl",
			"cybozu",
			"xyz",
			"@",
			"}",
			"["
			"`",
			"abc",
			"xyz",
			"openssl",
			"0123456789",
			"00000000000",
			"abcdefghijklmnopqrstuvwxyz",
		};
		for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
			benchmark("findiStr_C", Frange<findiStr_C>(), "mie::findiStr", Frange<mie::findiStr>(), text, tbl[i]);
		}
	}
	puts("ok");
}

int main(int argc, char *argv[])
{
	if (!mie::isAvaiableSSE42()) {
		fprintf(stderr, "SSE4.2 is not supported\n");
//		return 1;
	}
	argc--, argv++;
	const std::string text = (argc == 1) ? LoadFile(argv[0]) : "";
	std::vector<std::string> keyTbl;

	const char tbl[][32] = {
		"a", // dummy for cache
		"a", "b", "c", "d",
		"ab", "xy", "ex",
		"std", "jit", "asm",
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
		"static_assert",
		"const_iterator",
		"000000000000000",
		"WARIXDFSKVJWSVFDVWESVF",
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
	};

	for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
		keyTbl.push_back(tbl[i]);
	}

	try {
#ifdef USE_MISCHASAN
		benchmarkTbl("mischasan", Fmischasan_strstr(), "strstr", Fstrstr<STRSTR>(), text, keyTbl);
		benchmarkTbl("mischasan", Fmischasan_strstr(), "mie::strstr", Fstrstr<mie::strstr>(), text, keyTbl);
		return 0;
#endif
#if 1
		/*
			compare strstr with stristr
			stristr speed is about 0.66~0.70 times speed of strstr
		*/
		if (!text.empty()) {
			std::string itext = text;
			for (size_t i = 0; i < itext.size(); i++) {
				char c = itext[i];
				if ('A' <= c && c <= 'Z') itext[i] = c + 'a' - 'A';
			}
			benchmarkTbl("mie::strstr", Fstrstr<mie::strstr>(), "mie::stristr", Fstrstr<mie::stristr>(), itext, keyTbl);
		}
#endif
		stristr_test(text);
		findiStr_test(text);
		strlen_test();
		strchr_test(text);
		strchr_any_test(text);
		strchr_range_test(text);

		strstr_test();
		if (!text.empty()) {
			benchmarkTbl("strstr_C", Fstrstr<STRSTR>(), "strstr", Fstrstr<mie::strstr>(), text, keyTbl);
			benchmarkTbl("mystrstr_C", Fstrstr<mystrstr_C>(), "strstr", Fstrstr<mie::strstr>(), text, keyTbl);
			benchmarkTbl("string::find", Fstr_find(), "findStr", Frange<mie::findStr>(), text, keyTbl);
		}

		findChar_test(text);
		findChar_any_test(text);
		findChar_range_test(text);
		findStr_test(text);
		return 0;
	} catch (Xbyak::Error err) {
		printf("ERR:%s(%d)\n", Xbyak::ConvertErrorToString(err), err);
	} catch (...) {
		printf("unknown error\n");
	}
	return 1;
}
