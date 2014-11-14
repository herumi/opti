#define _GNU_SOURCE
#include "mie_findstr.h"
#include <string.h>
#include <stdio.h>
#include <memory.h>

int g_errNum = 0;
int g_testNum = 0;

#define VERIFY_ASSERT(x) if (!(x)) { printf("ERR %s:%d: " #x "\n", __FILE__, __LINE__); g_errNum++; } g_testNum++;

void test_findStr()
{
	const struct {
		const char *text;
		const char *key;
	} tbl[] = {
		{ "abcdefgh", "abcdefg" },
		{ "abcdefgh", "def" },
		{ "abcdefgh", "fgh" },
		{ "abcdefgh", "a" },
		{ "abcdefgh", "abcdefghi" },
		{ "abcdefghabCDEfgh", "CDEf" },
		{ "01234567890abcdefghijklmnopqrstuvwxyz", "fghijklmnopqrstuvwx" },
	};
	size_t i;
	for (i = 0; i < sizeof(tbl) / sizeof(*tbl); i++) {
		const char *begin = tbl[i].text;
		const size_t textSize = strlen(begin);
		const char *end = begin + textSize;
		const char *key = tbl[i].key;
		const size_t keySize = strlen(key);
		const char *p = mie_findStr(begin, end, key, keySize);
		const char *q = memmem(begin, textSize, key, keySize);
		if (q == NULL) {
			VERIFY_ASSERT(p == end);
		} else {
			VERIFY_ASSERT(p == q);
		}
	}
}
void test_findCaseStr()
{
	const struct {
		const char *text;
		const char *key;
	} tbl[] = {
		{ "abcdefgh", "abcdefg" },
		{ "aBcdEfgh", "abcdefg" },
		{ "abcDEFGh", "def" },
		{ "abcdefgh", "fgh" },
		{ "AbcadefAgh", "a" },
		{ "abcdEfgH", "abcdefghi" },
		{ "01234567890abcdefGhiJklmnopqrstuVwxyz", "fghijklmnopqrstuvwx" },
	};
	size_t i;
	for (i = 0; i < sizeof(tbl) / sizeof(*tbl); i++) {
		const char *begin = tbl[i].text;
		const size_t textSize = strlen(begin);
		const char *end = begin + textSize;
		const char *key = tbl[i].key;
		const size_t keySize = strlen(key);
		const char *p = mie_findCaseStr(begin, end, key, keySize);
		const char *q = strcasestr(begin, key);
		if (q == NULL) {
			VERIFY_ASSERT(p == end);
		} else {
			VERIFY_ASSERT(p == q);
		}
	}
}
int main()
{
	test_findStr();
	test_findCaseStr();
	if (g_errNum == 0) {
		printf("test ok %d\n", g_testNum);
	}
	return 0;
}
