#include <string>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "str_util.hpp"

#define NUM_OF_ARRAY(x) (sizeof(x)/sizeof(*x))

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
				const char *q = strstr_sse42(p, key.c_str());
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
	int ret = 0;
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
	printf("ret=%d, %.3f\n", ret / count, clk.getClock() / (double)ret);
}

void strchr_test()
{
	MIE_ALIGN(16) char str[MaxChar + 1];
	for (int i = 1; i < MaxChar; i++) {
		str[i - 1] = (char)i;
	}
	str[MaxChar] = '\0';
	strchr_test1(str, strchr);
	strchr_test1(str, strchr_sse42);
}
int main()
{
	const char *p = strstr_sse42("abcdefg", "de");
	printf("p=%p\n", p);
	try {
		strstr_test();
		strchr_test();
		return 0;
	} catch (Xbyak::Error err) {
		printf("ERR:%s(%d)\n", Xbyak::ConvertErrorToString(err), err);
	} catch (...) {
		printf("unknown error\n");
	}
	return 1;
}


