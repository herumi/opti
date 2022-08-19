MCL_DIR?=../../../mcl
ARCH?=$(shell uname -m)
UNAME_S=$(shell uname -s)

all: mulpre-test.exe

LLVM_VER?=-12
CFLAGS=-I $(MCL_DIR) -I $(MCL_DIR)/include -I $(MCL_DIR)/src -DNDEBUG -O2
LDFLAGS=-lgmp -lgmpxx -lstdc++

ifeq ($(UNAME_S),Darwin)
GMP_DIR?=/opt/homebrew/
CFLAGS+= -I $(GMP_DIR)/include
LDFLAGS+= -L $(GMP_DIR)/lib
endif

OBJS=mulpre-test.o mulpre.o
ifeq ($(ARCH),x86_64)
OBJS+=bint-x64-amd64.o
else
OBJS+=bint64.o
endif


gen-mulpre.exe: gen-mulpre.o
	$(CXX) -o $@ $<

mulpre.ll: gen-mulpre.exe
	./$< > $@

#CLANG_OPT=-mbmi2 -mno-sse
mulpre.o: mulpre.ll
	clang++$(LLVM_VER) -c -o $@ $< $(CFLAGS) $(CLANG_OPT)

bint-x64-amd64.o:
	as -o $@ $(MCL_DIR)/src/asm/bint-x64-amd64.s

bint64.o:
	clang++$(LLVM_VER) -c -o $@ $(MCL_DIR)/src/bint64.ll $(CLANG_OPT)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

mulpre-test.exe: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

test: mulpre-test.exe
	./mulpre-test.exe

asm:
	clang++$(LLVM_VER) -S -O2 -mbmi2 -masm=intel mulpre.ll

clean:
	$(RM) $(OBJS) *.exe *.o *.ll *.d *.asm

.PHONY: clean
