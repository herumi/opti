/*
	compare strstr and quick search
	require https://github.com/herumi/cybozulib
	        https://github.com/herumi/xbyak
	g++ quick_search_smpl.cpp -I../cybozulib/include/ -fno-operator-names -O3 -fomit-frame-pointer -march=native
	@author herumi
*/
#include <string>
#include <vector>
#include <stdio.h>
#include <cybozu/quick_search.hpp>
#include <cybozu/file.hpp>
#include <xbyak/xbyak_util.h>

void test(const std::string& text, const std::string& key)
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
			cybozu::QuickSearch qs(key);
			while (p != end) {
				clk.begin();
				const char *q = qs.find(p, end);
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
	double ave1 = time1 / (len1 ? len1 : text.size()) * 1e3;
	double ave2 = time2 / (len2 ? len2 : text.size()) * 1e3;
	printf("%27s %6d %10.1f %8.2f %5.2f %8.2f %5.2f %4.2f\n", key.c_str(), num1, len1, time1, ave1, time2, ave2, time1 / time2);
}

int main(int argc, char *argv[])
{
	argc--, argv++;
	if (argc < 1) {
		fprintf(stderr, "cmd dir [key]\n");
		return 1;
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

	printf("%27s %6s %10s %8s %5s %8s %5s rate\n", "key", "count", "len", "str:Kclk", "clk/B", "qs:Kclk", "clk/B");
	for (size_t i = 0; i < keyTbl.size(); i++) {
		test(text, keyTbl[i]);
	}
}
