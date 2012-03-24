#include <string>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "str_util.hpp"

#define NUM_OF_ARRAY(x) (sizeof(x)/sizeof(*x))
#define TEST_EQUAL(a, b) { if ((a) != (b)) { fprintf(stderr, "%s:%d err a=%lld, b=%lld\n", __FILE__, __LINE__, (long long)(a), (long long)(b)); exit(1); } }

double strstr_test1(const std::string& text, const std::string& key)
{
	int num1 = 0;
	int num2 = 0;
	double time1 = 0;
	double time2 = 0;
	long long len1 = 0;
	long long len2 = 0;
	const int N = 10;
	for (int i = 0; i < N; i++) {
		{
			Xbyak::util::Clock clk;
			const char *p = &text[0];
			const char *const end = p + text.size();
			while (p != end) {
				clk.begin();
				const char *q = strstr(p, key.c_str());
				clk.end();
				if (q == 0) break;
				num1++;
				len1 += (q - p);
				p = q + 1;
			}
			time1 += clk.getClock() / double(clk.getCount()) * 1e-3;
		}
		{
			Xbyak::util::Clock clk;
			const char *p = &text[0];
			const char *const end = p + text.size();
			while (p != end) {
				clk.begin();
				const char *q = mie::strstr(p, key.c_str());
				clk.end();
				if (q == 0) break;
				num2++;
				len2 += (q - p);
				p = q + 1;
			}
			time2 += clk.getClock() / double(clk.getCount()) * 1e-3;
		}
	}
	num1 /= N;
	num2 /= N;
	double flen1 = len1 / (double)N;
	double flen2 = len2 / (double)N;
	time1 /= N;
	time2 /= N;
	if (num1 != num2 || len1 != len2) {
		fprintf(stderr, "err key=%s, (%d, %d), (%d, %d)\n", key.c_str(), num1, (int)len1, num2, (int)len2);
	}
	if (num1) flen1 /= num1;
	if (num2) flen2 /= num2;
	double ave1 = time1 / (len1 ? flen1 : text.size()) * 1e3;
	double ave2 = time2 / (len2 ? flen2 : text.size()) * 1e3;
	double rate = time1 / time2;
	printf("%26s %6d%10.1f %8.2f %5.2f %8.2f %5.2f %4.2f\n", key.substr(0,26).c_str(), num1, flen1, time1, ave1, time2, ave2, rate);
	return rate;
}

void strstr_test()
{
	puts("strstr_test");
	const int SIZE = 1024 * 1024 * 10;
	struct {
		const char *text;
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
		std::string text;
		text.reserve(SIZE);
		const size_t len = strlen(tbl[i].text);
		for (size_t j = 0; j < SIZE / len; j++) {
			text.append(tbl[i].text, len);
		}
		strstr_test1(text, tbl[i].key);
	}
}
const int MaxChar = 254;

void strchr_test1(const char *str, const char *f(const char*, int))
{
	Xbyak::util::Clock clk;
	size_t ret = 0;
	const int count = 30000;
	for (int i = 0; i < count; i++) {
		clk.begin();
		for (int c = 1; c <= MaxChar; c++) {
			const char *p = f(str, c);
			if (p) {
				ret += p - str;
			} else {
				ret += MaxChar;
			}
		}
		clk.end();
	}
	printf("ret=%d, %.3f\n", (int)(ret / count), clk.getClock() / (double)ret);
}

void strchr_test()
{
	puts("strchr_test");
	MIE_ALIGN(16) char str[MaxChar + 1];
	for (int i = 1; i < MaxChar; i++) {
		str[i - 1] = (char)i;
	}
	str[MaxChar] = '\0';
	strchr_test1(str, (const char*(*)(const char*,int))strchr);
	strchr_test1(str, mie::strchr);
}

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

const char *strchr_range_C(const char *str, const char *key)
{
	while (*str) {
		unsigned char c = (unsigned char)*str;
		for (unsigned char *p = (unsigned char *)key; *p; p += 2) {
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

void strchr_any_test()
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
	for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
		const char *p1 = str.c_str();
		const char *p2 = str.c_str();
		const char *key = tbl[i];
		for (;;) {
			p1 = strchr_any_C(p1, key);
			p2 = mie::strchr_any(p2, key);
			TEST_EQUAL(p1, p2);
			if (p1 == 0) break;
			p1++;
			p2++;
		}
	}
	puts("ok");
}

void strchr_range_test()
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
	for (size_t i = 0; i < NUM_OF_ARRAY(tbl); i++) {
		const char *p1 = str.c_str();
		const char *p2 = str.c_str();
		const char *key = tbl[i];
		for (;;) {
			p1 = strchr_any_C(p1, key);
			p2 = mie::strchr_any(p2, key);
			TEST_EQUAL(p1, p2);
			if (p1 == 0) break;
			p1++;
			p2++;
		}
	}
	puts("ok");
}

const char *findChar_C(const char *begin, const char *end, char c)
{
	return std::find(begin, end, c);
}

void findChar_test()
{
	puts("findChar_test");
	std::string str = "123a456abcdefghijklmnob123aa3vnraw3nabcdevra";
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 256; i++) {
			str += (char)i;
		}
	}
	str += "abcdefghijklmn\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

	const char *const begin = &str[0];
	const char *const end = begin + str.size() - 16;
	for (int c = 0; c < 256; c++) {
		const char *p1 = begin;
		const char *p2 = begin;
		for (;;) {
			p1 = findChar_C(p1, end, (char)c);
			p2 = mie::findChar(p2, end, (char)c);
			TEST_EQUAL(p1, p2);
			if (p1 == end) break;
			p1++;
			p2++;
		}
	}
	puts("ok");
}

int main()
{
	try {
		strstr_test();
		strchr_test();
		strlen_test();
		strchr_any_test();
		strchr_range_test();
		findChar_test();
		return 0;
	} catch (Xbyak::Error err) {
		printf("ERR:%s(%d)\n", Xbyak::ConvertErrorToString(err), err);
	} catch (...) {
		printf("unknown error\n");
	}
	return 1;
}
