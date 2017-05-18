#include <stdio.h>
#include <cybozu/mmap.hpp>
#include <cybozu/benchmark.hpp>
#include <map>
#include "remove_ctrl.hpp"

typedef std::map<size_t, size_t> Stat;

Stat makeStat(const std::string& s)
{
	Stat st;
	size_t prev = 0;
	for (size_t i = 0; i < s.size(); i++) {
		if (s[i] < 0x20) {
			st[i - prev]++;
			prev = i;
		}
	}
	return st;
}

void putStat(const Stat& st)
{
	for (Stat::const_iterator i = st.begin(), ie = st.end(); i != ie; ++i) {
		printf("%5d %5d\n", (int)i->first, (int)i->second);
	}
	printf("\n");
}

Stat shrinkStat(const Stat& st)
{
	const size_t w = 10;
	const size_t max = 20;
	Stat sub;
	for (Stat::const_iterator i = st.begin(), ie = st.end(); i != ie; ++i) {
		size_t idx = i->first / w;
		if (idx >= max) idx = max - 1;
		sub[idx] += i->second;
	}
	return sub;
}

std::string bench(const std::string& s, size_t f(char *, const char *, size_t))
{
	cybozu::CpuClock clk;
	std::string t;
	t.resize(s.size() + 64); // +64 is margin
	const int N = 30;
	size_t writeSize = 0;
	for (int i = 0; i < N; i++) {
		clk.begin();
		writeSize = f(&t[0], &s[0], s.size());
		clk.end();
	}
	double ave = clk.getClock() / double(N);
	printf("ave = %.2e clk. %.2f clk/byte\n", ave, ave / s.size());
	t.resize(writeSize);
	return t;
}

void compare(const std::string& a, const std::string& b)
{
	if (a.size() != b.size()) {
		printf("err size %zd %zd\n", a.size(), b.size());
		return;
	}
	printf("ok size %zd\n", a.size());
	for (size_t i = 0; i < a.size(); i++) {
		if (a[i] != b[i]) {
			printf("%zd %x %x\n", i, (unsigned char)a[i], (unsigned char)b[i]);
			return;
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		printf("%s <file>\n", argv[0]);
		return 1;
	}
	cybozu::Mmap m(argv[1]);
	const std::string s(m.get(), m.size());
	Stat st = makeStat(s);
	putStat(shrinkStat(st));
	std::string a = bench(s, removeCtrlOrg);
	std::string b = bench(s, removeCtrlSearch);
	std::string c = bench(s, removeCtrlCmpestri);
	std::string d = bench(s, removeCtrlCmpestriCopy);
	std::string e = bench(s, removeCtrlAVX);
	compare(a, b);
	compare(a, c);
	compare(a, d);
	compare(a, e);
}
