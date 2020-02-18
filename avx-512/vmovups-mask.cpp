#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>
#include <stdint.h>

// return memory where base[16] can't be written if !canWrite
uint8_t *getBoundary(bool canWrite)
{
	const int size = 4096;
	static MIE_ALIGN(4096) uint8_t top[size * 3];
	printf("access %s\n", canWrite ? "ok" : "ng");
	uint8_t *const base = top + size - 16;
	for (int i = 0; i < 32; i++) {
		base[i] = uint8_t(i);
	}
	if (!canWrite) {
		if (!Xbyak::CodeArray::protect(top + size, size, Xbyak::CodeArray::PROTECT_RE)) {
			fprintf(stderr, "can't change access mode\n");
			exit(1);
		}
	}
	return base;
}

struct Code : Xbyak::CodeGenerator {
	Code()
	{
		Xbyak::util::StackFrame sf(this, 1);
		vpcmpeqb(xmm1, xmm1, xmm1);
		mov(eax, (1 << 15) - 1);
		kmovw(k1, eax);
		vmovdqu8(ptr[sf.p[0]]|k1, xmm1);
		/*
			ignore T_z to write memory
			write only 15-byte size
		*/
	}
	void genTestUps()
	{
		Xbyak::util::StackFrame sf(this, 1);
		xorps(xm0, xm0);
		mov(eax, 7);
		kmovd(k1, eax);
		vmovups(ptr[sf.p[0]]|k1, xm0);
	}
};

void dump(const void *buf, size_t n = 16)
{
	const uint8_t *p = (const uint8_t*)buf;
	for (size_t i = 0; i < n; i++) {
		printf("%02x ", p[i]);
	}
	printf("\n");
}

typedef void (*Func)(void*);

void dump(const float *x, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		printf("%e ", x[i]);
	}
	printf("\n");
}

void testUps(Func f, uint8_t *p)
{
	float *x = (float *)p;
	x[0] = 1;
	x[1] = 2;
	x[2] = 3;
	puts("x");
	dump(x, 4);
	f(x);
	puts("x after f");
	dump(x, 4);
}

int main()
	try
{
	Code code;
	auto f = code.getCode<Func>();
	auto g = code.getCurr<Func>();
	code.genTestUps();
	uint8_t buf[256] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
	dump(buf);
	f(buf);
	dump(buf);
	puts("boundary");
	uint8_t *p = getBoundary(false);
	dump(p, 20);
	f(p + 1);
	dump(p, 20);
	testUps(g, p);
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
	return 1;
}
