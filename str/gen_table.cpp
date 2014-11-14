#include <stdint.h>
#include <memory.h>
#include <set>
#include <vector>
#include <stdio.h>

typedef std::set<uint64_t> IntSet;
typedef std::vector<int> IntVec;

uint32_t hash(const char *p, size_t n, int s, int mod)
{
	uint64_t v = 14695981039346656037ULL;
	for (size_t i = 0; i < n; i++) {
		v ^= (uint8_t)p[i];
		v *= 1099511628211ULL;
	}
	v ^= v >> s;
	return uint32_t(v % mod);
}

const char keyTbl[][64] = {
	":authority",
	":method",
	":path",
	":scheme",
	":status",
	"accept",
	"accept-charset",
	"accept-encoding",
	"accept-language",
	"accept-ranges",
	"access-control-allow-origin",
	"age",
	"allow",
	"authorization",
	"cache-control",
	"connection",
	"content-disposition",
	"content-encoding",
	"content-language",
	"content-length",
	"content-location",
	"content-range",
	"content-type",
	"cookie",
	"date",
	"etag",
	"expect",
	"expires",
	"from",
	"host",
	"http2-settings",
	"if-match",
	"if-modified-since",
	"if-none-match",
	"if-range",
	"if-unmodified-since",
	"last-modified",
	"link",
	"location",
	"max-forwards",
	"proxy-authenticate",
	"proxy-authorization",
	"range",
	"referer",
	"refresh",
	"retry-after",
	"server",
	"set-cookie",
	"strict-transport-security",
	"transfer-encoding",
	"upgrade",
	"user-agent",
	"vary",
	"via",
	"www-authenticate",
	"x-reproxy-url",
};
const int N = sizeof(keyTbl) / sizeof(*keyTbl);

bool count(int s, int mod)
{
	IntSet set;
	for (int i = 0; i < N; i++) {
		const char *p = keyTbl[i];
		int h = hash(p, strlen(p), s, mod);
		set.insert(h);
	}
	return set.size() == N;
}

void put(int s, int mod)
{
	IntVec iv(mod);
	for (int i = 0; i < mod; i++) {
		iv[i] = -1;
	}
	for (int i = 0; i < N; i++) {
		const char *p = keyTbl[i];
		size_t len = strlen(p);
		uint32_t h = hash(p, len, s, mod);
		printf("%d %s(%d) -> %d\n", i, p, (int)len, h);
		iv[h] = i;
	}
	for (int i = 0; i < mod; i++) {
		printf("%d, ", iv[i]);
		if (((i + 1) % 10) == 0) printf("\n");
	}
	printf("\n");
}

int main()
{
	for (int mod = 64; mod < 1000; mod++) {
		for (int s = 1; s < 63; s++) {
			if (count(s, mod)) {
				printf("s=%d, mod=%d\n", s, mod);
				put(s, mod);
				return 0;
			}
		}
	}
}
