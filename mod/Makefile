VER=-3.5
LLC=llc$(VER)
OPT=opt$(VER)
DIS=llvm-dis$(VER)
ASM=llvm-as$(VER)

x64:
	$(LLC) modp.ll -o - -x86-asm-syntax=intel
x86:
	$(LLC) modp.ll -o - -x86-asm-syntax=intel -march=x86
arm64:
	$(LLC) modp.ll -o - -march=aarch64

arm:
	$(LLC) modp.ll -o - -march=arm

test: modp_test.cpp modp.ll
	clang++ -O3 modp_test.cpp modp.ll -o test -Wall -Wextra -I ../../cybozulib/include -I ../../mie/include -lgmp -lgmpxx

opt: modp.ll
	$(ASM) modp.ll -o modp.bc
	$(OPT) -O3 modp.bc -o modp.opt.bc
	$(DIS) modp.opt.bc -o modp.opt.ll
	$(LLC) modp.opt.ll -o - -x86-asm-syntax=intel

