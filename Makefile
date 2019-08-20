OPT=-msse4 -O3 -fomit-frame-pointer -DNDEBUG -msse4
GCC_VER=$(shell gcc -dumpversion)
ifeq ($(shell expr $(GCC_VER) \>= 4.2),1)
    OPT+=-mtune=native
endif

TARGET=intsort_test rank_test wm_test csucvector_test

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
CFLAGS+= $(OPT) -I../xbyak/ -g -I../cybozulib/include
CFLAGS_WARN=-Wall -Wextra -Wformat=2 -Wcast-qual -Wwrite-strings -Wfloat-equal -Wpointer-arith # -Wcast-align 
CFLAGS+=$(CFLAGS_WARN)
# ----------------------------------------------------------------
ifeq ($(COMPARE_MARISA),1)
	RANK_CFLAGS+=-DCOMPARE_MARISA -Icomp/marisa-0.2.2/lib/
	RANK_LDFLAGS+=-lmarisa -Lcomp/lib
	MARISA_LIB=comp/lib/libmarisa.a comp/marisa-0.2.2/lib/marisa/grimoire/vector/bit-vector.o
endif
ifeq ($(COMPARE_SUX),1)
	RANK_CFLAGS+=-DCOMPARE_SUX -Icomp/sux-0.7
	RANK_LDFLAGS+=comp/sux-0.7/rank9sel.o
	SUX_LIB=comp/sux-0.7/rank9sel.o
endif
ifeq ($(COMPARE_SDSL),1)
	RANK_CFLAGS+=-DCOMPARE_SDSL -Icomp/include
	RANK_LDFLAGS+=-lsdsl -ldivsufsort -ldivsufsort64 -Lcomp/lib
	SUX_LIB=comp/lib/libsdsl.a
endif

# for wm_test.cpp

ifeq ($(COMPARE_WAT),1)
	WAT_CFLAGS+=-DCOMPARE_WAT -Icomp/wat_array/src
	WAT_LDFLAGS+=-lwat_array -Lcomp/lib
	WAT_LIB=comp/lib/libwat_array.a
endif
ifeq ($(COMPARE_WAVELET),1)
	WAVELET_CFLAGS+=-DCOMPARE_WAVELET -Icomp/wavelet-matrix-cpp/src
	WAVELET_LDFLAGS+=-lwavelet_matrix -Lcomp/lib
	WAVELET_LIB=comp/lib/libwavelet_matrix.a
endif
ifeq ($(COMPARE_SHELLINFORD),1)
	SHELLINFORD_CFLAGS+=-DCOMPARE_SHELLINFORD -Icomp/shellinford/src
	SHELLINFORD_LDFLAGS=-lshellinford -Lcomp/lib
	SHELLINFORD_SRC=comp/shellinford/src/shellinford_bit_vector.cc
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
	$(RM) *.o $(TARGET) prof-vtune

rank_test: rank_test.o $(MARISA_LIB) $(SUX_LIB) $(SDSL_LIB) $(WAT_LIB) $(SHELLINFORD_LIB) ../cybozulib/include/cybozu/sucvector.hpp
	$(CXX) $< -o $@ $(LDFLAGS) $(RANK_LDFLAGS) $(WAT_LDFLAGS) $(SHELLINFORD_LDFLAGS) $(MARISA_LIB)

rank_test.o: rank_test.cpp
	$(CXX) -c $< -o $@ $(CFLAGS) $(RANK_CFLAGS) $(WAT_CFLAGS) $(SHELLINFORD_CFLAGS)

wm_test: wm_test.o $(WAVELET_LIB) $(WAT_LIB) $(SHELLINFORD_LIB)
	$(CXX) $< -o $@ $(LDFLAGS) $(WAVELET_LDFLAGS) $(WAT_LDFLAGS) $(SHELLINFORD_LDFLAGS)

wm_test.o: wm_test.cpp ../cybozulib/include/cybozu/wavelet_matrix.hpp
	$(CXX) -c $< -o $@ $(CFLAGS) $(WAVELET_CFLAGS) $(WAT_CFLAGS) $(SHELLINFORD_CFLAGS)

csucvector_test: csucvector_test.o $(MARISA_LIB) $(SUX_LIB) $(SDSL_LIB) $(WAT_LIB) $(SHELLINFORD_LIB) ../cybozulib/include/cybozu/sucvector.hpp ../cybozulib/include/cybozu/csucvector.hpp
	$(CXX) $< -o $@ $(LDFLAGS) $(RANK_LDFLAGS) $(WAT_LDFLAGS) $(SHELLINFORD_LDFLAGS) $(MARISA_LIB)

csucvector_test.o: csucvector_test.cpp ../cybozulib/include/cybozu/csucvector.hpp
	$(CXX) -c $< -o $@ $(CFLAGS) $(RANK_CFLAGS) $(WAT_CFLAGS) $(SHELLINFORD_CFLAGS)

comp/lib/libmarisa.a:
	(cd comp && ./build-marisa.sh)

comp/sux-0.7/rank9.cpp:
	(cd comp && ./build-sux.sh)

comp/sux-0.7/rank9.o: comp/sux-0.7/rank9.cpp
	$(CXX) -c $< -o $@ $(CFLAGS) $(RANK_CFLAGS)

comp/lib/libsdsl.a:
	-(cd comp && ./build-sdsl.sh)

comp/lib/libwavelet_matrix.a:
	-(cd comp && ./build-wavelet.sh)

comp/lib/libwat_array.a:
	-(cd comp && ./build-wat.sh)

comp/shellinford/src/shellinford_bit_vector.cc:
	-(cd comp && ./buld-shellinford.sh)

intsort_test.o: intsort_test.cpp intsort.hpp v128.h
rank_test.o: rank_test.cpp rank.hpp ../cybozulib/include/cybozu/sucvector.hpp

prof-vtune: prof.cpp
	$(CXX) -O3 -Wall -Wextra -o prof-vtune prof.cpp -DUSE_VTUNE -ljitprofiling -I ../xbyak -I /opt/intel/vtune_amplifier/include/ -L /opt/intel/vtune_amplifier/lib64/
