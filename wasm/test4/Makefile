all: add.wasm


CXX=clang++-6.0
LD=wasm-ld-6.0
CFLAGS+=-O3 -target wasm32-wasm -Wall -Wextra -fno-threadsafe-statics
CFLAGS+=-fno-builtin # to prevent importing memset
LDFLAGS=-no-entry --strip-all --allow-undefined
#LDFLAGS+=--check-signatures

add.o: add.cpp Makefile
	$(CXX) -c $< $(CFLAGS)

sub.o: sub.cpp Makefile
	$(CXX) -c $< $(CFLAGS)

add.wasm: add.o sub.o Makefile
	$(LD) -o $@ add.o sub.o $(LDFLAGS)

test: add.wasm
	node node-test.js

clean:
	rm -rf add.wasm add.o sub.o
