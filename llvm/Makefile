target = test

CC=clang
COPTS=-O3 -Wall -Wextra
LLVM_OPT_OPTS=-std-compile-opts

VER=-3.6
LLVM_AS=llvm-as$(VER)
LLVM_LINK=llvm-link$(VER)
LLVM_LLC=llc$(VER)
LLVM_OPT=opt$(VER)
LLVM_DIS=llvm-dis$(VER)
OPT_LLC=$(LLVM_OPT) -O3 -o - | $(LLVM_LLC) -O3 -o -

test: test.cpp test0.s
	g++ test.cpp test0.s -O3 -o test

%.s: %.ll
	$(LLVM_LLC) -O3 -o $@ $< 

.ll.bc:
	$(LLVM_AS) -o $@ $<

.cpp.bc:
	$(CC) $(COPTS) -emit-llvm -S -o $@ $<

uint.bc: uint.ll
	$(LLVM_AS) -o $@ $<

uint.optimized.bc: uint.bc
	$(LLVM_OPT) $(LLVM_OPT_OPTS) -o $@ $<

uint.optimized.ll: uint.optimized.bc
	$(LLVM_DIS) -o $@ $<

uint.optimized.s: uint.optimized.ll
	$(LLVM_LLC) -o $@ $<

test0.s: test0.ll

test1: test1-3.s
	clang++ test1-3.s -o test1

test1.s: test1.cpp rank1.hpp
	clang++ -O3 -S -emit-llvm -march=corei7 test1.cpp

test1-asm.s: test1-asm.ll
	$(LLVM_AS) -o test1-asm.s test1.asm.ll

test1.bc: test1.s test1-asm.s
	$(LLVM_LINK) -o test1.bc test1.s test1-asm.s

test1-2.bc: test1.bc
#	opt -O3 -std-link-opts -o test1-2.bc test1.bc
	opt -O3 -o test1-2.bc test1.bc

test1-3.s: test1-2.bc
	llc -O3 -o test1-3.s test1-2.bc

clean:
	rm -rf *.s *.o *.bc uint.optimized.ll mul.ll

mul.ll: mul.txt
	python gen_mul.py mul.ll 64
mul86.ll: mul.txt
	python gen_mul.py mul86.ll 32
opt: mul.ll
	cat mul.ll | $(OPT_LLC) -x86-asm-syntax=intel -mattr=bmi2 -march=x86-64
opt86: mul86.ll
	cat mul86.ll | $(OPT_LLC) -x86-asm-syntax=intel -mattr=bmi2 -march=x86
opt-arm: mul.ll
	cat mul.ll | $(OPT_LLC) -march=arm

asm: gen.cpp
	clang++ -I ../../mcl/src -I ../../cybozulib/include gen.cpp -o gen
	./gen > t.ll
	cat t.ll | $(OPT_LLC) -x86-asm-syntax=intel -mattr=bmi2 -march=x86-64
