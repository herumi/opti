x64:
	llc-3.5 modp.ll -o - -x86-asm-syntax=intel
x86:
	llc-3.5 modp.ll -o - -x86-asm-syntax=intel -march=x86
arm64:
	llc-3.5 modp.ll -o - -march=aarch64

arm:
	llc-3.5 modp.ll -o - -march=arm

test: modp_test.cpp modp.ll
	clang++ -O3 modp_test.cpp modp.ll -o test -Wall -Wextra -I ../../cybozulib/include -I ../../mie/include -lgmp -lgmpxx

