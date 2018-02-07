#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>
#include <stdint.h>

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
#ifdef _WIN32
		DWORD old;
		bool isOK = VirtualProtect(top + size, size, PAGE_READONLY, &old) != 0;
#else
		bool isOK = mprotect(top + size, size, PROT_READ) == 0;
#endif
		if (!isOK) {
			perror("protect");
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
};

void dump(const uint8_t *p, size_t n = 16)
{
	for (int i = 0; i < n; i++) {
		printf("%02x ", p[i]);
	}
	printf("\n");
}

int main()
	try
{
	Code code;
	auto f = code.getCode<void (*)(uint8_t*)>();
	uint8_t buf[256] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
	dump(buf);
	f(buf);
	dump(buf);
	puts("boundary");
	uint8_t *p = getBoundary(false);
	dump(p, 20);
	f(p + 1);
	dump(p, 20);
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
	return 1;
}
