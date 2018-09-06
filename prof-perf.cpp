/*
	How to profile JIT-code with perf
	@author herumi
*/
#include <stdio.h>
#include <math.h>
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>

#include <cybozu/exception.hpp>

struct PerfMap {
	FILE *fp;
	PerfMap()
		: fp(0)
	{
		const int pid = getpid();
		char name[128];
		snprintf(name, sizeof(name), "/tmp/perf-%d.map", pid);
		fp = fopen(name, "wb");
		if (fp == 0) throw cybozu::Exception("PerMap") << name;
	}
	~PerfMap()
	{
		close();
	}
	void close()
	{
		if (fp == 0) return;
		fclose(fp);
		fp = 0;
	}
	template<class Pointer>
	void set(const Pointer& p, size_t n, const char *name) const
	{
		fprintf(fp, "%llx %zx %s\n", (long long)p, n, name);
	}
};

struct Code : public Xbyak::CodeGenerator {
	Code()
	{
		mov(eax, 1000000);
	L("@@");
		for (int i = 0; i < 10; i++) {
			sub(eax, 1);
		}
		jg("@b");
		mov(eax, 1);
		ret();
	}
};

struct Code2 : public Xbyak::CodeGenerator {
	Code2()
	{
		mov(eax, 1000000);
	L("@@");
		for (int i = 0; i < 10; i++) {
			xorps(xm0, xm0);
		}
		sub(eax, 1);
		jg("@b");
		mov(eax, 1);
		ret();
	}
};

double s1(int n)
{
	double r = 0;
	for (int i = 0; i < n; i++) {
		r += 1.0 / (i + 1);
	}
	return r;
}

double s2(int n)
{
	double r = 0;
	for (int i = 0; i < n; i++) {
		r += 1.0 / (i * i + 1) + 2.0 / (i + 3);
	}
	return r;
}

int main()
{
#ifdef XBYAK64
	puts("64bit profile sample");
#else
	puts("32bit profile sample");
#endif
	Code c;
	Code2 c2;
	int (*f)() = (int (*)())c.getCode();
	int (*g)() = (int (*)())c2.getCode();

	printf("f:%p, %d\n", f, (int)c.getSize());
	printf("g:%p, %d\n", g, (int)c2.getSize());
	puts("use perf.map");
	PerfMap pm;
	pm.set(f, c.getSize(), "fff");
	pm.set(g, c2.getSize(), "ggg");

	double sum = 0;
	for (int i = 0; i < 20000; i++) {
		sum += s1(i);
		sum += s2(i);
	}
	printf("sum=%f\n", sum);
	for (int i = 0; i < 2000; i++) {
		sum += f();
	}
	printf("f=%f\n", sum);
	for (int i = 0; i < 2000; i++) {
		sum += g();
	}
	printf("g=%f\n", sum);
	puts("end");
}
