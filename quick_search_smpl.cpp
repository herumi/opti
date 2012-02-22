#include <string>
#include <vector>
#include <stdio.h>
#include <cybozu/quick_search.hpp>
#include <cybozu/file.hpp>
#include <xbyak/xbyak_util.h>

void test(const std::string& text, const std::string& key)
{
	int ret1 = 0;
	int ret2 = 0;
	double time1 = 0;
	double time2 = 0;
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
				ret1 += (int)(q - p);
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
				ret2 += (int)(q - p);
				p = q + 1;
			}
			time2 += clk.getClock() / double(clk.getCount()) * 1e-3;
		}
	}
	time1 /= N;
	time2 /= N;
	if (ret1 != ret2) {
		fprintf(stderr, "err key=%s, ret1=%d, ret2=%d\n", key.c_str(), ret1, ret2);
	}
	printf("%20s ret=%8d, strstr:%8.3fK quick:%8.3fK (%5.2f)\n", key.c_str(), ret1 / N, time1, time2, time1 / time2);
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
	text += '\0';
	printf("text size=%d\n", (int)text.size());

	std::vector<std::string> keyTbl;
	if (key.empty()) {
		const char tbl[][32] = {
			"cybozu", "namespace", "atoi", "cybozu::ssl", "static_assert", "File",
			"\xe3\x81\x93\xe3\x82\x8c\xe3\x81\xaf", /* ko-re-wa */
			"\xE3\x81\xA7\xE3\x81\x99", /* de-su */
			"openssl", "const_iterator", "patch",
			"a", "b", "c", "d", "ab", "xy", "ex", "std", "1234", "56789", "jit", "asm",
			"asdfasdfasdf", "000000000000000", "?????",
		};
		for (size_t i = 0; i < CYBOZU_NUM_OF_ARRAY(tbl); i++) {
			keyTbl.push_back(tbl[i]);
		}
	} else {
		keyTbl.push_back(key);
	}

	for (size_t i = 0; i < keyTbl.size(); i++) {
		test(text, keyTbl[i]);
	}
}
