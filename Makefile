GCC_VER=$(shell gcc -dumpversion)
ifeq ($(shell expr $(GCC_VER) \>= 4.2),1)
    ADD_OPT+=-mtune=native
endif

TARGET=str_util_test

BIT=32
ifeq ($(shell uname -m),x86_64)
BIT=64
endif
ifeq ($(shell uname -s),Darwin)
BIT=64
endif
ifeq ($(BIT),64)
TARGET += str_util_test64
endif
# ----------------------------------------------------------------
CFLAGS += -O3 -fomit-frame-pointer -DNDEBUG -fno-operator-names -msse4 $(ADD_OPT)
CFLAGS_WARN=-Wall -Wextra -Wformat=2 -Wcast-qual -Wwrite-strings -Wfloat-equal -Wpointer-arith # -Wcast-align 
CFLAGS+=$(CFLAGS_WARN)
# ----------------------------------------------------------------

HEADER=str_util.hpp benchmark.hpp mischasan_strstr.hpp
all:$(TARGET)

.SUFFIXES: .cpp

str_util_test: str_util_test.cpp $(HEADER)
	$(CXX) $(CFLAGS) str_util_test.cpp -o $@ -m32

str_util_test64: str_util_test.cpp $(HEADER)
	$(CXX) $(CFLAGS) str_util_test.cpp -o $@ -m64

.cpp.o:
	$(CXX) -c $< -o $@ $(CFLAGS)

.c.o:
	$(CXX) -c $< -o $@ $(CFLAGS)

clean:
	$(RM) *.o $(TARGET)

