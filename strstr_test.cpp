/*
	strstr benchmark
	require https://github.com/herumi/cybozulib
	        https://github.com/herumi/xbyak
	g++ -O3 -fomit-frame-pointer -march=native -msse4 -fno-operator-names strstr_test.cpp
	@author herumi
*/
#include <string>
#include <vector>
#include <stdio.h>
#include <cybozu/file.hpp>
#include <cybozu/quick_search.hpp>
#include "strstr_sse42.hpp"
#include "str_util.hpp"

#include <xbyak/xbyak_util.h>

double test(const std::string& text, const std::string& key)
{
	int num1 = 0;
	int num2 = 0;
	double time1 = 0;
	double time2 = 0;
	double len1 = 0;
	double len2 = 0;
	const int N = 10;
	for (int i = 0; i < N; i++) {
		{
			Xbyak::util::Clock clk;
			const char *p = &text[0];
			const char *const end = p + text.size();
			while (p != end) {
				clk.begin();
				const char *q = strstr(p, key.c_str());
//				const char *q = strstr_sse42(p, key.c_str());
				clk.end();
				if (q == 0) break;
				num1++;
				len1 += (int)(q - p);
				p = q + 1;
			}
			time1 += clk.getClock() / double(clk.getCount()) * 1e-3;
		}
		{
			Xbyak::util::Clock clk;
			const char *p = &text[0];
			const char *const end = p + text.size();
//			cybozu::QuickSearch qs(key);
			QuickSearch2 qs(key);
			const size_t len = key.size();
			while (p != end) {
				clk.begin();
				const char *q = strstr_sse42(p, key.c_str());
//				const char *q = qs.find(p, end);
//				const char *q = qs_find(p, key.c_str(), len, qs.tbl_);
				clk.end();
				if (q == 0) break;
				num2++;
				len2 += (int)(q - p);
				p = q + 1;
			}
			time2 += clk.getClock() / double(clk.getCount()) * 1e-3;
		}
	}
	num1 /= N;
	num2 /= N;
	len1 /= N;
	len2 /= N;
	time1 /= N;
	time2 /= N;
	if (num1 != num2 || len1 != len2) {
		fprintf(stderr, "err key=%s, (%d, %d), (%d, %d)\n", key.c_str(), num1, (int)len1, num2, (int)len2);
	}
	if (num1) len1 /= num1;
	if (num2) len2 /= num2;
	double ave1 = time1 / (len1 ? len1 : text.size()) * 1e3;
	double ave2 = time2 / (len2 ? len2 : text.size()) * 1e3;
	double rate = time1 / time2;
	printf("%26s %6d%10.1f %8.2f %5.2f %8.2f %5.2f %4.2f\n", key.substr(0,26).c_str(), num1, len1, time1, ave1, time2, ave2, rate);
	return rate;
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

double test2(const std::string& text, const std::string& key)
{
	int num1 = 0;
	int num2 = 0;
	double time1 = 0;
	double time2 = 0;
	double len1 = 0;
	double len2 = 0;
	const int N = 10;
	for (int i = 0; i < N; i++) {
		{
			Xbyak::util::Clock clk;
			const char *k = key.c_str();
			size_t keySize = key.size();
#if 1
			size_t p = 0;
			while (p != std::string::npos) {
				clk.begin();
				size_t q = text.find(key, p);
				clk.end();
				if (q == std::string::npos) break;
				num1++;
				len1 += (int)(q - p);
				p = q + 1;
			}
#else
			const char *p = &text[0];
			const char *const end = p + text.size();
			while (p != end) {
				clk.begin();
				const char *q = findStr_C(p, end, k, keySize);
				clk.end();
				if (q == end) break;
				num1++;
				len1 += (int)(q - p);
				p = q + 1;
			}
#endif
			time1 += clk.getClock() / double(clk.getCount()) * 1e-3;
		}
		{
			Xbyak::util::Clock clk;
			const char *p = &text[0];
			const char *const end = p + text.size();
			const char *k = key.c_str();
			size_t keySize = key.size();
			while (p != end) {
				clk.begin();
				const char *q = mie::findStr(p, end, k, keySize);
#if 0
				const char *r = findStr_C(p, end, k, keySize);
				if (q != r) {
					printf("p=%s, key=%s, q=%p(%d), r=%p(%d)\n", std::string(p, 10).c_str(), k, q, (int)(q - &text[0]), r, (int)(r - &text[0]));
					exit(1);
				}
#endif
				clk.end();
				if (q == end) break;
				num2++;
				len2 += (int)(q - p);
				p = q + 1;
			}
			time2 += clk.getClock() / double(clk.getCount()) * 1e-3;
		}
	}
	num1 /= N;
	num2 /= N;
	len1 /= N;
	len2 /= N;
	time1 /= N;
	time2 /= N;
	if (num1 != num2 || len1 != len2) {
		fprintf(stderr, "err key=%s, (%d, %d), (%d, %d)\n", key.c_str(), num1, (int)len1, num2, (int)len2);
	}
	if (num1) len1 /= num1;
	if (num2) len2 /= num2;
	double ave1 = time1 / (len1 ? len1 : text.size()) * 1e3;
	double ave2 = time2 / (len2 ? len2 : text.size()) * 1e3;
	double rate = time1 / time2;
	printf("%24s %6d%10.1f %8.2f %5.2f %8.2f %5.2f %4.2f\n", key.substr(0,24).c_str(), num1, len1, time1, ave1, time2, ave2, rate);
	return rate;
}

void simpleTest()
{
	std::string text;
	for (int i = 0; i < 100000; i++) {
		text += "0123456789abcdefghijkl";
	}
	test(text, "0123456789abcdefghijklm");
	std::string text2;
	for (int i = 0; i < 10000; i++) {
		text2 += "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTU";
	}
	test(text2, "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTU@");
}

int main(int argc, char *argv[])
{
	argc--, argv++;
	if (argc < 1) {
		simpleTest();
		return 0;
	}
	const std::string dir = argv[0];
	const std::string key = argc == 2 ? argv[1] : "";
	std::vector<cybozu::file::FileInfo> v;
	if (!cybozu::file::GetFilesInDir(v, dir)) {
		fprintf(stderr, "not find in %s\n", dir.c_str());
		return 1;
	}
	std::string text;
	for (size_t i = 0; i < v.size(); i++) {
		if (!v[i].isFile) continue;
		std::string fileName = dir + '/' + v[i].name;
		cybozu::File f;
		if (!f.openR(fileName)) continue;
		size_t cur = text.size();
		int fileSize = (int)f.getSize();
//		printf("file %s %d\n", fileName.c_str(), fileSize);
		text.resize(cur + fileSize);
		f.read(&text[cur], fileSize);
	}
	/* remove NUL */
	for (size_t i = 0; i < text.size(); i++) {
		if (text[i] == 0) text[i] = ' ';
	}
	text += '\0';
	printf("text size=%d\n", (int)text.size());

	std::vector<std::string> keyTbl;
	if (key.empty()) {
		const char tbl[][32] = {
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
		for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
			keyTbl.push_back(tbl[i]);
		}
	} else {
		keyTbl.push_back(key);
	}

	printf("%25s %6s %10s %8s %5s %8s %5s rate\n", "key", "count", "len", "C:Kclk", "clk/B", "asm:Kclk", "clk/B");
#if 0
	double score = 0;
	for (size_t i = 0; i < keyTbl.size(); i++) {
		score += test(text, keyTbl[i]);
	}
	printf("score rate=%f\n", score / keyTbl.size());
#endif
	for (size_t i = 0; i < keyTbl.size(); i++) {
		test2(text, keyTbl[i]);
	}
}
