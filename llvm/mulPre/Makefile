MCL_DIR?=../../../mcl
all: gen-mulpre-test.exe

LLVM_VER?=-12
CFLAGS=-I $(MCL_DIR) -I $(MCL_DIR)/include -I $(MCL_DIR)/src -DNDEBUG -O2
LDFLAGS=-lgmp -lgmpxx -lstdc++

OBJS=mulpre-test.o mulpre.o bint-x64-amd64.o

gen-mulpre.exe: gen-mulpre.o
	$(CXX) -o $@ $<

mulpre.ll: gen-mulpre.exe
	./$< > $@

#CLANG_OPT=-mbmi2 -mno-sse
mulpre.o: mulpre.ll
	clang++$(LLVM_VER) -c -o $@ $< $(CFLAGS) $(CLANG_OPT)

bint-x64-amd64.o:
	as -o $@ $(MCL_DIR)/src/asm/bint-x64-amd64.s

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

mulpre-test.exe: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

test: mulpre-test.exe
	./mulpre-test.exe


clean:
	$(RM) $(OBJS) *.exe *.o *.ll *.d

.PHONY: clean
