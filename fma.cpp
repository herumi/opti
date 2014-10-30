#define XBYAK_NO_OP_NAMES
#include <xbyak/xbyak_util.h>
#include <cybozu/array.hpp>
#include <cybozu/benchmark.hpp>
#include <assert.h>
#include <random>

typedef cybozu::AlignedArray<double, 32> RealVec;

struct Code : Xbyak::CodeGenerator {
	Code()
	{
		Xbyak::util::StackFrame sf(this, 3);
		const Xbyak::Reg64& px = sf.p[0];
		const Xbyak::Reg64& py = sf.p[1];
		const Xbyak::Reg64& n = sf.p[2];

#if 1
		vxorpd(ym0, ym0);
		lea(px, ptr [px + n * 8]);
		lea(py, ptr [py + n * 8]);
		neg(n);
	L("@@");
		vmovapd(ym1, ptr [px + n * 8]);
		vfmadd231pd(ym0, ym1, ptr [py + n * 8]);
		add(n, 4);
		jnz("@b");
#else
		vxorpd(ym0, ym0);
		vxorpd(ym1, ym1);
		vxorpd(ym2, ym2);
		vxorpd(ym3, ym3);
		lea(px, ptr [px + n * 8]);
		lea(py, ptr [py + n * 8]);
		neg(n);
	L("@@");
		vmovapd(ym4, ptr [px + n * 8]);
		vmovapd(ym5, ptr [px + n * 8 + 32]);
		vmovapd(ym6, ptr [px + n * 8 + 64]);
		vmovapd(ym7, ptr [px + n * 8 + 96]);
		vfmadd231pd(ym0, ym4, ptr [py + n * 8]);
		vfmadd231pd(ym1, ym5, ptr [py + n * 8 + 32]);
		vfmadd231pd(ym2, ym6, ptr [py + n * 8 + 64]);
		vfmadd231pd(ym3, ym7, ptr [py + n * 8 + 96]);
		add(n, 16);
		jnz("@b");
		vaddpd(ym0, ym1);
		vaddpd(ym2, ym3);
		vaddpd(ym0, ym2);
#endif
		vhaddpd(ym0, ym0);
		vpermpd(ym0, ym0, 2 << 2);
		vhaddpd(ym0, ym0);
	}
};

const int N = 1000 * 1000;

void init(RealVec& v, size_t n)
{
	static std::mt19937 rg(0);
	std::uniform_real_distribution<double> dist(-2, 2);
	v.resize(n);
	for (size_t i = 0; i < n; i++) {
		v[i] = dist(rg);
	}
}

double dotC(const double *a, const double *b, int n)
{
	double sum = 0;
	for (int i = 0; i < n; i++) {
		sum += a[i] * b[i];
	}
	return sum;
}

void put(const RealVec& v)
{
	for (size_t i = 0; i < v.size(); i++) {
		printf("%f ", v[i]);
	}
	printf("\n");
}

int main()
{
	assert((N % 16) == 0);
	Code c;
	auto dotA = c.getCode<double (*)(const double *, const double *, int)>();
	RealVec x, y;
	init(x, N);
	init(y, N);
	const double *px = &x[0];
	const double *py = &y[0];
	double a = 0;
	double b = 0;
	CYBOZU_BENCH("A", a += dotA, px, py, N);
	CYBOZU_BENCH("C", b += dotC, px, py, N);
	printf("dot A=%f C=%f\n", a, b);
}

