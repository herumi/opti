all: add.wasm

LLVM_VER=11
CXX=clang++-$(LLVM_VER)
LD=wasm-ld-$(LLVM_VER)
CFLAGS+=-O3 -target wasm32-wasm -Wall -Wextra -fno-threadsafe-statics
CFLAGS+=-fno-builtin # to prevent importing memset
LDFLAGS=-no-entry --allow-undefined #--strip-all
#LDFLAGS+=--check-signatures

add.o: add.cpp Makefile
	$(CXX) -c $< $(CFLAGS)

sub.o: sub.cpp Makefile
	$(CXX) -c $< $(CFLAGS)

add.wasm: add.o sub.o Makefile
	$(LD) -o $@ add.o sub.o $(LDFLAGS)

test: add.wasm
	node node-test.js

disas: add.wasm
	wasm-objdump -s -d add.wasm

clean:
	rm -rf add.wasm add.o sub.o
