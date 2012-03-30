/*
	compare strstr and quick search
	require https://github.com/herumi/xbyak
	g++ quick_search_smpl.cpp -fno-operator-names -O3 -fomit-frame-pointer -march=native
	@author herumi
*/
#include <string>
#include <vector>
#include <cstring>
#include <stdio.h>
#include <xbyak/xbyak_util.h>
#include "quick_search.hpp"
#include "util.hpp"
#include "benchmark.hpp"

struct FquickSearch {
	const char *str_;
	const char *key_;
	const char *end_;
	typedef const char* type;
	QuickSearch qs_;
	void set(const std::string& str, const std::string& key)
	{
		str_ = &str[0];
		end_ = str_ + str.size();
		key_ = &key[0];
		qs_.init(key_, key_ + key.size());
	}
	FquickSearch() : str_(0), key_(0), end_(0) { }
	const char *begin() const { return str_; }
	const char *end() const { return end_; }
	const char *find(const char *p) const { return qs_.find(p, end_); }
};

struct FquickSearch_org {
	const char *str_;
	const char *key_;
	const char *end_;
	typedef const char* type;
	QuickSearch qs_;
	void set(const std::string& str, const std::string& key)
	{
		str_ = &str[0];
		end_ = str_ + str.size();
		key_ = &key[0];
		qs_.init(key_, key_ + key.size());
	}
	FquickSearch_org() : str_(0), key_(0), end_(0) { }
	const char *begin() const { return str_; }
	const char *end() const { return end_; }
	const char *find(const char *p) const { return qs_.find_org(p, end_); }
};

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

int main(int argc, char *argv[])
{
	argc--, argv++;
	if (argc < 1) {
		fprintf(stderr, "cmd text-file [key]\n");
		return 1;
	}
	std::string text = LoadFile(argv[0]);
	const std::string key = argc > 1 ? argv[1] : "";

	/* remove NUL */
	for (size_t i = 0; i < text.size(); i++) {
		if (text[i] == 0) text[i] = ' ';
	}
	text += '\0';
	printf("text size=%d\n", (int)text.size());

	std::vector<std::string> keyTbl;
	if (key.empty()) {
		const char tbl[][32] = {
			"a", "a", "b", "c", "d",
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
	} else {
		keyTbl.push_back(key);
	}

//	benchmarkTbl("strstr", Fstrstr<STRSTR>(), "qs org", FquickSearch_org(), text, keyTbl);
//	benchmarkTbl("strstr", Fstrstr<STRSTR>(), "qs", FquickSearch(), text, keyTbl);
	benchmarkTbl("qs", FquickSearch(), "mystrstr_C", Fstrstr<mystrstr_C>(), text, keyTbl);
}

