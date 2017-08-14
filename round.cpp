#include <stdio.h>
#include <math.h>
#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak.h>
#include <xbyak/xbyak_util.h>

struct Round : Xbyak::CodeGenerator {
	Round(uint8_t mode)
	{
		vroundpd(xmm0, xmm0, mode);
		vcvtsd2si(eax, xmm0);
		ret();
	}
};

struct Round2 : Xbyak::CodeGenerator {
	Round2(uint8_t mode, bool useAVX512)
	{
		if (useAVX512) {
			switch (mode) {
			case 0: vcvtsd2si(eax, xmm0 | T_rn_sae); break;
			case 1: vcvtsd2si(eax, xmm0 | T_rd_sae); break;
			case 2: vcvtsd2si(eax, xmm0 | T_ru_sae); break;
			case 3: vcvtsd2si(eax, xmm0 | T_rz_sae); break;
			}
			ret();
		} else {
			sub(rsp, 8);
			stmxcsr(ptr[rsp]);
			mov(eax, ptr[rsp]);
			mov(edx, eax);
			and_(eax, ~(3u << 13));
			or_(eax, mode << 13);
			mov(ptr[rsp], eax);
			ldmxcsr(ptr[rsp]);
			vcvtsd2si(eax, xmm0);
			mov(ptr[rsp], edx);
			ldmxcsr(ptr[rsp]);
			add(rsp, 8);
			ret();
		}
	}
};

int main()
	try
{
	const double tbl[] = { -1.6, -1.5, -1.4, -0.6, -0.5, -0.4, 0.4, 0.5, 0.6, 1.4, 1.5, 1.6 };
	for (uint8_t mode = 0; mode < 4; mode++) {
		printf("mode=%d\n", mode);
		Round c(mode);
		int (*f)(double) = c.getCode<int (*)(double)>();
		for (const auto& d : tbl) {
			printf("f(%f) = %d\n", d, f(d));
		}
	}
	union di {
		uint32_t u[2];
		double d;
	} di;
	di.u[0] = 0xffffffff;
	di.u[1] = 0x3fdfffff;
	printf("d=%f\n", di.d);
	di.u[1] |= 0x80000000;
	printf("d=%f\n", di.d);

	for (int i = 0; i < 2; i++) {
		if (i == 1) {
			puts("avx512");
			Xbyak::util::Cpu cpu;
			if (!cpu.has(Xbyak::util::Cpu::tAVX512F)) {
				puts("not supported");
				break;
			}
		}
		for (uint8_t mode = 0; mode < 4; mode++) {
			printf("mode=%d\n", mode);
			Round2 c(mode, i == 1);
			int (*f)(double) = c.getCode<int (*)(double)>();
			for (const auto& d : tbl) {
				printf("f(%f) = %d\n", d, f(d));
			}
		}
	}
} catch (std::exception& e) {
	printf("ERR %s\n", e.what());
	return 1;
}
