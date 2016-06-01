#include "llvm_gen.hpp"

struct Code : public mcl::Generator {
	Operand Void;
	void gen1()
	{
		const int bit = 1024;
		resetGlobalIdx();
		Operand pz(IntPtr, bit);
		Operand px(IntPtr, bit);
		Operand py(IntPtr, bit);
		std::string name = "add" + cybozu::itoa(bit);
		Function f(name, Void, pz, px, py);
		beginFunc(f);
		Operand x = load(px);
		Operand y = load(py);
		x = add(x, y);
		store(x, pz);
		ret(Void);
		endFunc();
	}
	void gen2()
	{
		const int unit = 64;
		resetGlobalIdx();
		Operand pz(IntPtr, unit);
		Operand px(IntPtr, unit);
		Operand py(IntPtr, unit);
		std::string name = "add128";
		Function f(name, Void, pz, px, py);
		beginFunc(f);
		Operand x = load(px);
		Operand y = load(py);
		x = zext(x, 128);
		y = zext(y, 128);
		x = add(x, y);
		Operand L = trunc(x, 64);
		store(L, pz);
		Operand xH = load(getelementptr(px, makeImm(32, 1)));
		Operand yH = load(getelementptr(py, makeImm(32, 1)));
		x = trunc(lshr(x, 64), 64);
		x = add(x, xH);
		x = add(x, yH);
		store(x, getelementptr(pz, makeImm(32, 1)));
		
		ret(Void);
		endFunc();
	}
};

int main()
{
	Code c;
	c.gen1();
}

