all: add.wasm


CXX=clang++-6.0
LD=wasm-ld-6.0
CFLAGS+=-Ofast -Oz -target wasm32-wasm -Wall -Wextra

add.o: add.cpp
	$(CXX) -c $< $(CFLAGS)

sub.o: sub.cpp
	$(CXX) -c $< $(CFLAGS)

add.wasm: add.o sub.o
	$(LD) -o $@ add.o sub.o -no-entry --strip-all --allow-undefined

clean:
	rm -rf add.js add.wasm add.o
