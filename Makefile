OPT=-msse4 -O3 -fomit-frame-pointer -DNDEBUG -msse4
GCC_VER=$(shell gcc -dumpversion)
ifeq ($(shell expr $(GCC_VER) \>= 4.2),1)
    OPT+=-mtune=native
endif

TARGET=intsort_test rank_test

BIT=32
ifeq ($(shell uname -m),x86_64)
BIT=64
endif
ifeq ($(shell uname -s),Darwin)
BIT=64
endif
ifeq ($(BIT),64)
endif
ifeq ($(USE_C11),1)
	CFLAGS+=-std=c++0x -DUSE_C11
endif
# ----------------------------------------------------------------
CFLAGS+= -fno-operator-names $(OPT) -I../xbyak/ -g -I../cybozulib/include
CFLAGS_WARN=-Wall -Wextra -Wformat=2 -Wcast-qual -Wwrite-strings -Wfloat-equal -Wpointer-arith # -Wcast-align 
CFLAGS+=$(CFLAGS_WARN)
# ----------------------------------------------------------------
ifeq ($(COMPARE_MARISA),1)
	RANK_CFLAGS+=-DCOMPARE_MARISA -Icomp/marisa-0.2.0/lib/
	RANK_LDFLAGS+=-lmarisa -Lcomp/lib
	MARISA_LIB=comp/lib/libmarisa.a
endif
ifeq ($(COMPARE_SUX),1)
	RANK_CFLAGS+=-DCOMPARE_SUX -Icomp/sux-0.7
	RANK_LDFLAGS+=comp/sux-0.7/rank9.o
	SUX_LIB=comp/sux-0.7/rank9.o
endif
ifeq ($(COMPARE_SDSL),1)
	RANK_CFLAGS+=-DCOMPARE_SDSL -Icomp/include
	RANK_LDFLAGS+=-lsdsl -ldivsufsort -ldivsufsort64 -Lcomp/lib
	SUX_LIB=comp/lib/libsdsl.a
endif

HEADER=util.hpp
all:$(TARGET)

.SUFFIXES: .cpp

intsort_test: intsort_test.o
	$(CXX) $(LDFLAGS) $< -o $@

.cpp.o:
	$(CXX) -c $< -o $@ $(CFLAGS)

.cc.o:
	$(CXX) -c $< -o $@ $(CFLAGS)

.c.o:
	$(CXX) -c $< -o $@ $(CFLAGS)

clean:
	$(RM) *.o $(TARGET)

rank_test: rank_test.o $(MARISA_LIB) $(SUX_LIB) $(SDSL_LIB)
	$(CXX) $< -o $@ $(LDFLAGS) $(RANK_LDFLAGS)

rank_test.o: rank_test.cpp
	$(CXX) -c $< -o $@ $(CFLAGS) $(RANK_CFLAGS)

comp/lib/libmarisa.a:
	(cd comp && ./build-marisa.sh)

comp/sux-0.7/rank9.cpp:
	(cd comp && ./build-sux.sh)

comp/sux-0.7/rank9.o: comp/sux-0.7/rank9.cpp
	$(CXX) -c $< -o $@ $(CFLAGS) $(RANK_CFLAGS)

comp/lib/libsdsl.a:
	-(cd comp && ./build-sdsl.sh)

intsort_test.o: intsort_test.cpp intsort.hpp v128.h
rank_test.o: rank_test.cpp rank.hpp

